#include "kt_cmd.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_gpio.h"
#include "kt_port_uart.h"
#include "kt_protocol.h"
#include "kt_debug.h"

/* Track LED state for status reporting */
static uint8_t kt_cmd_led_state = 0;

/**
 * @brief Initialize command dispatch
 */
void kt_cmd_init(void)
{
    kt_cmd_led_state = 0;
    KT_LOG_INFO("Command dispatch initialized (v0.2.1)");
}

/**
 * @brief Dispatch a command received from the protocol layer
 *
 * Supported commands:
 *   0x01  any     Print system information
 *   0x02  any     Turn on test LED (PC13)
 *   0x03  any     Turn off test LED (PC13)
 *   0x04  any     Print current debug status (expanded v0.2.1)
 */
void kt_cmd_dispatch(uint8_t cmd, uint8_t data)
{
    (void)data;  /* data byte reserved for future use */

    switch (cmd) {

    case 0x01:
        KT_LOG_INFO("CMD: Print system info");
        kt_debug_print_system_info();
        break;

    case 0x02:
        KT_LOG_INFO("CMD: LED ON");
        kt_port_led_on();
        kt_cmd_led_state = 1;
        break;

    case 0x03:
        KT_LOG_INFO("CMD: LED OFF");
        kt_port_led_off();
        kt_cmd_led_state = 0;
        break;

    case 0x04:
        KT_LOG_INFO("CMD: Print status");
        kt_cmd_print_status();
        break;

    default:
        KT_LOG_WARN("Unknown command: cmd=0x%02X data=0x%02X", cmd, data);
        break;
    }
}

/**
 * @brief Print current debug / command status (v0.2.1 expanded)
 *
 *        Includes protocol statistics to help diagnose frame loss,
 *        UART overflows, partial-frame timeouts, and invalid frames.
 */
void kt_cmd_print_status(void)
{
    /* Buffer for single-line formatted output */
    char buf[64];

    KT_LOG_INFO("=== Debug Status (v0.2.1) ===");
    KT_LOG_INFO("LED State      : %s", kt_cmd_led_state ? "ON" : "OFF");

    KT_LOG_INFO("Protocol       : FF CMD DATA FF");

    KT_LOG_INFO("Build          : %s %s", __DATE__, __TIME__);

    /* UART RX statistics */
    snprintf(buf, sizeof(buf), "RX ring avail  : %u", kt_port_uart_rx_available());
    KT_LOG_INFO("%s", buf);

    snprintf(buf, sizeof(buf), "RX overflow    : %lu",
             (unsigned long)kt_port_uart_rx_overflow_count());
    KT_LOG_INFO("%s", buf);

    /* Protocol parser statistics */
    snprintf(buf, sizeof(buf), "Timeout drops  : %lu",
             (unsigned long)kt_protocol_get_timeout_drop_count());
    KT_LOG_INFO("%s", buf);

    snprintf(buf, sizeof(buf), "Error drops    : %lu",
             (unsigned long)kt_protocol_get_error_drop_count());
    KT_LOG_INFO("%s", buf);

    KT_LOG_INFO("===============================");
}