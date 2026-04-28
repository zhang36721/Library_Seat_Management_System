#ifndef KT_DEBUG_H
#define KT_DEBUG_H

#include "usart.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the debug subsystem (protocol state machine)
 */
void kt_debug_init(void);

/**
 * @brief Print system information banner via USART2
 */
void kt_debug_print_system_info(void);

/**
 * @brief Print help / command list via USART2
 */
void kt_debug_print_help(void);

/**
 * @brief Execute a debug command received from the protocol
 * @param cmd    Command byte
 * @param value  Value byte
 */
void kt_debug_execute_command(uint8_t cmd, uint8_t value);

/**
 * @brief UART RX callback - called from HAL_UART_RxCpltCallback
 *        when huart->Instance == USART2
 * @param huart  UART handle that triggered the callback
 */
void kt_debug_uart_rx_callback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* KT_DEBUG_H */