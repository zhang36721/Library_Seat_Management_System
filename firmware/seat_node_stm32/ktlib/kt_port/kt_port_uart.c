#include "kt_port_uart.h"
#include <string.h>

/* Single-byte receive buffer for UART2 interrupt reception */
volatile uint8_t kt_uart_rx_byte = 0;

/* Registered callback for received UART bytes (set externally by upper layers) */
static kt_uart_rx_callback_t kt_uart_rx_cb = NULL;

/**
 * @brief Register a callback for received UART bytes
 */
void kt_port_uart_set_rx_callback(kt_uart_rx_callback_t cb)
{
    kt_uart_rx_cb = cb;
}

/**
 * @brief Transmit a single byte over USART2 (blocking)
 */
void kt_port_uart_tx_byte(uint8_t byte)
{
    HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
}

/**
 * @brief Transmit a string over USART2 (blocking)
 * @return HAL_OK on success, error code otherwise
 */
int kt_port_uart_tx_string(const char *str)
{
    HAL_StatusTypeDef status;

    if (str == NULL) {
        return HAL_ERROR;
    }
    status = HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
    return (int)status;
}

/**
 * @brief Start UART2 interrupt reception for a single byte
 */
void kt_port_uart_start_receive_it(void)
{
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