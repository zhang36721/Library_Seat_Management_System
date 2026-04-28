#include "kt_port_uart.h"
#include "kt_config.h"
#include <string.h>

/* Ring buffer backing storage: 256 bytes (255 usable) */
#define KT_UART_RX_RING_SIZE  256
static uint8_t kt_uart_rx_ring_buf[KT_UART_RX_RING_SIZE];

/* Single-byte receive buffer for UART2 interrupt reception */
volatile uint8_t kt_uart_rx_byte = 0;

/* Ring buffer instance (public for upper layers that need direct access) */
kt_ringbuf_t kt_uart_rx_ring;

/* Overflow counter - incremented in ISR when ring buffer is full */
static volatile uint32_t kt_uart_rx_overflow_cnt = 0;

/**
 * @brief Transmit a single byte over USART2 (finite timeout)
 *
 *        Uses KT_UART_TX_TIMEOUT_MS to avoid permanent blocking if
 *        UART TX is stuck (e.g., no cable connected, RTS/CTS issue).
 */
void kt_port_uart_tx_byte(uint8_t byte)
{
    HAL_UART_Transmit(&huart2, &byte, 1, (uint32_t)KT_UART_TX_TIMEOUT_MS);
}

/**
 * @brief Transmit a string over USART2 (finite timeout)
 * @return HAL_OK on success, error code otherwise
 */
int kt_port_uart_tx_string(const char *str)
{
    HAL_StatusTypeDef status;
    size_t len;

    if (str == NULL) {
        return HAL_ERROR;
    }

    len = strlen(str);
    if (len == 0) {
        return HAL_OK;
    }

    status = HAL_UART_Transmit(&huart2, (uint8_t *)str, len,
                               (uint32_t)KT_UART_TX_TIMEOUT_MS);
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

    kt_uart_rx_overflow_cnt = 0;

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
 *          - Call any logging (KT_LOG_* not ISR-safe)
 *
 *        If the ring buffer is full, the byte is silently dropped and
 *        the overflow counter is incremented.
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* ISR: only push to ring buffer; track overflow */
        if (kt_ringbuf_put(&kt_uart_rx_ring, kt_uart_rx_byte) != 0) {
            kt_uart_rx_overflow_cnt++;
            /* Byte dropped — no ISR-safe logging, just count */
        }

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

/**
 * @brief Get RX overflow count (incremented when ring buffer is full in ISR)
 * @return Number of bytes lost due to ring buffer full
 */
uint32_t kt_port_uart_rx_overflow_count(void)
{
    /* Safe read of volatile counter from main loop context */
    return kt_uart_rx_overflow_cnt;
}