#include "kt_debug.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"
#include "kt_debug_protocol.h"
#include <stdio.h>

/* Buffer for formatted output - shared across all debug print functions */
static char kt_print_buf[128];

/**
 * @brief Initialize the debug subsystem
 */
void kt_debug_init(void)
{
    kt_debug_protocol_init();

    /* Register UART RX callback: protocol parser receives bytes directly */
    kt_port_uart_set_rx_callback(kt_debug_protocol_input_byte);

    KT_LOG_INFO("System boot");
}

/**
 * @brief Debug main task (placeholder for future deferred processing)
 *
 *        Currently empty. In a future iteration, protocol parsing will
 *        be moved here from interrupt context to keep ISRs lean.
 */
void kt_debug_task(void)
{
    /* TODO: Deferred protocol processing will go here */
}

/**
 * @brief Print system information banner via USART2
 *
 * Uses snprintf into a static buffer, then transmits the buffer
 * via kt_port_uart_tx_string.  This avoids the C runtime printf
 * which depends on semihosting (__sys_write / BKPT) and will
 * hang the MCU when no debugger is attached.
 */
void kt_debug_print_system_info(void)
{
    kt_port_uart_tx_string("========================================\r\n");

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Project Name : %s\r\n", KT_PROJECT_NAME);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Firmware Role: %s\r\n", KT_FIRMWARE_ROLE);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Author       : %s\r\n", KT_AUTHOR);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Student ID   : %s\r\n", KT_STUDENT_ID);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Build Date   : %s\r\n", __DATE__);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Build Time   : %s\r\n", __TIME__);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Boot Count   : %d\r\n", KT_BOOT_COUNT);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Debug UART   : USART2 %d\r\n", KT_DEBUG_UART_BAUDRATE);
    kt_port_uart_tx_string(kt_print_buf);

    kt_port_uart_tx_string("Protocol     : FF CMD VALUE FF\r\n");
    kt_port_uart_tx_string("========================================\r\n");
}

/**
 * @brief Print help / command list via USART2
 */
void kt_debug_print_help(void)
{
    KT_LOG_INFO("Command List:");
    KT_LOG_INFO("  FF 00 00 FF  -> Turn OFF PC13 LED");
    KT_LOG_INFO("  FF 00 01 FF  -> Turn ON  PC13 LED");
    KT_LOG_INFO("  FF 01 00 FF  -> Print system info");
    KT_LOG_INFO("  FF 02 00 FF  -> Print this help");
}

/**
 * @brief Execute a debug command received from the protocol
 */
void kt_debug_execute_command(uint8_t cmd, uint8_t value)
{
    switch (cmd) {

    case 0x00: /* LED control */
        if (value == 0x00) {
            kt_port_led_off();
            KT_LOG_INFO("CMD_LED_OFF: PC13 LED OFF");
        } else if (value == 0x01) {
            kt_port_led_on();
            KT_LOG_INFO("CMD_LED_ON: PC13 LED ON");
        } else {
            KT_LOG_WARN("Unknown LED value: 0x%02X", value);
        }
        break;

    case 0x01: /* Print system info */
        if (value == 0x00) {
            kt_debug_print_system_info();
        } else {
            KT_LOG_WARN("Unknown value for CMD 0x01: 0x%02X", value);
        }
        break;

    case 0x02: /* Print help */
        if (value == 0x00) {
            kt_debug_print_help();
        } else {
            KT_LOG_WARN("Unknown value for CMD 0x02: 0x%02X", value);
        }
        break;

    default:
        KT_LOG_WARN("Unknown command: cmd=0x%02X value=0x%02X", cmd, value);
        break;
    }
}