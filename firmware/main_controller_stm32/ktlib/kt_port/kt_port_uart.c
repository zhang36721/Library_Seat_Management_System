#include "kt_port_uart.h"
#include <string.h>

/* Ring buffer backing storage: 256 bytes (255 usable) */
#define KT_UART_RX_RING_SIZE  256
static uint8_t kt_uart_rx_ring_buf[KT_UART_RX_RING_SIZE];

/* Single-byte receive buffer for UART2 interrupt reception */
volatile uint8_t kt_uart_rx_byte = 0;

/* Ring buffer instance (public for upper layers that need direct access) */
kt_ringbuf_t kt_uart_rx_ring;

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
 *
 *        Must be called once after UART init.  The ISR will re-arm itself
 *        after every received byte.
 */
void kt_port_uart_start_receive_it(void)
{
    /* Initialize the ring buffer before starting reception */
    kt_ringbuf_init(&kt_uart_rx_ring, kt_uart_rx_ring_buf, KT_UART_RX_RING_SIZE);

    HAL_UART_Receive_IT(&huart2, (uint8_t *)&kt_uart_rx_byte, 1);
}

/**
 * @brief ISR callback: ONLY writes byte to ring buffer and re-arms
 *
 *        This function must be called from HAL_UART_RxCpltCallback
 *        when huart->Instance == USART2.
 *
 *        It does NOT:
 *          - Parse any protocol
 *          - Call any business-logic callbacks
 *          - Perform blocking operations
 *
 *        If the ring buffer is full, the byte is silently dropped.
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* ISR: only push to ring buffer */
        (void)kt_ringbuf_put(&kt_uart_rx_ring, kt_uart_rx_byte);

        /* Re-arm interrupt for next byte */
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&kt_uart_rx_byte, 1);
    }
}

/**
 * @brief Get number of bytes available for reading from the RX ring buffer
 */
uint16_t kt_port_uart_rx_available(void)
{
    return kt_ringbuf_available(&kt_uart_rx_ring);
}

/**
 * @brief Read one byte from the RX ring buffer (main loop context)
 * @return 0 on success, -1 if buffer is empty
 */
int kt_port_uart_rx_read(uint8_t *byte)
{
    return kt_ringbuf_get(&kt_uart_rx_ring, byte);
}