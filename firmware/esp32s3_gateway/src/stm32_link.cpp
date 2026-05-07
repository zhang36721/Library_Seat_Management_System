#include "stm32_link.h"
#include "cloud_client.h"
#include "device_state.h"

static HardwareSerial Stm32Serial(1);
static uint16_t tx_seq = 1;
static bool stm32_is_online = false;
static bool offline_reported = false;
static uint32_t last_valid_stm32_rx_ms = 0;
constexpr uint32_t STM32_RX_OFFLINE_TIMEOUT_MS = 15000UL;

enum RxState { WAIT_SOF1, WAIT_SOF2, HEADER, PAYLOAD, CRC1, CRC2, EOF_BYTE };
static RxState rx_state = WAIT_SOF1;
static uint8_t header[6];
static uint8_t header_pos = 0;
static uint8_t payload[KT_BIN_MAX_PAYLOAD];
static uint16_t payload_pos = 0;
static uint16_t payload_len = 0;
static uint16_t rx_crc = 0;

static bool is_heartbeat_type(uint8_t type);

static void mark_stm32_online()
{
    const bool was_online = stm32_is_online;
    stm32_is_online = true;
    device_state_set_stm32_online(true);
    offline_reported = false;
    last_valid_stm32_rx_ms = millis();
    if (!was_online) {
        Serial.println("[STM32] online");
    }
}

static uint16_t send_frame(uint8_t type, const uint8_t *data, uint16_t len)
{
    uint8_t frame[8 + KT_BIN_MAX_PAYLOAD + 3];
    uint16_t pos = 0;
    uint16_t seq = tx_seq++;
    frame[pos++] = KT_BIN_SOF1;
    frame[pos++] = KT_BIN_SOF2;
    frame[pos++] = KT_BIN_VERSION;
    frame[pos++] = type;
    kt_put_u16_le(&frame[pos], seq);
    pos += 2;
    kt_put_u16_le(&frame[pos], len);
    pos += 2;
    if (len && data) {
        memcpy(&frame[pos], data, len);
        pos += len;
    }
    uint16_t crc = kt_crc16_modbus(&frame[2], 6 + len);
    kt_put_u16_le(&frame[pos], crc);
    pos += 2;
    frame[pos++] = KT_BIN_EOF;
    Stm32Serial.write(frame, pos);
    return seq;
}

static void send_ack(uint16_t ack_seq, uint8_t ack_type)
{
#if UART_VERBOSE_LOG
    uint8_t p[3];
    kt_put_u16_le(p, ack_seq);
    p[2] = ack_type;
    (void)send_frame(KT_MSG_ACK, p, sizeof(p));
    if (!is_heartbeat_type(ack_type)) {
        Serial.printf("[UART TX] ACK seq=%u type=%s\n", ack_seq, kt_msg_type_name(ack_type));
    }
#else
    (void)ack_seq;
    (void)ack_type;
#endif
}

static void send_err(uint16_t err_seq, uint8_t err_type, uint8_t reason)
{
    if (UART_VERBOSE_LOG) {
        Serial.printf("[UART RX] drop bad frame seq=%u type=%s reason=%u\n",
                      err_seq, kt_msg_type_name(err_type), reason);
    }
}

static void send_pong(uint16_t ping_seq)
{
    WifiStatus status = wifi_manager_status();
    uint8_t p[5];
    kt_put_u32_le(p, millis());
    p[4] = status.connected ? 1 : 0;
    (void)send_frame(KT_MSG_PONG, p, sizeof(p));
    Serial.printf("[UART TX] type=PONG seq=%u wifi=%s\n",
                  ping_seq, status.connected ? "connected" : "offline");
}

void stm32_link_send_wifi_status(const WifiStatus &status)
{
    (void)status;
#if UART_VERBOSE_LOG
    uint8_t p[39]{};
    p[0] = status.connected ? 1 : 0;
    p[1] = static_cast<uint8_t>(status.rssi);
    uint8_t ssid_len = status.ssid.length() > 32 ? 32 : static_cast<uint8_t>(status.ssid.length());
    p[2] = ssid_len;
    memcpy(&p[3], status.ssid.c_str(), ssid_len);
    memcpy(&p[35], status.ip, 4);
    (void)send_frame(KT_MSG_WIFI_STATUS, p, sizeof(p));
    Serial.printf("[UART TX] type=WIFI_STATUS state=%u ssid=%s\n", p[0], status.ssid.c_str());
#endif
}

static const char *access_type_text(uint8_t type)
{
    if (type == 0) return "CHECK_IN";
    if (type == 1) return "CHECK_OUT";
    return "DENIED";
}

static bool is_heartbeat_type(uint8_t type)
{
    return type == KT_MSG_HEARTBEAT || type == KT_MSG_HEARTBEAT_ACK;
}

static void handle_card_event(uint16_t seq, const uint8_t *p, uint16_t len)
{
    if (len < 12) {
        send_err(seq, KT_MSG_CARD_EVENT, KT_ERR_LEN_ERR);
        return;
    }
    Serial.printf("[CARD] uid=%02X %02X %02X %02X type=%s allowed=%u time=20%02u-%02u-%02u %02u:%02u:%02u\n",
                  p[0], p[1], p[2], p[3],
                  access_type_text(p[4]), p[5],
                  p[6], p[7], p[8], p[9], p[10], p[11]);
    device_state_add_card_event(p, len);
    CardEvent event;
    if (device_state_get_latest_card_event(event)) {
        cloud_client_request_card_event_upload(event);
    }
    send_ack(seq, KT_MSG_CARD_EVENT);
}

static void handle_ack(const uint8_t *p, uint16_t len)
{
    if (len < 3) {
        return;
    }
    uint16_t ack_seq = kt_get_u16_le(p);
    uint8_t ack_type = p[2];
    if (UART_VERBOSE_LOG) {
        Serial.printf("[UART RX] ACK seq=%u type=%s\n", ack_seq, kt_msg_type_name(ack_type));
    }
}

static void handle_device_status(uint8_t type, const uint8_t *p, uint16_t len)
{
    if (len < 15) {
        send_err(0, type, KT_ERR_LEN_ERR);
        return;
    }
    mark_stm32_online();
    if (UART_VERBOSE_LOG) {
        Serial.printf("[SYNC RX] cards=%u logs=%u time=20%02u-%02u-%02u %02u:%02u:%02u rc522=%s\n",
                      p[4], p[5], p[9], p[10], p[11], p[12], p[13], p[14],
                      p[7] ? "OK" : "PENDING");
    }
    device_state_update_device_status(p, len);
    cloud_client_request_device_status_upload();
}

static void handle_frame(uint8_t type, uint16_t seq, const uint8_t *p, uint16_t len)
{
    if (UART_VERBOSE_LOG && !is_heartbeat_type(type)) {
        Serial.printf("[UART RX] type=%s seq=%u crc=OK\n", kt_msg_type_name(type), seq);
    }
    if (type == KT_MSG_PING) {
        mark_stm32_online();
        send_pong(seq);
    } else if (type == KT_MSG_HEARTBEAT_ACK || type == KT_MSG_ACK) {
        handle_ack(p, len);
    } else if (type == KT_MSG_CARD_EVENT) {
        mark_stm32_online();
        handle_card_event(seq, p, len);
    } else if (type == KT_MSG_DEVICE_STATUS || type == KT_MSG_BOOT_SYNC) {
        handle_device_status(type, p, len);
    } else {
        send_err(seq, type, KT_ERR_TYPE_ERR);
    }
}

static void reset_rx()
{
    rx_state = WAIT_SOF1;
    header_pos = 0;
    payload_pos = 0;
    payload_len = 0;
    rx_crc = 0;
}

static void parse_byte(uint8_t b)
{
    uint8_t crc_buf[6 + KT_BIN_MAX_PAYLOAD];
    switch (rx_state) {
    case WAIT_SOF1:
        if (b == KT_BIN_SOF1) rx_state = WAIT_SOF2;
        break;
    case WAIT_SOF2:
        if (b == KT_BIN_SOF2) {
            rx_state = HEADER;
        } else if (b == KT_BIN_SOF1) {
            rx_state = WAIT_SOF2;
        } else {
            rx_state = WAIT_SOF1;
        }
        header_pos = 0;
        break;
    case HEADER:
        header[header_pos++] = b;
        if (header_pos >= 6) {
            payload_len = kt_get_u16_le(&header[4]);
            if (header[0] != KT_BIN_VERSION || payload_len > KT_BIN_MAX_PAYLOAD) {
                send_err(kt_get_u16_le(&header[2]), header[1], KT_ERR_FORMAT_ERR);
                reset_rx();
            } else {
                rx_state = payload_len ? PAYLOAD : CRC1;
            }
        }
        break;
    case PAYLOAD:
        payload[payload_pos++] = b;
        if (payload_pos >= payload_len) rx_state = CRC1;
        break;
    case CRC1:
        rx_crc = b;
        rx_state = CRC2;
        break;
    case CRC2: {
        rx_crc |= static_cast<uint16_t>(b) << 8;
        rx_state = EOF_BYTE;
        break;
    }
    case EOF_BYTE: {
        uint16_t seq = kt_get_u16_le(&header[2]);
        if (b != KT_BIN_EOF) {
#if UART_VERBOSE_LOG
            Serial.printf("[UART RX] EOF_FAIL seq=%u eof=0x%02X\n", seq, b);
#endif
            send_err(seq, header[1], KT_ERR_FORMAT_ERR);
            reset_rx();
            break;
        }
        memcpy(crc_buf, header, 6);
        if (payload_len) memcpy(&crc_buf[6], payload, payload_len);
        uint16_t calc = kt_crc16_modbus(crc_buf, 6 + payload_len);
        if (calc == rx_crc) {
            handle_frame(header[1], seq, payload, payload_len);
        } else {
#if UART_VERBOSE_LOG
            Serial.printf("[UART RX] CRC_FAIL seq=%u\n", seq);
#endif
            send_err(seq, header[1], KT_ERR_CRC_FAIL);
        }
        reset_rx();
        break;
    }
    }
}

void stm32_link_begin()
{
    Stm32Serial.setRxBufferSize(1024);
    Stm32Serial.begin(115200, SERIAL_8N1, 48, 47);
    Serial.println("[UART] STM32 link on RX=GPIO48 TX=GPIO47 baud=115200");
}

static void drain_rx()
{
    while (Stm32Serial.available() > 0) {
        parse_byte(static_cast<uint8_t>(Stm32Serial.read()));
    }
}

void stm32_link_task()
{
    const uint32_t now = millis();
    drain_rx();

    if (stm32_is_online && last_valid_stm32_rx_ms != 0 &&
        now - last_valid_stm32_rx_ms > STM32_RX_OFFLINE_TIMEOUT_MS) {
        stm32_is_online = false;
        device_state_set_stm32_online(false);
        if (!offline_reported) {
            Serial.println("[STM32] offline");
            offline_reported = true;
        }
    }

    drain_rx();
}

bool stm32_link_online()
{
    return stm32_is_online;
}
