#include "kt_esp32_link.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_app/app_io.h"
#include "kt_app/main_access_log.h"
#include "kt_app/main_card_db.h"
#include "kt_app/main_controller_app.h"
#include "kt_components/kt_ringbuf.h"
#include "kt_system/kt_tick.h"
#include "kt_system/kt_system_health.h"
#include "usart.h"
#include <string.h>

#define BIN_SOF1 0xA5U
#define BIN_SOF2 0x5AU
#define BIN_EOF  0x0DU
#define BIN_MAX_FRAME_LEN (8U + KT_BIN_MAX_PAYLOAD_LEN + 3U)
#define ESP32_RX_RING_SIZE 256U
#define ESP32_DEVICE_STATUS_PERIOD_MS 30000U
#define ESP32_TX_FRAME_MAX_LEN BIN_MAX_FRAME_LEN

typedef enum {
    RX_WAIT_SOF1 = 0,
    RX_WAIT_SOF2,
    RX_HEADER,
    RX_PAYLOAD,
    RX_CRC1,
    RX_CRC2,
    RX_EOF
} esp32_rx_state_t;

typedef struct {
    uint8_t msg_type;
    uint16_t seq;
    uint16_t payload_len;
    uint8_t payload[KT_BIN_MAX_PAYLOAD_LEN];
} esp32_frame_t;

typedef struct {
    uint8_t type;
    uint16_t len;
    uint8_t bytes[ESP32_TX_FRAME_MAX_LEN];
} esp32_tx_item_t;

static volatile uint8_t rx_byte;
static kt_ringbuf_t rx_ring;
static uint8_t rx_ring_storage[ESP32_RX_RING_SIZE];
static volatile uint32_t rx_overflow_count;
static esp32_tx_item_t tx_queue[KT_ESP32_TX_QUEUE_LEN];
static uint8_t tx_head;
static uint8_t tx_tail;
static uint8_t tx_count;
static uint32_t tx_drop_count;
static uint32_t tx_busy_count;
static uint32_t tx_fail_count;
static uint8_t last_tx_type;
static HAL_StatusTypeDef last_tx_result;
static esp32_rx_state_t rx_state = RX_WAIT_SOF1;
static uint8_t header[6];
static uint8_t header_pos;
static uint8_t payload[KT_BIN_MAX_PAYLOAD_LEN];
static uint16_t payload_pos;
static uint16_t payload_len;
static uint16_t rx_crc;
static uint16_t tx_seq = 1U;
static uint8_t recent_valid;
static uint8_t link_ok;
static uint8_t ever_online;
static uint32_t heartbeat_count;
static uint32_t recovered_count;
static uint32_t eof_error_count;
static uint8_t wifi_state;
static int8_t wifi_rssi;
static uint32_t last_rx_ms;
static uint32_t last_tx_ms;
static uint32_t last_heartbeat_ms;
static uint32_t last_device_status_ms;
static uint8_t device_status_dirty;
static uint32_t link_led_on_ms;
static uint8_t link_led_active;
static esp32_frame_t recent_frame;
static char wifi_ssid[33];
static uint8_t wifi_ip[4];

static uint16_t crc16_modbus_update(uint16_t crc, uint8_t data)
{
    uint8_t i;
    crc ^= data;
    for (i = 0; i < 8U; i++) {
        if (crc & 0x0001U) {
            crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
        } else {
            crc >>= 1U;
        }
    }
    return crc;
}

static uint16_t crc16_modbus(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;

    for (i = 0; i < len; i++) {
        crc = crc16_modbus_update(crc, data[i]);
    }
    return crc;
}

static void put_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)(v >> 8U);
}

static uint16_t get_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8U);
}

static const char *msg_type_text(uint8_t type)
{
    switch (type) {
    case KT_MSG_PING: return "PING";
    case KT_MSG_PONG: return "PONG";
    case KT_MSG_ACK: return "ACK";
    case KT_MSG_ERR: return "ERR";
    case KT_MSG_HEARTBEAT: return "HEARTBEAT";
    case KT_MSG_HEARTBEAT_ACK: return "HEARTBEAT_ACK";
    case KT_MSG_WIFI_STATUS: return "WIFI_STATUS";
    case KT_MSG_CARD_EVENT: return "CARD_EVENT";
    case KT_MSG_DEVICE_STATUS: return "DEVICE_STATUS";
    case KT_MSG_BOOT_SYNC: return "BOOT_SYNC";
    case KT_MSG_SYNC_ACK: return "SYNC_ACK";
    default: return "UNKNOWN";
    }
}

static uint8_t is_heartbeat_type(uint8_t type)
{
    return (type == KT_MSG_HEARTBEAT || type == KT_MSG_HEARTBEAT_ACK) ? 1U : 0U;
}

static uint8_t tx_queue_push(uint8_t type, const uint8_t *frame, uint16_t len)
{
    if (len > ESP32_TX_FRAME_MAX_LEN) {
        tx_drop_count++;
        return 0U;
    }
    if (tx_count >= KT_ESP32_TX_QUEUE_LEN) {
        tx_drop_count++;
        return 0U;
    }

    tx_queue[tx_head].type = type;
    tx_queue[tx_head].len = len;
    memcpy(tx_queue[tx_head].bytes, frame, len);
    tx_head++;
    if (tx_head >= KT_ESP32_TX_QUEUE_LEN) {
        tx_head = 0U;
    }
    tx_count++;
    return 1U;
}

static void send_frame(uint8_t type, const uint8_t *data, uint16_t len, uint8_t corrupt_crc)
{
    uint8_t frame[BIN_MAX_FRAME_LEN];
    uint16_t crc;
    uint16_t seq = tx_seq++;
    uint16_t pos = 0U;

    if (len > KT_BIN_MAX_PAYLOAD_LEN) {
        KT_LOG_WARN("ESP32 TX payload too long: %u", (unsigned int)len);
        return;
    }

    frame[pos++] = BIN_SOF1;
    frame[pos++] = BIN_SOF2;
    frame[pos++] = KT_BIN_PROTOCOL_VERSION;
    frame[pos++] = type;
    put_u16_le(&frame[pos], seq);
    pos += 2U;
    put_u16_le(&frame[pos], len);
    pos += 2U;
    if (len > 0U && data != 0) {
        memcpy(&frame[pos], data, len);
        pos = (uint16_t)(pos + len);
    }

    crc = crc16_modbus(&frame[2], (uint16_t)(6U + len));
    if (corrupt_crc) {
        crc ^= 0xFFFFU;
    }
    put_u16_le(&frame[pos], crc);
    pos += 2U;
    frame[pos++] = BIN_EOF;

    if (tx_queue_push(type, frame, pos) != 0U) {
#if (KT_LOG_UART_FRAME_ENABLE != 0)
        if (!is_heartbeat_type(type)) {
            KT_LOG_INFO("ESP32 TX queued: %s seq=%u", msg_type_text(type), (unsigned int)seq);
        }
#endif
    } else {
        if (!is_heartbeat_type(type)) {
            KT_LOG_WARN("ESP32 TX queue full: %s", msg_type_text(type));
        }
    }
}

static void send_ack_for(uint16_t ack_seq, uint8_t ack_type, uint8_t msg_type)
{
    uint8_t p[3];

    put_u16_le(p, ack_seq);
    p[2] = ack_type;
    send_frame(msg_type, p, sizeof(p), 0U);
}

static void enqueue_device_status_now(void)
{
    uint8_t p[20] = {0};
    kt_ds1302_time_t now;

    kt_ds1302_read_time(&now);
    p[0] = KT_BIN_PROTOCOL_VERSION;
    p[1] = 0U;
    p[2] = 9U;
    p[3] = 1U;
    p[4] = main_card_db_count();
    p[5] = main_access_log_count();
    p[6] = 0U;
    p[7] = 1U;
    p[8] = kt_ds1302_time_is_valid(&now);
    if (p[8]) {
        p[9] = now.year;
        p[10] = now.month;
        p[11] = now.day;
        p[12] = now.hour;
        p[13] = now.minute;
        p[14] = now.second;
    }
    p[15] = main_controller_app_get_seat_state(0U);
    p[16] = main_controller_app_get_seat_state(1U);
    p[17] = main_controller_app_get_seat_state(2U);
    p[18] = main_controller_app_get_gate_state();
    p[19] = main_controller_app_get_last_card_result();
    send_frame(KT_MSG_DEVICE_STATUS, p, sizeof(p), 0U);
    last_device_status_ms = kt_tick_get_ms();
    device_status_dirty = 0U;
#if (KT_LOG_VERBOSE_ENABLE != 0)
    KT_LOG_INFO("ESP32 TX: DEVICE_STATUS cards=%u logs=%u seats=%u/%u/%u gate=%u",
                (unsigned int)p[4], (unsigned int)p[5],
                (unsigned int)p[15], (unsigned int)p[16], (unsigned int)p[17],
                (unsigned int)p[18]);
#endif
}

void kt_esp32_link_send_device_status(void)
{
    device_status_dirty = 1U;
}

static void pulse_link_led(void)
{
    kt_led_on(&app_led);
    link_led_on_ms = kt_tick_get_ms();
    link_led_active = 1U;
}

static void handle_wifi_status(const uint8_t *data, uint16_t len)
{
    uint8_t ssid_len;

    if (len < 36U) {
        KT_LOG_WARN("ESP32 WIFI_STATUS len error: %u", (unsigned int)len);
        return;
    }

    wifi_state = data[0];
    wifi_rssi = (int8_t)data[1];
    ssid_len = data[2];
    if (ssid_len > 32U) {
        ssid_len = 32U;
    }
    memcpy(wifi_ssid, &data[3], ssid_len);
    wifi_ssid[ssid_len] = '\0';
    memcpy(wifi_ip, &data[35], 4U);
    KT_LOG_INFO("ESP32 WIFI %s ssid=%s ip=%u.%u.%u.%u rssi=%d",
                wifi_state ? "connected" : "offline",
                wifi_ssid,
                wifi_ip[0], wifi_ip[1], wifi_ip[2], wifi_ip[3],
                (int)wifi_rssi);
}

static void handle_heartbeat(uint16_t seq, const uint8_t *data, uint16_t len)
{
    uint8_t was_online = link_ok;

    heartbeat_count++;
    last_heartbeat_ms = kt_tick_get_ms();
    link_ok = 1U;
    if (len >= 6U) {
        wifi_state = data[4];
        wifi_rssi = (int8_t)data[5];
    }

    pulse_link_led();
    send_ack_for(seq, KT_MSG_HEARTBEAT, KT_MSG_HEARTBEAT_ACK);

    if (!ever_online || !was_online) {
        if (ever_online) {
            recovered_count++;
        }
        ever_online = 1U;
        kt_esp32_link_send_device_status();
    }
}

static void handle_frame(uint8_t type, uint16_t seq, const uint8_t *data, uint16_t len)
{
    recent_valid = 1U;
    recent_frame.msg_type = type;
    recent_frame.seq = seq;
    recent_frame.payload_len = len;
    if (len > 0U) {
        memcpy(recent_frame.payload, data, len);
    }
    last_rx_ms = kt_tick_get_ms();

    if (type == KT_MSG_HEARTBEAT) {
        handle_heartbeat(seq, data, len);
    } else if (type == KT_MSG_PONG) {
        link_ok = 1U;
        if (len >= 5U) {
            wifi_state = data[4];
        }
        KT_LOG_INFO("ESP32 RX: PONG seq=%u wifi=%u", (unsigned int)seq, (unsigned int)wifi_state);
    } else if (type == KT_MSG_ACK) {
        link_ok = 1U;
#if (KT_LOG_UART_FRAME_ENABLE != 0)
        if (len >= 3U) {
            if (!is_heartbeat_type(data[2])) {
                KT_LOG_INFO("ESP32 RX: ACK ack_seq=%u type=%s",
                            (unsigned int)get_u16_le(data),
                            msg_type_text(data[2]));
            }
        } else {
            KT_LOG_INFO("ESP32 RX: ACK seq=%u", (unsigned int)seq);
        }
#endif
    } else if (type == KT_MSG_ERR) {
        link_ok = 0U;
        if (len >= 4U) {
            KT_LOG_WARN("ESP32 RX: ERR err_seq=%u type=%s reason=%u",
                        (unsigned int)get_u16_le(data),
                        msg_type_text(data[2]),
                        (unsigned int)data[3]);
        } else {
            KT_LOG_WARN("ESP32 RX: ERR seq=%u", (unsigned int)seq);
        }
    } else if (type == KT_MSG_WIFI_STATUS) {
        handle_wifi_status(data, len);
    } else {
#if (KT_LOG_UART_FRAME_ENABLE != 0)
        KT_LOG_INFO("ESP32 RX: %s seq=%u len=%u",
                    msg_type_text(type), (unsigned int)seq, (unsigned int)len);
#endif
    }
}

static void reset_rx(void)
{
    rx_state = RX_WAIT_SOF1;
    header_pos = 0U;
    payload_pos = 0U;
    payload_len = 0U;
    rx_crc = 0U;
}

static void parse_byte(uint8_t b)
{
    uint8_t crc_buf[6 + KT_BIN_MAX_PAYLOAD_LEN];
    uint16_t calc_crc;
    uint16_t seq;

    switch (rx_state) {
    case RX_WAIT_SOF1:
        if (b == BIN_SOF1) {
            rx_state = RX_WAIT_SOF2;
        }
        break;
    case RX_WAIT_SOF2:
        if (b == BIN_SOF2) {
            rx_state = RX_HEADER;
        } else if (b == BIN_SOF1) {
            rx_state = RX_WAIT_SOF2;
        } else {
            rx_state = RX_WAIT_SOF1;
        }
        header_pos = 0U;
        break;
    case RX_HEADER:
        header[header_pos++] = b;
        if (header_pos >= 6U) {
            payload_len = get_u16_le(&header[4]);
            if (header[0] != KT_BIN_PROTOCOL_VERSION || payload_len > KT_BIN_MAX_PAYLOAD_LEN) {
                KT_LOG_WARN("ESP32 RX format error");
                reset_rx();
            } else {
                payload_pos = 0U;
                rx_state = (payload_len == 0U) ? RX_CRC1 : RX_PAYLOAD;
            }
        }
        break;
    case RX_PAYLOAD:
        payload[payload_pos++] = b;
        if (payload_pos >= payload_len) {
            rx_state = RX_CRC1;
        }
        break;
    case RX_CRC1:
        rx_crc = b;
        rx_state = RX_CRC2;
        break;
    case RX_CRC2:
        rx_crc |= ((uint16_t)b << 8U);
        rx_state = RX_EOF;
        break;

    case RX_EOF:
        if (b != BIN_EOF) {
            eof_error_count++;
            KT_LOG_WARN("ESP32 RX EOF fail: 0x%02X", b);
            reset_rx();
            break;
        }
        memcpy(crc_buf, header, 6U);
        if (payload_len > 0U) {
            memcpy(&crc_buf[6], payload, payload_len);
        }
        calc_crc = crc16_modbus(crc_buf, (uint16_t)(6U + payload_len));
        if (calc_crc == rx_crc) {
            seq = get_u16_le(&header[2]);
            handle_frame(header[1], seq, payload, payload_len);
        } else {
            KT_LOG_WARN("ESP32 RX CRC fail");
        }
        reset_rx();
        break;
    default:
        reset_rx();
        break;
    }
}

void kt_esp32_link_init(void)
{
    reset_rx();
    kt_ringbuf_init(&rx_ring, rx_ring_storage, ESP32_RX_RING_SIZE);
    rx_overflow_count = 0U;
    tx_head = 0U;
    tx_tail = 0U;
    tx_count = 0U;
    tx_drop_count = 0U;
    tx_busy_count = 0U;
    tx_fail_count = 0U;
    last_tx_type = 0U;
    last_tx_result = HAL_OK;
    recent_valid = 0U;
    link_ok = 0U;
    ever_online = 0U;
    heartbeat_count = 0U;
    recovered_count = 0U;
    eof_error_count = 0U;
    last_heartbeat_ms = 0U;
    link_led_active = 0U;
    wifi_state = 0U;
    wifi_rssi = 0;
    wifi_ssid[0] = '\0';
    memset(wifi_ip, 0, sizeof(wifi_ip));
    last_device_status_ms = 0U;
    device_status_dirty = 0U;
    (void)HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_byte, 1U);
}

void kt_esp32_link_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) {
        if (kt_ringbuf_put(&rx_ring, rx_byte) != 0) {
            rx_overflow_count++;
        }
        (void)HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_byte, 1U);
    }
}

void kt_esp32_link_task(void)
{
    uint8_t b;
    uint8_t rx_budget = KT_ESP32_RX_BYTES_PER_TASK;
    esp32_tx_item_t *item;
    HAL_StatusTypeDef st;

    while (rx_budget > 0U && kt_ringbuf_get(&rx_ring, &b) == 0) {
        parse_byte(b);
        rx_budget--;
    }

    if (tx_count > 0U &&
        (last_tx_ms == 0U || kt_tick_is_timeout(last_tx_ms, KT_ESP32_TX_GAP_MS))) {
        if (huart3.gState == HAL_UART_STATE_READY) {
            item = &tx_queue[tx_tail];
            st = HAL_UART_Transmit(&huart3, item->bytes, item->len, KT_ESP32_TX_TIMEOUT_MS);
            last_tx_type = item->type;
            last_tx_result = st;
            last_tx_ms = kt_tick_get_ms();
            if (st != HAL_OK) {
                tx_fail_count++;
            }
            tx_tail++;
            if (tx_tail >= KT_ESP32_TX_QUEUE_LEN) {
                tx_tail = 0U;
            }
            tx_count--;
        } else {
            tx_busy_count++;
        }
    }

    if (link_led_active && kt_tick_is_timeout(link_led_on_ms, KT_ESP32_LINK_LED_PULSE_MS)) {
        kt_led_off(&app_led);
        link_led_active = 0U;
    }

    if (link_ok && last_heartbeat_ms != 0U &&
        kt_tick_is_timeout(last_heartbeat_ms, KT_ESP32_HEARTBEAT_TIMEOUT_MS)) {
        link_ok = 0U;
    }

    if (device_status_dirty &&
        (last_device_status_ms == 0U ||
         kt_tick_is_timeout(last_device_status_ms, KT_ESP32_DEVICE_STATUS_MIN_MS))) {
        enqueue_device_status_now();
    } else if (link_ok && kt_tick_is_timeout(last_device_status_ms, ESP32_DEVICE_STATUS_PERIOD_MS)) {
        device_status_dirty = 1U;
    }

    kt_system_health_note_esp32_task();
}

void kt_esp32_link_send_ping(void)
{
    uint8_t payload_data[4];
    uint32_t uptime = kt_tick_get_ms();

    payload_data[0] = (uint8_t)(uptime & 0xFFU);
    payload_data[1] = (uint8_t)((uptime >> 8U) & 0xFFU);
    payload_data[2] = (uint8_t)((uptime >> 16U) & 0xFFU);
    payload_data[3] = (uint8_t)((uptime >> 24U) & 0xFFU);
    send_frame(KT_MSG_PING, payload_data, sizeof(payload_data), 0U);
}

void kt_esp32_link_send_bad_crc_test(void)
{
    uint8_t payload_data[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    send_frame(KT_MSG_PING, payload_data, sizeof(payload_data), 1U);
}

void kt_esp32_link_send_card_event(const uint8_t uid[4],
                                   uint8_t access_type,
                                   uint8_t allowed,
                                   const kt_ds1302_time_t *time)
{
    uint8_t p[13];

    if (uid == 0 || time == 0) {
        return;
    }

    memcpy(&p[0], uid, 4U);
    p[4] = access_type;
    p[5] = allowed ? 1U : 0U;
    p[6] = time->year;
    p[7] = time->month;
    p[8] = time->day;
    p[9] = time->hour;
    p[10] = time->minute;
    p[11] = time->second;
    p[12] = 0U;
    send_frame(KT_MSG_CARD_EVENT, p, 12U, 0U);
}

void kt_esp32_link_send_mock_card_event(void)
{
    uint8_t uid[4] = {0xBFU, 0xA4U, 0xA5U, 0x1FU};
    kt_ds1302_time_t t = {26U, 4U, 29U, 15U, 30U, 10U};
    kt_esp32_link_send_card_event(uid, 0U, 1U, &t);
}

void kt_esp32_link_print_recent_rx(void)
{
    if (!recent_valid) {
        KT_LOG_WARN("ESP32 recent RX empty");
        return;
    }
    if (is_heartbeat_type(recent_frame.msg_type)) {
        KT_LOG_INFO("ESP32 recent RX: no user frame");
        return;
    }
    KT_LOG_INFO("ESP32 recent RX: type=%s seq=%u len=%u",
                msg_type_text(recent_frame.msg_type),
                (unsigned int)recent_frame.seq,
                (unsigned int)recent_frame.payload_len);
    if (recent_frame.msg_type == KT_MSG_WIFI_STATUS) {
        KT_LOG_INFO("WIFI %s ssid=%s ip=%u.%u.%u.%u rssi=%d",
                    wifi_state ? "connected" : "offline",
                    wifi_ssid,
                    wifi_ip[0], wifi_ip[1], wifi_ip[2], wifi_ip[3],
                    (int)wifi_rssi);
    }
}

void kt_esp32_link_print_status(void)
{
    KT_LOG_INFO("ESP32 link: %s", link_ok ? "ONLINE" : (ever_online ? "OFFLINE" : "PENDING"));
    KT_LOG_INFO("ESP32 eof errors: %lu", (unsigned long)eof_error_count);
    KT_LOG_INFO("ESP32 rx overflow: %lu ring pending: %u",
                (unsigned long)rx_overflow_count,
                (unsigned int)kt_ringbuf_available(&rx_ring));
    KT_LOG_INFO("ESP32 wifi: %s ssid=%s rssi=%d",
                wifi_state ? "connected" : "offline",
                wifi_ssid[0] ? wifi_ssid : "-",
                (int)wifi_rssi);
    KT_LOG_INFO("ESP32 last tx=%lu ms last rx=%lu ms",
                (unsigned long)last_tx_ms,
                (unsigned long)last_rx_ms);
    KT_LOG_INFO("ESP32 tx queue: pending=%u drop=%lu busy=%lu fail=%lu",
                (unsigned int)tx_count,
                (unsigned long)tx_drop_count,
                (unsigned long)tx_busy_count,
                (unsigned long)tx_fail_count);
    KT_LOG_INFO("ESP32 last tx type=%s result=%d",
                msg_type_text(last_tx_type),
                (int)last_tx_result);
}
