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
 * @param byte    Received byte
 * @param now_ms  Current system tick (milliseconds), used for timeout tracking
 */
void kt_protocol_input_byte(uint8_t byte, uint32_t now_ms);

/**
 * @brief Check for partial-frame timeout
 *
 *        If the state machine has been waiting in a non-IDLE state
 *        for longer than KT_PROTOCOL_RX_TIMEOUT_MS, the partial frame
 *        is discarded and the state machine resets to IDLE.
 *
 *        Call this at the beginning of each main-loop iteration,
 *        BEFORE draining the ring buffer.
 *
 * @param now_ms  Current system tick (milliseconds)
 */
void kt_protocol_check_timeout(uint32_t now_ms);

/**
 * @brief Get timeout-drop count (partial frames discarded due to timeout)
 * @return Number of timeout drops since last reset
 */
uint32_t kt_protocol_get_timeout_drop_count(void);

/**
 * @brief Get error-drop count (invalid frames discarded)
 * @return Number of error drops since last reset
 */
uint32_t kt_protocol_get_error_drop_count(void);

/**
 * @brief Reset protocol state machine to IDLE and clear statistics
 */
void kt_protocol_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_PROTOCOL_H */
