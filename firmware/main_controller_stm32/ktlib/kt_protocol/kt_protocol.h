#ifndef KT_PROTOCOL_H
#define KT_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Frame handler callback - called when a valid FF CMD DATA FF frame
 *        is parsed.
 * @param cmd   Command byte
 * @param data  Data byte
 */
typedef void (*kt_protocol_frame_handler_t)(uint8_t cmd, uint8_t data);

/**
 * @brief Protocol state machine states
 */
typedef enum {
    KT_PROTO_STATE_IDLE = 0,
    KT_PROTO_STATE_GOT_FF1,
    KT_PROTO_STATE_GOT_CMD,
    KT_PROTO_STATE_GOT_DATA,
} kt_proto_state_t;

/**
 * @brief Initialize the protocol parser state machine
 */
void kt_protocol_init(void);

/**
 * @brief Register a handler for parsed frames
 * @param handler  Callback function (NULL to unregister)
 */
void kt_protocol_set_handler(kt_protocol_frame_handler_t handler);

/**
 * @brief Feed one byte into the protocol parser
 *
 *        This function IS safe to call from main loop context.
 *        It is intentionally NOT safe for ISR context because it may
 *        invoke user callbacks that do blocking UART TX.
 *
 * @param byte  Received byte
 */
void kt_protocol_input_byte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* KT_PROTOCOL_H */