#include "kt_debug_protocol.h"
#include "kt_config.h"
#include "kt_debug.h"
#include "kt_port_uart.h"
#include <stdio.h>

/* Protocol state machine context */
static kt_protocol_state_t proto_state = KT_PROTO_STATE_IDLE;
static uint8_t proto_cmd   = 0;
static uint8_t proto_value = 0;

/**
 * @brief Initialize the debug protocol state machine
 */
void kt_debug_protocol_init(void)
{
    proto_state = KT_PROTO_STATE_IDLE;
    proto_cmd   = 0;
    proto_value = 0;
}

/**
 * @brief Print the raw frame bytes received
 */
static void print_raw_frame(uint8_t cmd, uint8_t value)
{
    char buf[64];
    snprintf(buf, sizeof(buf),
             "[DEBUG] RX FRAME: FF %02X %02X FF\r\n",
             cmd, value);
    kt_port_uart_tx_string(buf);
}

/**
 * @brief Check if a byte is valid (not the frame marker)
 */
static int is_valid_byte(uint8_t byte)
{
    /* The only byte that cannot appear as CMD or VALUE is 0xFF
       because it's reserved as the frame marker */
    return (byte != KT_PROTOCOL_FRAME_MARKER);
}

/**
 * @brief Feed a single byte into the protocol parser
 */
void kt_debug_protocol_input_byte(uint8_t byte)
{
    switch (proto_state) {

    case KT_PROTO_STATE_IDLE:
        /* Waiting for first 0xFF */
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            proto_state = KT_PROTO_STATE_GOT_FF1;
        }
        /* Any other byte is silently ignored */
        break;

    case KT_PROTO_STATE_GOT_FF1:
        /* Got first 0xFF, waiting for CMD */
        if (is_valid_byte(byte)) {
            proto_cmd   = byte;
            proto_state = KT_PROTO_STATE_GOT_CMD;
        } else {
            /* byte == 0xFF: consecutive FF, stay in GOT_FF1 and treat this
               as a new first FF (resync) */
            /* proto_state stays GOT_FF1 */
        }
        break;

    case KT_PROTO_STATE_GOT_CMD:
        /* Got CMD, waiting for VALUE */
        if (is_valid_byte(byte)) {
            proto_value = byte;
            proto_state = KT_PROTO_STATE_GOT_VALUE;
        } else {
            /* byte == 0xFF: invalid VALUE, resync to GOT_FF1 with this as new FF1 */
            proto_state = KT_PROTO_STATE_GOT_FF1;
        }
        break;

    case KT_PROTO_STATE_GOT_VALUE:
        /* Got VALUE, waiting for second 0xFF */
        if (byte == KT_PROTOCOL_FRAME_MARKER) {
            /* Valid frame received */
            print_raw_frame(proto_cmd, proto_value);
            kt_debug_execute_command(proto_cmd, proto_value);
            proto_state = KT_PROTO_STATE_IDLE;
        } else {
            /* Invalid frame: missing trailing FF */
            kt_port_uart_tx_string("[DEBUG] Invalid frame\r\n");
            proto_state = KT_PROTO_STATE_IDLE;
        }
        break;

    default:
        /* Safety reset */
        proto_state = KT_PROTO_STATE_IDLE;
        break;
    }
}