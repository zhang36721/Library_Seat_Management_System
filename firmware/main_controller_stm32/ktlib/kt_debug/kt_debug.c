#include "kt_debug.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"
#include "kt_protocol.h"
#include "kt_cmd.h"
#include <stdio.h>

/* Buffer for formatted output - shared across all debug print functions */
static char kt_print_buf[128];

/**
 * @brief Initialize the debug subsystem (v0.2)
 *
 *        - Initializes protocol parser state machine
 *        - Registers protocol -> command dispatch handler
 *        - Initializes command dispatch subsystem
 */
void kt_debug_init(void) 
{
    /* Initialize protocol parser */
    kt_protocol_init();

    /* Route parsed frames to the command dispatcher */
    kt_protocol_set_handler(kt_cmd_dispatch);

    /* Initialize command dispatch */
    kt_cmd_init();

    KT_LOG_INFO("System boot (v0.2 ringbuffer + protocol + cmd dispatch)");
}

/**
 * @brief Debug main task (call from main loop)
 *
 *        v0.2.1: Adds protocol RX timeout checking (call BEFORE draining
 *        ring buffer). Each byte is fed to the protocol parser with the
 *        current system tick for timeout tracking.
 *
 *        This should be called as frequently as possible in the main
 *        loop so that incoming bytes are processed with minimal latency.
 */
void kt_debug_task(void)
{
    uint8_t byte;
    uint32_t now_ms;

    /* Get current system tick once per task invocation */
    now_ms = HAL_GetTick();

    /* Check and handle partial-frame timeout BEFORE processing new bytes */
    kt_protocol_check_timeout(now_ms);

    /* Drain all available bytes */
    while (kt_port_uart_rx_read(&byte) == 0) {
        kt_protocol_input_byte(byte, now_ms);
    }
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

    kt_port_uart_tx_string("Protocol     : FF CMD DATA FF\r\n");
    kt_port_uart_tx_string("========================================\r\n");
}

/**
 * @brief Print help / command list via USART2
 */
void kt_debug_print_help(void)
{
    KT_LOG_INFO("=== Command List ===");
    KT_LOG_INFO("  FF 01 xx FF  -> Print system info");
    KT_LOG_INFO("  FF 02 xx FF  -> Turn ON  test LED (PC13)");
    KT_LOG_INFO("  FF 03 xx FF  -> Turn OFF test LED (PC13)");
    KT_LOG_INFO("  FF 04 xx FF  -> Print debug status");
    KT_LOG_INFO("====================");
}