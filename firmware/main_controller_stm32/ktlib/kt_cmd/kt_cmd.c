#include "kt_cmd.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_gpio.h"
#include "kt_debug.h"

/* Track LED state for status reporting */
static uint8_t kt_cmd_led_state = 0;

/**
 * @brief Initialize command dispatch
 */
void kt_cmd_init(void)
{
    kt_cmd_led_state = 0;
    KT_LOG_INFO("Command dispatch initialized");
}

/**
 * @brief Dispatch a command received from the protocol layer
 *
 * Supported commands:
 *   0x01  any     Print system information
 *   0x02  any     Turn on test LED (PC13)
 *   0x03  any     Turn off test LED (PC13)
 *   0x04  any     Print current debug status
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
 * @brief Print current debug / command status
 */
void kt_cmd_print_status(void)
{
    KT_LOG_INFO("=== Debug Status ===");
    KT_LOG_INFO("LED State : %s", kt_cmd_led_state ? "ON" : "OFF");
    KT_LOG_INFO("Protocol : FF CMD DATA FF");
    KT_LOG_INFO("Build    : %s %s", __DATE__, __TIME__);
    KT_LOG_INFO("====================");
}