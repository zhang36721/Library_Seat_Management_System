#include "kt_zigbee/kt_zigbee.h"
#include "kt_config.h"
#include <string.h>

#define KT_ZIGBEE_TX_QUEUE_LEN 8U
#define KT_ZIGBEE_TX_FRAME_MAX (KT_ZIGBEE_MAX_PAYLOAD_LEN + 5U)

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

typedef struct {
    uint8_t len;
    uint8_t bytes[KT_ZIGBEE_TX_FRAME_MAX];
} zigbee_tx_item_t;

static zigbee_tx_item_t tx_queue[KT_ZIGBEE_TX_QUEUE_LEN];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;
static volatile uint8_t tx_count;
static volatile uint8_t tx_active;
static uint8_t tx_active_bytes[KT_ZIGBEE_TX_FRAME_MAX];
static uint8_t tx_active_len;

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
    tx_head = 0U;
    tx_tail = 0U;
    tx_count = 0U;
    tx_active = 0U;
    tx_active_len = 0U;
    parser_reset();
}

static void tx_kick(void)
{
    zigbee_tx_item_t *item;

    if (zigbee_uart == 0 || tx_active != 0U || tx_count == 0U) {
        return;
    }

    if (zigbee_uart->gState != HAL_UART_STATE_READY) {
        return;
    }

    item = &tx_queue[tx_tail];
    memcpy(tx_active_bytes, item->bytes, item->len);
    tx_active_len = item->len;
    tx_tail++;
    if (tx_tail >= KT_ZIGBEE_TX_QUEUE_LEN) {
        tx_tail = 0U;
    }
    tx_count--;
    tx_active = 1U;

    if (HAL_UART_Transmit_IT(zigbee_uart, tx_active_bytes, tx_active_len) != HAL_OK) {
        tx_active = 0U;
        tx_active_len = 0U;
    }
}

HAL_StatusTypeDef kt_zigbee_send_to(uint16_t dst_addr,
                                    const uint8_t *data,
                                    uint8_t len)
{
    uint8_t frame[KT_ZIGBEE_TX_FRAME_MAX];
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

    __disable_irq();
    if (tx_count >= KT_ZIGBEE_TX_QUEUE_LEN) {
        __enable_irq();
        return HAL_BUSY;
    }

    tx_queue[tx_head].len = index;
    memcpy(tx_queue[tx_head].bytes, frame, index);
    tx_head++;
    if (tx_head >= KT_ZIGBEE_TX_QUEUE_LEN) {
        tx_head = 0U;
    }
    tx_count++;
    __enable_irq();
    tx_kick();
    return HAL_OK;
}

void kt_zigbee_tx_callback(UART_HandleTypeDef *huart)
{
    if (zigbee_uart != 0 && huart->Instance == zigbee_uart->Instance) {
        tx_active = 0U;
        tx_active_len = 0U;
        tx_kick();
    }
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
