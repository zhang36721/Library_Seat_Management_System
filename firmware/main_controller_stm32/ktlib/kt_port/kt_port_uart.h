#ifndef KT_PORT_UART_H
#define KT_PORT_UART_H

#include "usart.h"
#include <stdint.h>
#include "kt_ringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Extern UART handle defined by CubeMX in usart.c */
extern UART_HandleTypeDef huart2;

/* Receive buffer for single-byte interrupt reception */
extern volatile uint8_t kt_uart_rx_byte;

/* Ring buffer for deferred RX processing (ISR writes, main loop reads) */
extern kt_ringbuf_t kt_uart_rx_ring;

/**
 * @brief Transmit a string over USART2 (blocking)
 * @param str  Null-terminated string to send
 * @return HAL_StatusTypeDef  HAL_OK on success, error code otherwise
 */
int kt_port_uart_tx_string(const char *str);

/**
 * @brief Transmit a single byte over USART2 (blocking)
 * @param byte  Byte to send
 */
void kt_port_uart_tx_byte(uint8_t byte);

/**
 * @brief Start UART2 interrupt reception for a single byte
 */
void kt_port_uart_start_receive_it(void);

/**
 * @brief Callback to be called from HAL_UART_RxCpltCallback when huart == huart2
 *
 *        ISR responsibility: ONLY write the received byte into kt_uart_rx_ring.
 *        No callbacks, no protocol parsing, no blocking calls in ISR.
 *
 * @param huart  UART handle that triggered the callback
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart);

/**
 * @brief Get number of bytes available for reading from the RX ring buffer
 * @return Number of bytes currently buffered
 */
uint16_t kt_port_uart_rx_available(void);

/**
 * @brief Read one byte from the RX ring buffer (main loop context)
 * @param byte  Output: received byte
 * @return 0 on success, -1 if buffer is empty
 */
int kt_port_uart_rx_read(uint8_t *byte);

/**
 * @brief Get RX overflow count (incremented when ring buffer is full in ISR)
 * @return Number of bytes lost due to ring buffer full
 */
uint32_t kt_port_uart_rx_overflow_count(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_PORT_UART_H */
