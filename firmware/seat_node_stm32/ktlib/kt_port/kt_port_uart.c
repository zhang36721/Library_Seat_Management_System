#include "kt_port_uart.h"
#include "kt_config.h"
#include <string.h>

#define KT_UART_TX_RING_SIZE 1024U

/* Single-byte receive buffer for UART2 interrupt reception */
volatile uint8_t kt_uart_rx_byte = 0;

/* Registered callback for received UART bytes (set externally by upper layers) */
static kt_uart_rx_callback_t kt_uart_rx_cb = NULL;
static uint8_t tx_ring[KT_UART_TX_RING_SIZE];
static volatile uint16_t tx_head;
static volatile uint16_t tx_tail;
static volatile uint8_t tx_busy;
static volatile uint32_t tx_drop_count;
static uint8_t tx_active_byte;

static uint16_t tx_next(uint16_t value)
{
    value++;
    if (value >= KT_UART_TX_RING_SIZE) {
        value = 0U;
    }
    return value;
}

static void tx_kick(void)
{
    uint16_t tail;

    if (tx_busy != 0U || tx_head == tx_tail) {
        return;
    }

    tail = tx_tail;
    tx_active_byte = tx_ring[tail];
    tx_tail = tx_next(tail);
    tx_busy = 1U;
    if (HAL_UART_Transmit_IT(&huart2, &tx_active_byte, 1U) != HAL_OK) {
        tx_busy = 0U;
        tx_drop_count++;
    }
}

static uint8_t tx_enqueue(uint8_t byte)
{
    uint16_t next;

    __disable_irq();
    next = tx_next(tx_head);
    if (next == tx_tail) {
        tx_drop_count++;
        __enable_irq();
        return 0U;
    }
    tx_ring[tx_head] = byte;
    tx_head = next;
    __enable_irq();
    return 1U;
}

/**
 * @brief Register a callback for received UART bytes
 */
void kt_port_uart_set_rx_callback(kt_uart_rx_callback_t cb)
{
    kt_uart_rx_cb = cb;
}

/**
 * @brief Queue a single byte over USART2
 */
void kt_port_uart_tx_byte(uint8_t byte)
{
    (void)tx_enqueue(byte);
    tx_kick();
}

/**
 * @brief Transmit a string over USART2 (blocking)
 * @return HAL_OK on success, error code otherwise
 */
int kt_port_uart_tx_string(const char *str)
{
    uint8_t ok = 1U;
    size_t i;
    size_t len;

    if (str == NULL) {
        return HAL_ERROR;
    }
    len = strlen(str);
    for (i = 0U; i < len; i++) {
        if (tx_enqueue((uint8_t)str[i]) == 0U) {
            ok = 0U;
        }
    }
    tx_kick();
    return ok ? HAL_OK : HAL_BUSY;
}

/**
 * @brief Start UART2 interrupt reception for a single byte
 */
void kt_port_uart_start_receive_it(void)
{
    tx_head = 0U;
    tx_tail = 0U;
    tx_busy = 0U;
    tx_drop_count = 0U;
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&kt_uart_rx_byte, 1);
}

/**
 * @brief Callback called from HAL_UART_RxCpltCallback when huart == huart2
 *
 *        This function is kept minimal by design:
 *        1. Verify the UART instance is USART2
 *        2. Forward the received byte to the registered upper-layer callback
 *        3. Re-arm the interrupt for the next byte
 *
 *        No protocol parsing or business logic lives here.
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* Forward received byte to registered upper-layer callback */
        if (kt_uart_rx_cb != NULL) {
            kt_uart_rx_cb(kt_uart_rx_byte);
        }

        /* Re-arm interrupt for next byte */
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&kt_uart_rx_byte, 1);
    }
}

void kt_port_uart_tx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        tx_busy = 0U;
        tx_kick();
    }
}
