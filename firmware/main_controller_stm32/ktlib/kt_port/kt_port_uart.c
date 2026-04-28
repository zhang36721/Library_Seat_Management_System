#include "kt_port_uart.h"
#include "kt_debug.h"
#include <string.h>

/* Single-byte receive buffer for UART2 interrupt reception */
volatile uint8_t kt_uart_rx_byte = 0;

/**
 * @brief Transmit a single byte over USART2 (blocking)
 */
void kt_port_uart_tx_byte(uint8_t byte)
{
    HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
}

/**
 * @brief Transmit a string over USART2 (blocking)
 */
void kt_port_uart_tx_string(const char *str)
{
    if (str == NULL) {
        return;
    }
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
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
 *        This forwards the received byte to the debug layer for protocol parsing,
 *        then re-arms the interrupt for the next byte.
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* Forward received byte to debug protocol layer */
        kt_debug_uart_rx_callback(huart);

        /* Re-arm interrupt for next byte */
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&kt_uart_rx_byte, 1);
    }
}