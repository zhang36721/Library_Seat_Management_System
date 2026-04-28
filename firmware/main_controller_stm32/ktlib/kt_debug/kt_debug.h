#ifndef KT_DEBUG_H
#define KT_DEBUG_H

#include "usart.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the debug subsystem
 *         - Initializes the protocol state machine
 *         - Registers the UART RX callback with kt_port_uart
 */
void kt_debug_init(void);

/**
 * @brief Debug main task (call periodically from main loop)
 *
 *        Currently a placeholder. In future iterations, protocol
 *        parsing will be deferred here from interrupt context.
 */
void kt_debug_task(void);

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

#ifdef __cplusplus
}
#endif

#endif /* KT_DEBUG_H */