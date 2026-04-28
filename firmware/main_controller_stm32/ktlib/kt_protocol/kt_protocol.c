#include "kt_protocol.h"
#include "kt_config.h"
#include "kt_log.h"
#include <stddef.h>

/* Protocol state machine instance */
static struct {
    kt_proto_state_t state;
    uint8_t          cmd;
    uint8_t          data;
    kt_protocol_frame_handler_t handler;
} kt_proto;

/**
 * @brief Initialize the protocol parser
 */
void kt_protocol_init(void)
{
    kt_proto.state   = KT_PROTO_STATE_IDLE;
    kt_proto.cmd     = 0;
    kt_proto.data    = 0;
    kt_proto.handler = NULL;
}

/**
 * @brief Register a handler for parsed frames
 */
void kt_protocol_set_handler(kt_protocol_frame_handler_t handler)
{
    kt_proto.handler = handler;
}

/**
 * @brief Feed one byte into the protocol state machine
 *
 * Frame format: FF CMD DATA FF
 *
 * State machine:
 *   IDLE  --(0xFF)-->  GOT_FF1
 *   IDLE  --(other)->  IDLE  (discard byte)
 *
 *   GOT_FF1 --(0xFF)-->     GOT_FF1  (re-sync: treat as new FF1)
 *   GOT_FF1 --(other)-->    GOT_CMD  (store cmd)
 *
 *   GOT_CMD --(0xFF)-->     GOT_FF1  (re-sync: cmd was FF)
 *   GOT_CMD --(other)-->    GOT_DATA (store data)
 *
 *   GOT_DATA --(any)-->     Frame complete.
 *       If byte == 0xFF -> valid frame, dispatch.
 *       If byte != 0xFF -> invalid frame, log warning.
 *       Then reset to IDLE.
 *       If the received byte == 0xFF and it's the end marker,
 *       subsequent bytes start fresh at IDLE.
 */
void kt_protocol_input_byte(uint8_t byte)
{
    switch (kt_proto.state) {

    case KT_PROTO_STATE_IDLE:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            kt_proto.state = KT_PROTO_STATE_GOT_FF1;
        }
        /* Non-0xFF bytes in IDLE are silently discarded */
        break;

    case KT_PROTO_STATE_GOT_FF1:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* Re-sync: stay in GOT_FF1 */
            /* kt_proto.state = KT_PROTO_STATE_GOT_FF1; */
        } else {
            kt_proto.cmd   = byte;
            kt_proto.state = KT_PROTO_STATE_GOT_CMD;
        }
        break;

    case KT_PROTO_STATE_GOT_CMD:
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* CMD cannot be FF - re-sync */
            kt_proto.state = KT_PROTO_STATE_GOT_FF1;
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
        }
        kt_proto.state = KT_PROTO_STATE_IDLE;
        break;

    default:
        /* Should never reach here; reset to safe state */
        kt_proto.state = KT_PROTO_STATE_IDLE;
        break;
    }
}