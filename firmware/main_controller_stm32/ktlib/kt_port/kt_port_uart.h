#ifndef KT_PORT_UART_H
#define KT_PORT_UART_H

#include "usart.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Extern UART handle defined by CubeMX in usart.c */
extern UART_HandleTypeDef huart2;

/* Receive buffer for single-byte interrupt reception */
extern volatile uint8_t kt_uart_rx_byte;

/**
 * @brief Transmit a string over USART2 (blocking)
 * @param str  Null-terminated string to send
 */
void kt_port_uart_tx_string(const char *str);

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
 * @param huart  UART handle that triggered the callback
 */
void kt_port_uart_rx_callback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* KT_PORT_UART_H */