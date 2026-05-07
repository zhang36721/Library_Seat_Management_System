#include "kt_zigbee/kt_zigbee.h"
#include "kt_config.h"
#include <string.h>

typedef enum
{
    ZB_PARSE_WAIT_HEAD = 0,
    ZB_PARSE_ADDR_L,
    ZB_PARSE_ADDR_H,
    ZB_PARSE_LEN,
    ZB_PARSE_DATA,
    ZB_PARSE_TAIL
} zigbee_parse_state_t;

static UART_HandleTypeDef *zigbee_uart;
static kt_zigbee_rx_callback_t zigbee_callback;
static zigbee_parse_state_t parse_state = ZB_PARSE_WAIT_HEAD;
static uint16_t parse_addr;
static uint8_t parse_len;
static uint8_t parse_index;
static uint8_t parse_payload[KT_ZIGBEE_MAX_PAYLOAD_LEN];

static void parser_reset(void)
{
    parse_state = ZB_PARSE_WAIT_HEAD;
    parse_addr = 0U;
    parse_len = 0U;
    parse_index = 0U;
}

void kt_zigbee_init(UART_HandleTypeDef *uart, kt_zigbee_rx_callback_t callback)
{
    zigbee_uart = uart;
    zigbee_callback = callback;
    parser_reset();
}

HAL_StatusTypeDef kt_zigbee_send_to(uint16_t dst_addr,
                                    const uint8_t *data,
                                    uint8_t len)
{
    uint8_t frame[KT_ZIGBEE_MAX_PAYLOAD_LEN + 5U];
    uint8_t index = 0U;

    if (zigbee_uart == 0 || data == 0 || len == 0U || len > KT_ZIGBEE_MAX_PAYLOAD_LEN) {
        return HAL_ERROR;
    }

    frame[index++] = KT_ZIGBEE_FRAME_HEAD;
    frame[index++] = (uint8_t)(dst_addr & 0xFFU);
    frame[index++] = (uint8_t)((dst_addr >> 8) & 0xFFU);
    frame[index++] = len;
    memcpy(&frame[index], data, len);
    index = (uint8_t)(index + len);
    frame[index++] = KT_ZIGBEE_FRAME_TAIL;

    return HAL_UART_Transmit(zigbee_uart, frame, index, KT_UART_TX_TIMEOUT_MS);
}

void kt_zigbee_rx_byte(uint8_t byte)
{
    switch (parse_state) {
    case ZB_PARSE_WAIT_HEAD:
        if (byte == KT_ZIGBEE_FRAME_HEAD) {
            parser_reset();
            parse_state = ZB_PARSE_ADDR_L;
        }
        break;

    case ZB_PARSE_ADDR_L:
        parse_addr = byte;
        parse_state = ZB_PARSE_ADDR_H;
        break;

    case ZB_PARSE_ADDR_H:
        parse_addr |= ((uint16_t)byte << 8);
        parse_state = ZB_PARSE_LEN;
        break;

    case ZB_PARSE_LEN:
        parse_len = byte;
        parse_index = 0U;
        if (parse_len == 0U || parse_len > KT_ZIGBEE_MAX_PAYLOAD_LEN) {
            parser_reset();
        } else {
            parse_state = ZB_PARSE_DATA;
        }
        break;

    case ZB_PARSE_DATA:
        parse_payload[parse_index++] = byte;
        if (parse_index >= parse_len) {
            parse_state = ZB_PARSE_TAIL;
        }
        break;

    case ZB_PARSE_TAIL:
        if (byte == KT_ZIGBEE_FRAME_TAIL && zigbee_callback != 0) {
            zigbee_callback(parse_addr, parse_payload, parse_len);
        }
        parser_reset();
        break;

    default:
        parser_reset();
        break;
    }
}
