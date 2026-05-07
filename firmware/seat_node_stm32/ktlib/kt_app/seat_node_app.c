#include "seat_node_app.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_zigbee/kt_zigbee.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define ZB_RX_RING_SIZE 128U
#define ZB_PAYLOAD_SEAT_STATUS 0x10U
#define ZB_PAYLOAD_PING        0x01U
#define ZB_PAYLOAD_PONG        0x02U

typedef enum {
    ZB_WAIT_HEAD = 0,
    ZB_ADDR_L,
    ZB_ADDR_H,
    ZB_LEN,
    ZB_DATA,
    ZB_TAIL,
    ZB_RAW_DATA
} zb_parse_state_t;

static volatile uint8_t zb_rx_byte;
static uint8_t zb_rx_ring[ZB_RX_RING_SIZE];
static volatile uint8_t zb_rx_head;
static volatile uint8_t zb_rx_tail;
static zb_parse_state_t zb_parse_state;
static uint16_t zb_parse_addr;
static uint8_t zb_parse_len;
static uint8_t zb_parse_index;
static uint8_t zb_parse_payload[KT_ZIGBEE_MAX_PAYLOAD_LEN];
static uint16_t zb_recent_addr;
static uint8_t zb_recent_payload[KT_ZIGBEE_MAX_PAYLOAD_LEN];
static uint8_t zb_recent_len;
static uint32_t zb_rx_count;
static uint32_t zb_tx_count;
static uint32_t zb_rx_bytes;
static uint32_t zb_rx_overflow;
static uint32_t zb_len_error;
static uint32_t zb_tail_error;
static uint32_t zb_addr_mismatch;
static uint32_t zb_tx_fail_count;
static uint8_t zb_test_seq = 1U;
static int32_t seat1_hx711_offset;
static int32_t seat1_counts_per_gram_x100;
static uint8_t seat1_hx711_tared;
static uint32_t seat_last_poll_ms;
static uint32_t seat_last_report_ms;
static uint8_t seat_last_valid;
static uint8_t seat_candidate_valid;
static uint8_t seat_candidate_count;

typedef struct {
    uint32_t magic;
    int32_t offset;
    int32_t counts_per_gram_x100;
    uint32_t checksum;
} hx711_cal_flash_t;

typedef enum {
    SEAT_STATE_FREE = 0,
    SEAT_STATE_OCCUPIED,
    SEAT_STATE_UNKNOWN
} seat_state_t;

typedef struct {
    seat_state_t seat1;
    seat_state_t seat2;
    seat_state_t seat3;
    int32_t seat1_raw;
    int32_t seat1_delta;
    int32_t seat1_gram;
    GPIO_PinState seat1_ir;
    GPIO_PinState seat2_raw;
    GPIO_PinState seat3_raw;
} seat_snapshot_t;

static seat_snapshot_t seat_last_snapshot;
static seat_snapshot_t seat_candidate_snapshot;

static void seat_node_send_snapshot(const seat_snapshot_t *snapshot, uint8_t verbose);

static void zb_ring_clear(void)
{
    zb_rx_head = 0U;
    zb_rx_tail = 0U;
}

static uint8_t zb_ring_put(uint8_t b)
{
    uint8_t next = (uint8_t)(zb_rx_head + 1U);
    if (next >= ZB_RX_RING_SIZE) {
        next = 0U;
    }
    if (next == zb_rx_tail) {
        return 0U;
    }
    zb_rx_ring[zb_rx_head] = b;
    zb_rx_head = next;
    return 1U;
}

static uint8_t zb_ring_get(uint8_t *b)
{
    if (b == NULL || zb_rx_head == zb_rx_tail) {
        return 0U;
    }
    *b = zb_rx_ring[zb_rx_tail];
    zb_rx_tail++;
    if (zb_rx_tail >= ZB_RX_RING_SIZE) {
        zb_rx_tail = 0U;
    }
    return 1U;
}

static void seat_led_write(GPIO_TypeDef *port, uint16_t pin, uint8_t on)
{
    GPIO_PinState off = (SEAT_LED_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(port, pin, on ? SEAT_LED_ACTIVE_LEVEL : off);
}

static void seat_led_pair(seat_state_t state,
                          GPIO_TypeDef *red_port,
                          uint16_t red_pin,
                          GPIO_TypeDef *green_port,
                          uint16_t green_pin)
{
    if (state == SEAT_STATE_OCCUPIED) {
        seat_led_write(red_port, red_pin, 1U);
        seat_led_write(green_port, green_pin, 0U);
    } else if (state == SEAT_STATE_FREE) {
        seat_led_write(red_port, red_pin, 0U);
        seat_led_write(green_port, green_pin, 1U);
    } else {
        seat_led_write(red_port, red_pin, 0U);
        seat_led_write(green_port, green_pin, 0U);
    }
}

static void seat_led_apply(const seat_snapshot_t *snapshot)
{
    uint8_t weight_detected;

    if (snapshot == NULL) {
        return;
    }

    seat_led_pair(snapshot->seat1, SEAT1_LED_RED_PORT, SEAT1_LED_RED_PIN,
                  SEAT1_LED_GREEN_PORT, SEAT1_LED_GREEN_PIN);
    seat_led_pair(snapshot->seat2, SEAT2_LED_RED_PORT, SEAT2_LED_RED_PIN,
                  SEAT2_LED_GREEN_PORT, SEAT2_LED_GREEN_PIN);
    seat_led_pair(snapshot->seat3, SEAT3_LED_RED_PORT, SEAT3_LED_RED_PIN,
                  SEAT3_LED_GREEN_PORT, SEAT3_LED_GREEN_PIN);

    weight_detected = (snapshot->seat1 != SEAT_STATE_UNKNOWN &&
                       snapshot->seat1_gram > (int32_t)SEAT1_OCCUPIED_WEIGHT_GRAM) ? 1U : 0U;
    seat_led_write(SEAT_IR_LED_PORT, SEAT_IR_LED_PIN,
                   (snapshot->seat1_ir == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U);
    seat_led_write(SEAT_WEIGHT_LED_PORT, SEAT_WEIGHT_LED_PIN, weight_detected);
}

static void hx711_delay(void)
{
    volatile uint8_t i;
    for (i = 0U; i < 12U; i++) {
        __NOP();
    }
}

static int32_t abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static int32_t hx711_delta_to_gram(int32_t delta)
{
    if (seat1_counts_per_gram_x100 <= 0) {
        return 0;
    }
    return (abs_i32(delta) * 100L) / seat1_counts_per_gram_x100;
}

static uint32_t hx711_cal_checksum(uint32_t magic, int32_t offset, int32_t scale)
{
    return magic ^ (uint32_t)offset ^ (uint32_t)scale ^ 0xA55A5AA5UL;
}

static uint8_t hx711_load_calibration(void)
{
    const hx711_cal_flash_t *cal = (const hx711_cal_flash_t *)SEAT1_HX711_FLASH_ADDR;
    uint32_t checksum = hx711_cal_checksum(cal->magic, cal->offset, cal->counts_per_gram_x100);

    if (cal->magic != SEAT1_HX711_FLASH_MAGIC || cal->checksum != checksum) {
        return 0U;
    }

    seat1_hx711_offset = cal->offset;
    seat1_counts_per_gram_x100 = cal->counts_per_gram_x100;
    seat1_hx711_tared = 1U;
    KT_LOG_INFO("HX711 calib loaded from Flash");
    KT_LOG_INFO("HX711 offset=%ld scale=%ld counts/g x100",
                (long)seat1_hx711_offset, (long)seat1_counts_per_gram_x100);
    return 1U;
}

static uint8_t hx711_load_default_calibration(void)
{
#if (SEAT1_HX711_DEFAULT_ENABLE != 0U)
    if (SEAT1_HX711_DEFAULT_SCALE_X100 <= 0L) {
        return 0U;
    }

    seat1_hx711_offset = (int32_t)SEAT1_HX711_DEFAULT_OFFSET;
    seat1_counts_per_gram_x100 = (int32_t)SEAT1_HX711_DEFAULT_SCALE_X100;
    seat1_hx711_tared = 1U;
    KT_LOG_WARN("HX711 Flash calib invalid, using firmware defaults");
    KT_LOG_INFO("HX711 default offset=%ld scale=%ld counts/g x100",
                (long)seat1_hx711_offset, (long)seat1_counts_per_gram_x100);
    return 1U;
#else
    return 0U;
#endif
}

static uint8_t hx711_save_calibration(void)
{
    hx711_cal_flash_t cal;
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0U;
    uint32_t addr = SEAT1_HX711_FLASH_ADDR;
    const uint16_t *data = (const uint16_t *)&cal;
    uint32_t halfwords = sizeof(cal) / sizeof(uint16_t);
    uint32_t i;
    HAL_StatusTypeDef st;

    KT_LOG_INFO("HX711 calib saving to Flash...");
    cal.magic = SEAT1_HX711_FLASH_MAGIC;
    cal.offset = seat1_hx711_offset;
    cal.counts_per_gram_x100 = seat1_counts_per_gram_x100;
    cal.checksum = hx711_cal_checksum(cal.magic, cal.offset, cal.counts_per_gram_x100);

    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = SEAT1_HX711_FLASH_ADDR;
    erase.NbPages = 1U;
    st = HAL_FLASHEx_Erase(&erase, &page_error);
    if (st == HAL_OK) {
        for (i = 0U; i < halfwords; i++) {
            st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data[i]);
            if (st != HAL_OK) {
                break;
            }
            addr += 2U;
        }
    }

    HAL_FLASH_Lock();

    if (st != HAL_OK) {
        KT_LOG_WARN("HX711 calib Flash save failed");
        return 0U;
    }

    KT_LOG_INFO("HX711 calib saved to Flash: 0x%08lX", (unsigned long)SEAT1_HX711_FLASH_ADDR);
    return 1U;
}

static uint8_t hx711_wait_ready(void)
{
    uint32_t start_ms = HAL_GetTick();
    while (HAL_GPIO_ReadPin(SEAT1_HX711_DT_PORT, SEAT1_HX711_DT_PIN) == GPIO_PIN_SET) {
        if ((HAL_GetTick() - start_ms) >= (uint32_t)SEAT1_HX711_READY_TIMEOUT) {
            return 0U;
        }
    }
    return 1U;
}

static uint8_t hx711_read_raw(int32_t *value)
{
    uint32_t raw = 0U;
    uint8_t i;

    if (value == NULL || hx711_wait_ready() == 0U) {
        return 0U;
    }

    for (i = 0U; i < 24U; i++) {
        HAL_GPIO_WritePin(SEAT1_HX711_SCK_PORT, SEAT1_HX711_SCK_PIN, GPIO_PIN_SET);
        hx711_delay();
        raw = (raw << 1) | ((HAL_GPIO_ReadPin(SEAT1_HX711_DT_PORT, SEAT1_HX711_DT_PIN) == GPIO_PIN_SET) ? 1UL : 0UL);
        HAL_GPIO_WritePin(SEAT1_HX711_SCK_PORT, SEAT1_HX711_SCK_PIN, GPIO_PIN_RESET);
        hx711_delay();
    }

    /* 25th clock selects channel A gain 128 for the next conversion. */
    HAL_GPIO_WritePin(SEAT1_HX711_SCK_PORT, SEAT1_HX711_SCK_PIN, GPIO_PIN_SET);
    hx711_delay();
    HAL_GPIO_WritePin(SEAT1_HX711_SCK_PORT, SEAT1_HX711_SCK_PIN, GPIO_PIN_RESET);
    hx711_delay();

    if ((raw & 0x800000UL) != 0UL) {
        raw |= 0xFF000000UL;
    }
    *value = (int32_t)raw;
    return 1U;
}

static uint8_t hx711_read_average(int32_t *value, uint8_t samples)
{
    int32_t raw;
    int32_t sum = 0;
    uint8_t count = 0U;
    uint8_t i;

    if (value == NULL || samples == 0U) {
        return 0U;
    }

    for (i = 0U; i < samples; i++) {
        if (hx711_read_raw(&raw) != 0U) {
            sum += raw;
            count++;
        }
    }

    if (count == 0U) {
        return 0U;
    }

    *value = sum / (int32_t)count;
    return 1U;
}

static GPIO_PinState seat1_ir_raw(void)
{
    return HAL_GPIO_ReadPin(SEAT1_IR_PORT, SEAT1_IR_PIN);
}

static uint8_t seat1_ir_detected(void)
{
    return (seat1_ir_raw() == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U;
}

static seat_state_t seat1_hx711_state(int32_t *raw, int32_t *delta)
{
    int32_t current = 0;
    int32_t diff = 0;

    if (hx711_read_average(&current, 1U) == 0U) {
        if (raw != NULL) {
            *raw = 0;
        }
        if (delta != NULL) {
            *delta = 0;
        }
        return SEAT_STATE_UNKNOWN;
    }

    if (seat1_hx711_tared == 0U) {
        seat1_hx711_offset = current;
        seat1_hx711_tared = 1U;
    }

    diff = current - seat1_hx711_offset;
    if (raw != NULL) {
        *raw = current;
    }
    if (delta != NULL) {
        *delta = diff;
    }

    if (seat1_counts_per_gram_x100 > 0) {
        return (hx711_delta_to_gram(diff) > (int32_t)SEAT1_OCCUPIED_WEIGHT_GRAM) ? SEAT_STATE_OCCUPIED : SEAT_STATE_FREE;
    }

    return (abs_i32(diff) >= (int32_t)SEAT1_HX711_THRESHOLD) ? SEAT_STATE_OCCUPIED : SEAT_STATE_FREE;
}

static seat_state_t seat1_read_state(int32_t *raw, int32_t *delta)
{
    seat_state_t weight_state = seat1_hx711_state(raw, delta);

    if (weight_state == SEAT_STATE_UNKNOWN) {
        return SEAT_STATE_UNKNOWN;
    }

    if (weight_state == SEAT_STATE_OCCUPIED && seat1_ir_detected() != 0U) {
        return SEAT_STATE_OCCUPIED;
    }

    return SEAT_STATE_FREE;
}

static GPIO_PinState seat_gpio_raw(uint8_t seat)
{
    return (seat == 3U) ? HAL_GPIO_ReadPin(SEAT3_GPIO_PORT, SEAT3_GPIO_PIN)
                        : HAL_GPIO_ReadPin(SEAT2_GPIO_PORT, SEAT2_GPIO_PIN);
}

static seat_state_t seat_gpio_state(uint8_t seat)
{
    return (seat_gpio_raw(seat) == SEAT_GPIO_ACTIVE_LEVEL) ? SEAT_STATE_OCCUPIED : SEAT_STATE_FREE;
}

static const char *level_str(GPIO_PinState state)
{
    return (state == GPIO_PIN_SET) ? "1" : "0";
}

static const char *seat_status(seat_state_t state)
{
    if (state == SEAT_STATE_OCCUPIED) {
        return "OCCUPIED";
    }
    if (state == SEAT_STATE_FREE) {
        return "FREE";
    }
    return "UNKNOWN";
}

static void seat_read_snapshot(seat_snapshot_t *snapshot)
{
    if (snapshot == NULL) {
        return;
    }

    snapshot->seat1_raw = 0;
    snapshot->seat1_delta = 0;
    snapshot->seat1_gram = 0;
    snapshot->seat1 = seat1_read_state(&snapshot->seat1_raw, &snapshot->seat1_delta);
    snapshot->seat1_ir = seat1_ir_raw();
    snapshot->seat2_raw = seat_gpio_raw(2U);
    snapshot->seat3_raw = seat_gpio_raw(3U);
    snapshot->seat2 = (snapshot->seat2_raw == SEAT_GPIO_ACTIVE_LEVEL) ? SEAT_STATE_OCCUPIED : SEAT_STATE_FREE;
    snapshot->seat3 = (snapshot->seat3_raw == SEAT_GPIO_ACTIVE_LEVEL) ? SEAT_STATE_OCCUPIED : SEAT_STATE_FREE;

    if (snapshot->seat1 != SEAT_STATE_UNKNOWN && seat1_counts_per_gram_x100 > 0) {
        snapshot->seat1_gram = hx711_delta_to_gram(snapshot->seat1_delta);
    }
}

static void seat_print_change(uint8_t seat_id, seat_state_t old_state, seat_state_t new_state,
                              const seat_snapshot_t *snapshot)
{
    if (old_state == new_state) {
        return;
    }

    if (seat_id == 1U) {
        KT_LOG_INFO("Seat1 change: %s -> %s, weight=%ldg, ir=%u",
                    seat_status(old_state), seat_status(new_state),
                    (long)snapshot->seat1_gram,
                    (snapshot->seat1_ir == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U);
    } else {
        KT_LOG_INFO("Seat%u change: %s -> %s",
                    seat_id, seat_status(old_state), seat_status(new_state));
    }
}

static uint8_t seat_snapshot_state_equal(const seat_snapshot_t *a, const seat_snapshot_t *b)
{
    if (a == NULL || b == NULL) {
        return 0U;
    }

    return (a->seat1 == b->seat1 &&
            a->seat2 == b->seat2 &&
            a->seat3 == b->seat3) ? 1U : 0U;
}

static uint8_t seat_state_code(seat_state_t state)
{
    if (state == SEAT_STATE_OCCUPIED) {
        return 1U;
    }
    if (state == SEAT_STATE_UNKNOWN) {
        return 2U;
    }
    return 0U;
}

static void seat_poll_changes(void)
{
    uint32_t now = HAL_GetTick();
    seat_snapshot_t snapshot;

    if ((now - seat_last_poll_ms) < (uint32_t)SEAT_NODE_POLL_PERIOD_MS) {
        return;
    }
    seat_last_poll_ms = now;

    seat_read_snapshot(&snapshot);

    if (seat_last_valid == 0U) {
        seat_last_snapshot = snapshot;
        seat_last_valid = 1U;
        seat_candidate_valid = 0U;
        seat_candidate_count = 0U;
        seat_led_apply(&seat_last_snapshot);
        seat_node_send_snapshot(&seat_last_snapshot, 1U);
        seat_last_report_ms = now;
        return;
    }

    if (seat_snapshot_state_equal(&seat_last_snapshot, &snapshot) != 0U) {
        seat_candidate_valid = 0U;
        seat_candidate_count = 0U;
        if ((now - seat_last_report_ms) >= (uint32_t)SEAT_NODE_KEEPALIVE_REPORT_MS) {
            seat_node_send_snapshot(&seat_last_snapshot, 0U);
            seat_last_report_ms = now;
        }
        return;
    }

    if (seat_candidate_valid == 0U ||
        seat_snapshot_state_equal(&seat_candidate_snapshot, &snapshot) == 0U) {
        seat_candidate_snapshot = snapshot;
        seat_candidate_valid = 1U;
        seat_candidate_count = 1U;
        return;
    }

    if (seat_candidate_count < SEAT_NODE_STATE_CONFIRM_COUNT) {
        seat_candidate_count++;
    }

    if (seat_candidate_count >= SEAT_NODE_STATE_CONFIRM_COUNT) {
        seat_print_change(1U, seat_last_snapshot.seat1, snapshot.seat1, &snapshot);
        seat_print_change(2U, seat_last_snapshot.seat2, snapshot.seat2, &snapshot);
        seat_print_change(3U, seat_last_snapshot.seat3, snapshot.seat3, &snapshot);

        seat_last_snapshot = snapshot;
        seat_candidate_valid = 0U;
        seat_candidate_count = 0U;
        seat_led_apply(&seat_last_snapshot);
        seat_node_send_snapshot(&seat_last_snapshot, 1U);
        seat_last_report_ms = now;
    }
}

static void zigbee_send_to_main(const uint8_t *data, uint8_t len, uint8_t verbose)
{
    HAL_StatusTypeDef st;

    st = kt_zigbee_send_to(ZIGBEE_ADDR_MAIN_CTRL, data, len);
    if (st != HAL_OK) {
        zb_tx_fail_count++;
        KT_LOG_WARN("ZigBee TX FAIL");
    } else {
        zb_tx_count++;
        if (verbose != 0U) {
            KT_LOG_INFO("ZigBee TX to 0x%04X: type=0x%02X len=%u",
                        ZIGBEE_ADDR_MAIN_CTRL,
                        (unsigned int)data[0],
                        (unsigned int)len);
        }
    }
}

static void zigbee_handle_payload(uint16_t addr, const uint8_t *data, uint8_t len)
{
    if (len == 0U) {
        return;
    }

    zb_recent_addr = addr;
    zb_recent_len = len;
    memcpy(zb_recent_payload, data, len);
    zb_rx_count++;
    if (addr != ZIGBEE_ADDR_SELF) {
        zb_addr_mismatch++;
    }

    if (data[0] == ZB_PAYLOAD_PING && len >= 2U) {
        uint8_t pong[2] = {ZB_PAYLOAD_PONG, data[1]};
        KT_LOG_INFO("ZigBee RX PING seq=%u", (unsigned int)data[1]);
        zigbee_send_to_main(pong, sizeof(pong), 1U);
    } else if (data[0] == ZB_PAYLOAD_PONG && len >= 2U) {
        KT_LOG_INFO("ZigBee RX PONG seq=%u", (unsigned int)data[1]);
    } else {
        KT_LOG_INFO("ZigBee RX type=0x%02X len=%u",
                    (unsigned int)data[0], (unsigned int)len);
    }
}

static void zigbee_parse_byte(uint8_t b)
{
    switch (zb_parse_state) {
    case ZB_WAIT_HEAD:
        if (b == KT_ZIGBEE_FRAME_HEAD) {
            zb_parse_state = ZB_ADDR_L;
            zb_parse_addr = 0U;
            zb_parse_len = 0U;
            zb_parse_index = 0U;
        } else if (b == ZB_PAYLOAD_PING || b == ZB_PAYLOAD_PONG) {
            zb_parse_addr = ZIGBEE_ADDR_SELF;
            zb_parse_len = 2U;
            zb_parse_index = 1U;
            zb_parse_payload[0] = b;
            zb_parse_state = ZB_RAW_DATA;
        } else if (b == ZB_PAYLOAD_SEAT_STATUS) {
            zb_parse_addr = ZIGBEE_ADDR_SELF;
            zb_parse_len = 4U;
            zb_parse_index = 1U;
            zb_parse_payload[0] = b;
            zb_parse_state = ZB_RAW_DATA;
        }
        break;
    case ZB_ADDR_L:
        zb_parse_addr = b;
        zb_parse_state = ZB_ADDR_H;
        break;
    case ZB_ADDR_H:
        zb_parse_addr |= ((uint16_t)b << 8U);
        zb_parse_state = ZB_LEN;
        break;
    case ZB_LEN:
        zb_parse_len = b;
        zb_parse_index = 0U;
        if (zb_parse_len == 0U || zb_parse_len > KT_ZIGBEE_MAX_PAYLOAD_LEN) {
            zb_len_error++;
            zb_parse_state = ZB_WAIT_HEAD;
        } else {
            zb_parse_state = ZB_DATA;
        }
        break;
    case ZB_DATA:
        zb_parse_payload[zb_parse_index++] = b;
        if (zb_parse_index >= zb_parse_len) {
            zb_parse_state = ZB_TAIL;
        }
        break;
    case ZB_TAIL:
        if (b == KT_ZIGBEE_FRAME_TAIL) {
            zigbee_handle_payload(zb_parse_addr, zb_parse_payload, zb_parse_len);
        } else {
            zb_tail_error++;
        }
        zb_parse_state = ZB_WAIT_HEAD;
        break;
    case ZB_RAW_DATA:
        zb_parse_payload[zb_parse_index++] = b;
        if (zb_parse_index >= zb_parse_len) {
            zigbee_handle_payload(zb_parse_addr, zb_parse_payload, zb_parse_len);
            zb_parse_state = ZB_WAIT_HEAD;
        }
        break;
    default:
        zb_parse_state = ZB_WAIT_HEAD;
        break;
    }
}

void seat_node_app_init(void)
{
    int32_t offset = 0;

    zb_ring_clear();
    zb_parse_state = ZB_WAIT_HEAD;
    zb_parse_addr = 0U;
    zb_parse_len = 0U;
    zb_parse_index = 0U;
    zb_rx_count = 0U;
    zb_tx_count = 0U;
    zb_rx_bytes = 0U;
    zb_rx_overflow = 0U;
    zb_len_error = 0U;
    zb_tail_error = 0U;
    zb_addr_mismatch = 0U;
    zb_tx_fail_count = 0U;
    zb_recent_len = 0U;
    seat_last_valid = 0U;
    seat_candidate_valid = 0U;
    seat_candidate_count = 0U;
    seat_last_poll_ms = 0U;
    seat_last_report_ms = 0U;
    seat1_counts_per_gram_x100 = 0;
    seat1_hx711_tared = 0U;
    HAL_GPIO_WritePin(SEAT1_HX711_SCK_PORT, SEAT1_HX711_SCK_PIN, GPIO_PIN_RESET);
    if (hx711_load_calibration() != 0U) {
        /* Calibration loaded from Flash. */
    } else if (hx711_load_default_calibration() != 0U) {
        /* Calibration loaded from firmware defaults. */
    } else if (hx711_read_average(&offset, SEAT1_HX711_TARE_SAMPLES) != 0U) {
        seat1_hx711_offset = offset;
        seat1_hx711_tared = 1U;
        KT_LOG_INFO("Seat1 HX711 tare: %ld", (long)seat1_hx711_offset);
    } else {
        KT_LOG_WARN("Seat1 HX711 not ready");
    }
    kt_zigbee_init(&huart1, 0);
    (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zb_rx_byte, 1);
    KT_LOG_INFO("Seat node app init: OK");
    KT_LOG_INFO("USART1 ZigBee UART init: OK, link pending");
}

void seat_node_app_task(void)
{
    uint8_t b;
    uint8_t rx_budget = SEAT_NODE_ZIGBEE_RX_BYTES_PER_TASK;

    seat_poll_changes();

    while (rx_budget > 0U && zb_ring_get(&b) != 0U) {
        zb_rx_bytes++;
        zigbee_parse_byte(b);
        rx_budget--;
    }
}

void seat_node_zigbee_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (zb_ring_put(zb_rx_byte) == 0U) {
            zb_rx_overflow++;
        }
        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zb_rx_byte, 1);
    }
}

void seat_node_print_hardware_status(void)
{
    KT_LOG_INFO("Seat node hardware");
    KT_LOG_INFO("Seat1: HX711 DT=PA4 SCK=PA5 + IR PB9");
    KT_LOG_INFO("Seat1: weight>%ldg and IR active = OCCUPIED", (long)SEAT1_OCCUPIED_WEIGHT_GRAM);
    KT_LOG_INFO("Seat2: PB8 input pull-down, high=OCCUPIED");
    KT_LOG_INFO("Seat3: PB7 input pull-down, high=OCCUPIED");
    KT_LOG_INFO("Seat LEDs: S1 PB5/PB6, S2 PB3/PB4, S3 PA12/PA15");
    KT_LOG_INFO("Detect LEDs: IR PB14, weight PB15");
    KT_LOG_INFO("Seat poll: %u ms, confirm=%u samples", (unsigned int)SEAT_NODE_POLL_PERIOD_MS, (unsigned int)SEAT_NODE_STATE_CONFIRM_COUNT);
    KT_LOG_INFO("ZigBee report: boot/change plus %u ms keepalive, dst=0x%04X",
                (unsigned int)SEAT_NODE_KEEPALIVE_REPORT_MS,
                ZIGBEE_ADDR_MAIN_CTRL);
    KT_LOG_INFO("ZigBee : USART1 PA9/PA10 38400");
    KT_LOG_INFO("Debug  : USART2 PA2/PA3 115200 FF CMD DATA FF");
}

void seat_node_print_uart_roles(void)
{
    KT_LOG_INFO("USART1: ZigBee CC2530 point-to-point UART");
    KT_LOG_INFO("USART2: Debug UART, FF CMD DATA FF");
}

void seat_node_print_zigbee_info(void)
{
    KT_LOG_INFO("ZigBee seat node link:");
    KT_LOG_INFO("role=end-device self=0x%04X peer main=0x%04X",
                (unsigned int)ZIGBEE_ADDR_SELF,
                (unsigned int)ZIGBEE_ADDR_MAIN_CTRL);
    KT_LOG_INFO("UART=USART1 PA9/TX PA10/RX 38400 8N1");
    KT_LOG_INFO("frame=FA ADDRL ADDRH LEN DATA F5");
    KT_LOG_INFO("TX count=%lu TX fail=%lu RX frame ok=%lu",
                (unsigned long)zb_tx_count,
                (unsigned long)zb_tx_fail_count,
                (unsigned long)zb_rx_count);
    KT_LOG_INFO("RX bytes=%lu overflow=%lu len_err=%lu tail_err=%lu addr_mismatch=%lu",
                (unsigned long)zb_rx_bytes,
                (unsigned long)zb_rx_overflow,
                (unsigned long)zb_len_error,
                (unsigned long)zb_tail_error,
                (unsigned long)zb_addr_mismatch);
    if (zb_recent_len == 0U) {
        KT_LOG_WARN("recent payload empty");
    } else {
        KT_LOG_INFO("recent addr=0x%04X payload len=%u hex=%02X %02X %02X %02X",
                    (unsigned int)zb_recent_addr,
                    (unsigned int)zb_recent_len,
                    (unsigned int)zb_recent_payload[0],
                    (unsigned int)((zb_recent_len > 1U) ? zb_recent_payload[1] : 0U),
                    (unsigned int)((zb_recent_len > 2U) ? zb_recent_payload[2] : 0U),
                    (unsigned int)((zb_recent_len > 3U) ? zb_recent_payload[3] : 0U));
    }
    KT_LOG_INFO("test: FF 80 00 FF sends binary SEAT to 0x%04X", (unsigned int)ZIGBEE_ADDR_MAIN_CTRL);
    KT_LOG_INFO("test: FF 81/83 sends binary PONG to 0x%04X", (unsigned int)ZIGBEE_ADDR_MAIN_CTRL);
}

void seat_node_print_raw_levels(void)
{
    seat_snapshot_t snapshot;

    seat_read_snapshot(&snapshot);

    if (snapshot.seat1 == SEAT_STATE_UNKNOWN) {
        KT_LOG_WARN("Seat1 HX711 raw: NOT_READY");
    } else {
        KT_LOG_INFO("Seat1 HX711 raw=%ld delta=%ld", (long)snapshot.seat1_raw, (long)snapshot.seat1_delta);
        if (seat1_counts_per_gram_x100 > 0) {
            KT_LOG_INFO("Seat1 weight=%ld g", (long)snapshot.seat1_gram);
        } else {
            KT_LOG_INFO("Seat1 weight: not calibrated");
        }
    }
    KT_LOG_INFO("Seat1 IR PB9 raw=%s detected=%u", level_str(snapshot.seat1_ir), (snapshot.seat1_ir == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U);
    KT_LOG_INFO("Seat2 PB8 raw=%s occupied=%u", level_str(snapshot.seat2_raw), (snapshot.seat2_raw == SEAT_GPIO_ACTIVE_LEVEL) ? 1U : 0U);
    KT_LOG_INFO("Seat3 PB7 raw=%s occupied=%u", level_str(snapshot.seat3_raw), (snapshot.seat3_raw == SEAT_GPIO_ACTIVE_LEVEL) ? 1U : 0U);
}

void seat_node_print_status(void)
{
    seat_snapshot_t snapshot;

    seat_read_snapshot(&snapshot);
    KT_LOG_INFO("Seat1: %s, weight=%ldg, ir=%u",
                seat_status(snapshot.seat1),
                (long)snapshot.seat1_gram,
                (snapshot.seat1_ir == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U);
    KT_LOG_INFO("Seat2: %s", seat_status(snapshot.seat2));
    KT_LOG_INFO("Seat3: %s", seat_status(snapshot.seat3));
    KT_LOG_INFO("SEAT: %s,%s,%s", seat_status(snapshot.seat1), seat_status(snapshot.seat2), seat_status(snapshot.seat3));
}

void seat_node_print_led_map(void)
{
    KT_LOG_INFO("Seat LED map:");
    KT_LOG_INFO("S1 red=PB5 green=PB6");
    KT_LOG_INFO("S2 red=PB3 green=PB4");
    KT_LOG_INFO("S3 red=PA12 green=PA15");
    KT_LOG_INFO("IR detected LED=PB14");
    KT_LOG_INFO("Weight > %ldg LED=PB15", (long)SEAT1_OCCUPIED_WEIGHT_GRAM);
    KT_LOG_INFO("FREE: green on, OCCUPIED: red on, UNKNOWN: both off");
}

void seat_node_refresh_and_print_leds(void)
{
    seat_snapshot_t snapshot;

    seat_read_snapshot(&snapshot);
    seat_led_apply(&snapshot);
    KT_LOG_INFO("LED state refreshed");
    KT_LOG_INFO("Seat1=%s ir=%u weight=%ldg",
                seat_status(snapshot.seat1),
                (snapshot.seat1_ir == SEAT1_IR_ACTIVE_LEVEL) ? 1U : 0U,
                (long)snapshot.seat1_gram);
    KT_LOG_INFO("Seat2=%s raw=%s", seat_status(snapshot.seat2), level_str(snapshot.seat2_raw));
    KT_LOG_INFO("Seat3=%s raw=%s", seat_status(snapshot.seat3), level_str(snapshot.seat3_raw));
}

void seat_node_tare_hx711(void)
{
    int32_t offset = 0;

    if (hx711_read_average(&offset, SEAT1_HX711_TARE_SAMPLES) == 0U) {
        KT_LOG_WARN("HX711 tare failed: NOT_READY");
        return;
    }

    seat1_hx711_offset = offset;
    seat1_hx711_tared = 1U;
    KT_LOG_INFO("HX711 tare OK: offset=%ld", (long)seat1_hx711_offset);
    (void)hx711_save_calibration();
}

void seat_node_calibrate_hx711(uint8_t units_10g)
{
    int32_t raw = 0;
    int32_t delta = 0;
    int32_t counts = 0;
    uint32_t known_grams;

    if (units_10g == 0U) {
        KT_LOG_WARN("HX711 calib rejected: weight is 0");
        KT_LOG_INFO("Use FF 33 NN FF, NN means NN*10g");
        return;
    }

    if (seat1_hx711_state(&raw, &delta) == SEAT_STATE_UNKNOWN) {
        KT_LOG_WARN("HX711 calib failed: NOT_READY");
        return;
    }

    counts = abs_i32(delta);
    if (counts == 0) {
        KT_LOG_WARN("HX711 calib failed: delta is 0");
        return;
    }

    known_grams = (uint32_t)units_10g * (uint32_t)SEAT1_HX711_CAL_UNIT_GRAM;
    seat1_counts_per_gram_x100 = (int32_t)(((uint32_t)counts * 100UL) / known_grams);
    KT_LOG_INFO("HX711 calib OK: known=%lu g", (unsigned long)known_grams);
    KT_LOG_INFO("HX711 scale=%ld counts/g x100", (long)seat1_counts_per_gram_x100);
    (void)hx711_save_calibration();
}

void seat_node_print_hx711_weight(void)
{
    int32_t raw = 0;
    int32_t delta = 0;
    int32_t gram = 0;

    if (seat1_hx711_state(&raw, &delta) == SEAT_STATE_UNKNOWN) {
        KT_LOG_WARN("HX711 weight: NOT_READY");
        return;
    }

    KT_LOG_INFO("HX711 raw=%ld delta=%ld", (long)raw, (long)delta);
    if (seat1_counts_per_gram_x100 <= 0) {
        KT_LOG_WARN("HX711 weight: not calibrated");
        KT_LOG_INFO("Tare empty seat: FF 32 00 FF");
        KT_LOG_INFO("Calibrate: put known weight, send FF 33 NN FF, NN=10g units");
        return;
    }

    gram = hx711_delta_to_gram(delta);
    KT_LOG_INFO("HX711 weight=%ld g", (long)gram);
}

static void seat_node_send_snapshot(const seat_snapshot_t *snapshot, uint8_t verbose)
{
    uint8_t msg[4];

    msg[0] = ZB_PAYLOAD_SEAT_STATUS;
    msg[1] = seat_state_code(snapshot->seat1);
    msg[2] = seat_state_code(snapshot->seat2);
    msg[3] = seat_state_code(snapshot->seat3);
    zigbee_send_to_main(msg, sizeof(msg), verbose);
}

void seat_node_send_zigbee_status(void)
{
    seat_snapshot_t snapshot;

    seat_read_snapshot(&snapshot);
    seat_node_send_snapshot(&snapshot, 1U);
}

void seat_node_send_zigbee_pong(void)
{
    uint8_t msg[2];

    msg[0] = ZB_PAYLOAD_PONG;
    msg[1] = zb_test_seq++;
    zigbee_send_to_main(msg, sizeof(msg), 1U);
}
