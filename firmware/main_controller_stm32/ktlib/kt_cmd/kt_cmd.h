#ifndef KT_CMD_H
#define KT_CMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the command dispatch subsystem
 */
void kt_cmd_init(void);

/**
 * @brief Dispatch a command received from the protocol layer
 * @param cmd   Command byte
 * @param data  Data byte
 */
void kt_cmd_dispatch(uint8_t cmd, uint8_t data);

/**
 * @brief Print current debug / command status
 */
void kt_cmd_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_CMD_H */