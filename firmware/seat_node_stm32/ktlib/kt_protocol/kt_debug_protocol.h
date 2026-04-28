#ifndef KT_DEBUG_PROTOCOL_H
#define KT_DEBUG_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Protocol state machine states
 */
typedef enum {
    KT_PROTO_STATE_IDLE = 0,       /**< Waiting for first 0xFF */
    KT_PROTO_STATE_GOT_FF1,        /**< Got first 0xFF, waiting for CMD */
    KT_PROTO_STATE_GOT_CMD,        /**< Got CMD, waiting for VALUE */
    KT_PROTO_STATE_GOT_VALUE,      /**< Got VALUE, waiting for second 0xFF */
} kt_protocol_state_t;

/**
 * @brief Initialize the debug protocol state machine
 */
void kt_debug_protocol_init(void);

/**
 * @brief Feed a single byte into the protocol parser
 * @param byte  Received byte from UART
 */
void kt_debug_protocol_input_byte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* KT_DEBUG_PROTOCOL_H */