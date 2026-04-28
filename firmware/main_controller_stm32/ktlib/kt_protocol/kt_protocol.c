#include "kt_protocol.h"
#include "kt_config.h"
#include "kt_log.h"
#include <stddef.h>

/* Protocol state machine instance (v0.2.1) */
static struct {
    kt_proto_state_t state;
    uint8_t          cmd;
    uint8_t          data;
    kt_protocol_frame_handler_t handler;

    /* Timeout tracking: records the system tick when state leaves IDLE */
    uint32_t         state_entry_ms;

    /* Statistics */
    uint32_t         timeout_drop_count;
    uint32_t         error_drop_count;
} kt_proto;

/**
 * @brief Initialize the protocol parser
 */
void kt_protocol_init(void)
{
    kt_proto.state         = KT_PROTO_STATE_IDLE;
    kt_proto.cmd           = 0;
    kt_proto.data          = 0;
    kt_proto.handler       = NULL;
    kt_proto.state_entry_ms= 0;
    kt_proto.timeout_drop_count = 0;
    kt_proto.error_drop_count   = 0;
}

/**
 * @brief Register a handler for parsed frames
 */
void kt_protocol_set_handler(kt_protocol_frame_handler_t handler)
{
    kt_proto.handler = handler;
}

/**
 * @brief Reset protocol state machine to IDLE and clear statistics
 */
void kt_protocol_reset(void)
{
    kt_proto.state         = KT_PROTO_STATE_IDLE;
    kt_proto.cmd           = 0;
    kt_proto.data          = 0;
    kt_proto.state_entry_ms= 0;
    kt_proto.timeout_drop_count = 0;
    kt_proto.error_drop_count   = 0;
}

/**
 * @brief Get timeout-drop count
 */
uint32_t kt_protocol_get_timeout_drop_count(void)
{
    return kt_proto.timeout_drop_count;
}

/**
 * @brief Get error-drop count
 */
uint32_t kt_protocol_get_error_drop_count(void)
{
    return kt_proto.error_drop_count;
}

/**
 * @brief Check for partial-frame timeout
 *
 *        If the state machine is in any non-IDLE state and more than
 *        KT_PROTOCOL_RX_TIMEOUT_MS has elapsed since entering that state,
 *        the current partial frame is discarded and the state resets to IDLE.
 *
 * @param now_ms  Current system tick (milliseconds)
 */
void kt_protocol_check_timeout(uint32_t now_ms)
{
    if (kt_proto.state == KT_PROTO_STATE_IDLE) {
        /* No partial frame in progress; nothing to timeout */
        return;
    }

    /* Check if timeout period has elapsed */
    /* Handle tick wraparound correctly (uint32_t wraps at ~49 days) */
    if ((now_ms - kt_proto.state_entry_ms) >= (uint32_t)KT_PROTOCOL_RX_TIMEOUT_MS) {
        /* Discard partial frame */
        KT_LOG_WARN("Protocol timeout: discarding partial frame "
                    "(state=%d, cmd=0x%02X, data=0x%02X, elapsed=%lu ms)",
                    (int)kt_proto.state, kt_proto.cmd, kt_proto.data,
                    (unsigned long)(now_ms - kt_proto.state_entry_ms));

        kt_proto.state = KT_PROTO_STATE_IDLE;
        kt_proto.cmd   = 0;
        kt_proto.data  = 0;
        kt_proto.timeout_drop_count++;
    }
}

/**
 * @brief Feed one byte into the protocol state machine (v0.2.1)
 *
 * Frame format: FF CMD DATA FF
 *
 * State machine:
 *   IDLE  --(0xFF)-->  GOT_FF1 (record state_entry_ms)
 *   IDLE  --(other)->  IDLE   (discard byte)
 *
 *   GOT_FF1 --(0xFF)-->     GOT_FF1 (re-sync: treat as new FF1, refresh time)
 *   GOT_FF1 --(other)-->    GOT_CMD  (store cmd)
 *
 *   GOT_CMD --(0xFF)-->     GOT_FF1 (re-sync: cmd was FF)
 *   GOT_CMD --(other)-->    GOT_DATA (store data)
 *
 *   GOT_DATA --(any)-->     Frame complete.
 *       If byte == 0xFF -> valid frame, dispatch.
 *       If byte != 0xFF -> invalid frame, log warning, increment error_drop.
 *       Then reset to IDLE.
 *
 * @param byte    Received byte
 * @param now_ms  Current system tick (milliseconds), used for timeout tracking
 */
void kt_protocol_input_byte(uint8_t byte, uint32_t now_ms)
{
    switch (kt_proto.state) {

    case KT_PROTO_STATE_IDLE:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            kt_proto.state          = KT_PROTO_STATE_GOT_FF1;
            kt_proto.state_entry_ms = now_ms;
        }
        /* Non-0xFF bytes in IDLE are silently discarded */
        break;

    case KT_PROTO_STATE_GOT_FF1:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* Re-sync: consecutive FF, refresh timeout anchor */
            kt_proto.state_entry_ms = now_ms;
            /* kt_proto.state stays GOT_FF1 */
        } else {
            kt_proto.cmd   = byte;
            kt_proto.state = KT_PROTO_STATE_GOT_CMD;
            /* state_entry_ms unchanged — timeout tracked from first FF */
        }
        break;

    case KT_PROTO_STATE_GOT_CMD:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* CMD cannot be FF - re-sync */
            kt_proto.state          = KT_PROTO_STATE_GOT_FF1;
            kt_proto.state_entry_ms = now_ms;
        } else {
            kt_proto.data  = byte;
            kt_proto.state = KT_PROTO_STATE_GOT_DATA;
        }
        break;

    case KT_PROTO_STATE_GOT_DATA:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* Valid frame complete */
            if (kt_proto.handler != NULL) {
                kt_proto.handler(kt_proto.cmd, kt_proto.data);
            }
            KT_LOG_INFO("RX FRAME: FF %02X %02X FF",
                        kt_proto.cmd, kt_proto.data);
        } else {
            /* Invalid frame - trailing byte is not 0xFF */
            KT_LOG_WARN("Invalid frame: expected FF, got 0x%02X", byte);
            kt_proto.error_drop_count++;
        }
        kt_proto.state = KT_PROTO_STATE_IDLE;
        break;

    default:
        /* Should never reach here; reset to safe state */
        kt_proto.state = KT_PROTO_STATE_IDLE;
        break;
    }
}