#include "kt_uart_links.h"
#include "main_controller_app.h"
#include "kt_config.h"
#include "kt_zigbee/kt_zigbee.h"
#include "kt_log.h"
#include "kt_components/kt_ringbuf.h"
#include "kt_system/kt_system_health.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define ZIGBEE_RX_RING_SIZE 256U
#define ZIGBEE_PAYLOAD_SEAT_STATUS 0x10U
#define ZIGBEE_PAYLOAD_PING        0x01U
#define ZIGBEE_PAYLOAD_PONG        0x02U

typedef enum {
    ZB_WAIT_HEAD = 0,
    ZB_ADDR_L,
    ZB_ADDR_H,
    ZB_LEN,
    ZB_DATA,
    ZB_TAIL
} zigbee_rx_state_t;

static volatile uint8_t zigbee_rx_byte;
static kt_ringbuf_t zigbee_rx_ring;
static uint8_t zigbee_rx_storage[ZIGBEE_RX_RING_SIZE];
static zigbee_rx_state_t zb_state;
static uint16_t zb_addr;
static uint8_t zb_len;
static uint8_t zb_index;
static uint8_t zb_payload[KT_ZIGBEE_MAX_PAYLOAD_LEN];
static uint8_t last_payload[KT_ZIGBEE_MAX_PAYLOAD_LEN];
static uint8_t last_payload_len;
static uint16_t last_addr;
static uint32_t zigbee_rx_count;
static uint32_t zigbee_tx_count;
static uint32_t zigbee_seat_rx_count;
static uint32_t zigbee_rx_bytes;
static uint32_t zigbee_len_error;
static uint32_t zigbee_tail_error;
static uint32_t zigbee_overflow;
static uint32_t zigbee_addr_mismatch;
static uint32_t zigbee_tx_fail_count;
static uint8_t zigbee_ping_seq = 1U;
static uint8_t last_seat_payload[4];
static uint8_t last_seat_valid;

static void zigbee_send_payload(uint16_t dst_addr, const uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef st;

    st = kt_zigbee_send_to(dst_addr, data, len);
    if (st == HAL_OK) {
        zigbee_tx_count++;
    } else {
        zigbee_tx_fail_count++;
        KT_LOG_WARN("ZigBee TX FAIL to 0x%04X", dst_addr);
    }
}

static void zigbee_handle_payload(uint16_t addr, const uint8_t *data, uint8_t len)
{
    uint8_t free_count;

    if (len == 0U) {
        return;
    }

    last_addr = addr;
    last_payload_len = len;
    memcpy(last_payload, data, len);
    zigbee_rx_count++;
    if (addr != ZIGBEE_ADDR_SELF) {
        zigbee_addr_mismatch++;
    }

    if (data[0] == ZIGBEE_PAYLOAD_PING && len >= 2U) {
        uint8_t p[2] = {ZIGBEE_PAYLOAD_PONG, data[1]};
        KT_LOG_INFO("[ZIGBEE] RX PING seq=%u", (unsigned int)data[1]);
        zigbee_send_payload(ZIGBEE_ADDR_SEAT_NODE, p, sizeof(p));
        return;
    }

    if (data[0] == ZIGBEE_PAYLOAD_PONG && len >= 2U) {
        KT_LOG_INFO("[ZIGBEE] RX PONG seq=%u", (unsigned int)data[1]);
        return;
    }

    if (data[0] != ZIGBEE_PAYLOAD_SEAT_STATUS || len < 4U) {
        KT_LOG_WARN("[ZIGBEE] RX unknown payload: type=0x%02X len=%u",
                    (unsigned int)data[0], (unsigned int)len);
        return;
    }
    if (data[1] > 2U || data[2] > 2U || data[3] > 2U) {
        KT_LOG_WARN("[ZIGBEE] Invalid seat payload: %u %u %u",
                    (unsigned int)data[1], (unsigned int)data[2], (unsigned int)data[3]);
        return;
    }

    last_seat_payload[0] = data[0];
    last_seat_payload[1] = data[1];
    last_seat_payload[2] = data[2];
    last_seat_payload[3] = data[3];
    last_seat_valid = 1U;
    zigbee_seat_rx_count++;
    free_count = ((data[1] == 0U) ? 1U : 0U) +
                 ((data[2] == 0U) ? 1U : 0U) +
                 ((data[3] == 0U) ? 1U : 0U);

    KT_LOG_INFO("[ZIGBEE] RX SEAT: S1=%u S2=%u S3=%u FREE=%u",
                (unsigned int)data[1], (unsigned int)data[2], (unsigned int)data[3],
                (unsigned int)free_count);
    if (main_controller_app_update_seat_states(data[1], data[2], data[3]) != 0U) {
        KT_LOG_INFO("[ZIGBEE] Seats updated");
    }
}

static void zigbee_parse_byte(uint8_t b)
{
    switch (zb_state) {
    case ZB_WAIT_HEAD:
        if (b == KT_ZIGBEE_FRAME_HEAD) {
            zb_state = ZB_ADDR_L;
            zb_addr = 0U;
            zb_len = 0U;
            zb_index = 0U;
        }
        break;
    case ZB_ADDR_L:
        zb_addr = b;
        zb_state = ZB_ADDR_H;
        break;
    case ZB_ADDR_H:
        zb_addr |= ((uint16_t)b << 8U);
        zb_state = ZB_LEN;
        break;
    case ZB_LEN:
        zb_len = b;
        zb_index = 0U;
        if (zb_len == 0U || zb_len > KT_ZIGBEE_MAX_PAYLOAD_LEN) {
            zigbee_len_error++;
            zb_state = ZB_WAIT_HEAD;
        } else {
            zb_state = ZB_DATA;
        }
        break;
    case ZB_DATA:
        zb_payload[zb_index++] = b;
        if (zb_index >= zb_len) {
            zb_state = ZB_TAIL;
        }
        break;
    case ZB_TAIL:
        if (b == KT_ZIGBEE_FRAME_TAIL) {
            zigbee_handle_payload(zb_addr, zb_payload, zb_len);
        } else {
            zigbee_tail_error++;
        }
        zb_state = ZB_WAIT_HEAD;
        break;
    default:
        zb_state = ZB_WAIT_HEAD;
        break;
    }
}

void kt_zigbee_uart_start_receive_it(void)
{
    kt_ringbuf_init(&zigbee_rx_ring, zigbee_rx_storage, ZIGBEE_RX_RING_SIZE);
    zb_state = ZB_WAIT_HEAD;
    zb_addr = 0U;
    zb_len = 0U;
    zb_index = 0U;
    zigbee_rx_count = 0U;
    zigbee_tx_count = 0U;
    zigbee_seat_rx_count = 0U;
    zigbee_rx_bytes = 0U;
    zigbee_len_error = 0U;
    zigbee_tail_error = 0U;
    zigbee_overflow = 0U;
    zigbee_addr_mismatch = 0U;
    zigbee_tx_fail_count = 0U;
    last_payload_len = 0U;
    last_addr = 0U;
    last_seat_valid = 0U;
    kt_zigbee_init(&huart1, 0);
    (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zigbee_rx_byte, 1);
}

void kt_zigbee_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (kt_ringbuf_put(&zigbee_rx_ring, zigbee_rx_byte) != 0) {
            zigbee_overflow++;
        }
        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zigbee_rx_byte, 1);
    }
}

void kt_zigbee_task(void)
{
    uint8_t b;
    uint8_t rx_budget = KT_ZIGBEE_RX_BYTES_PER_TASK;

    while (rx_budget > 0U && kt_ringbuf_get(&zigbee_rx_ring, &b) == 0) {
        zigbee_rx_bytes++;
        zigbee_parse_byte(b);
        rx_budget--;
    }
    kt_system_health_note_zigbee_task();
}

void kt_zigbee_print_link_info(void)
{
    KT_LOG_INFO("ZigBee main link:");
    KT_LOG_INFO("role=coordinator self=0x%04X peer seat=0x%04X",
                (unsigned int)ZIGBEE_ADDR_SELF,
                (unsigned int)ZIGBEE_ADDR_SEAT_NODE);
    KT_LOG_INFO("UART=USART1 PA9/TX PA10/RX 38400 8N1");
    KT_LOG_INFO("frame=FA ADDRL ADDRH LEN DATA F5");
    KT_LOG_INFO("TX count=%lu TX fail=%lu RX frame ok=%lu",
                (unsigned long)zigbee_tx_count,
                (unsigned long)zigbee_tx_fail_count,
                (unsigned long)zigbee_rx_count);
    KT_LOG_INFO("RX bytes=%lu len_err=%lu tail_err=%lu overflow=%lu addr_mismatch=%lu",
                (unsigned long)zigbee_rx_bytes,
                (unsigned long)zigbee_len_error,
                (unsigned long)zigbee_tail_error,
                (unsigned long)zigbee_overflow,
                (unsigned long)zigbee_addr_mismatch);
    if (last_payload_len == 0U) {
        KT_LOG_WARN("last payload empty");
    } else {
        KT_LOG_INFO("last addr=0x%04X payload len=%u hex=%02X %02X %02X %02X",
                    (unsigned int)last_addr,
                    (unsigned int)last_payload_len,
                    (unsigned int)last_payload[0],
                    (unsigned int)((last_payload_len > 1U) ? last_payload[1] : 0U),
                    (unsigned int)((last_payload_len > 2U) ? last_payload[2] : 0U),
                    (unsigned int)((last_payload_len > 3U) ? last_payload[3] : 0U));
    }
    KT_LOG_INFO("SEAT RX count=%lu", (unsigned long)zigbee_seat_rx_count);
    if (last_seat_valid == 0U) {
        KT_LOG_WARN("last SEAT payload empty");
    } else {
        KT_LOG_INFO("last SEAT: S1=%u S2=%u S3=%u",
                    (unsigned int)last_seat_payload[1],
                    (unsigned int)last_seat_payload[2],
                    (unsigned int)last_seat_payload[3]);
    }
    KT_LOG_INFO("test: FF 86 00 FF sends binary PING to 0x%04X",
                (unsigned int)ZIGBEE_ADDR_SEAT_NODE);
}

void kt_zigbee_send_test(void)
{
    kt_zigbee_send_ping();
}

void kt_zigbee_send_ping(void)
{
    uint8_t p[2];

    p[0] = ZIGBEE_PAYLOAD_PING;
    p[1] = zigbee_ping_seq++;
    zigbee_send_payload(ZIGBEE_ADDR_SEAT_NODE, p, sizeof(p));
    KT_LOG_INFO("[ZIGBEE] TX PING seq=%u dst=0x%04X",
                (unsigned int)p[1], (unsigned int)ZIGBEE_ADDR_SEAT_NODE);
}

void kt_zigbee_print_recent_rx(void)
{
    if (last_payload_len == 0U) {
        KT_LOG_WARN("ZigBee RX buffer empty");
    } else {
        KT_LOG_INFO("ZigBee recent payload: addr=0x%04X len=%u hex=%02X %02X %02X %02X",
                    (unsigned int)last_addr,
                    (unsigned int)last_payload_len,
                    (unsigned int)last_payload[0],
                    (unsigned int)((last_payload_len > 1U) ? last_payload[1] : 0U),
                    (unsigned int)((last_payload_len > 2U) ? last_payload[2] : 0U),
                    (unsigned int)((last_payload_len > 3U) ? last_payload[3] : 0U));
    }
}

void kt_zigbee_clear_stats(void)
{
    zigbee_rx_count = 0U;
    zigbee_tx_count = 0U;
    zigbee_seat_rx_count = 0U;
    zigbee_rx_bytes = 0U;
    zigbee_len_error = 0U;
    zigbee_tail_error = 0U;
    zigbee_overflow = 0U;
    zigbee_addr_mismatch = 0U;
    zigbee_tx_fail_count = 0U;
    last_payload_len = 0U;
    last_seat_valid = 0U;
    KT_LOG_INFO("ZigBee stats cleared");
}

void kt_esp32s3_send_test(void)
{
    KT_LOG_WARN("USART3 ESP32 uses binary protocol only; use FF D0 00 FF for PING");
}
