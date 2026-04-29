#include "kt_debug.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"
#include "kt_protocol.h"
#include "kt_cmd.h"
#include "kt_system/kt_boot_count.h"
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

    /* Keep boot output concise; kt_debug_print_system_info() prints the banner. */
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
    kt_port_uart_tx_string("=== System Boot ===\r\n");

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Project : %s\r\n", KT_PROJECT_NAME);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Firmware: %s\r\n", KT_FW_NAME);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Version : %s\r\n", KT_PROJECT_VERSION);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Build   : %s %s\r\n", __DATE__, __TIME__);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Author  : %s\r\n", KT_AUTHOR);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Student : %s\r\n", KT_STUDENT_ID);
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Boot    : %lu\r\n", (unsigned long)kt_boot_count_get());
    kt_port_uart_tx_string(kt_print_buf);

    snprintf(kt_print_buf, sizeof(kt_print_buf),
             "Debug   : USART2 %d, FF CMD DATA FF\r\n", KT_DEBUG_UART_BAUDRATE);
    kt_port_uart_tx_string(kt_print_buf);

    kt_port_uart_tx_string("===================\r\n");
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
    KT_LOG_INFO("  FF 20 00 FF  -> Print hardware resource status");
    KT_LOG_INFO("  FF 21 00 FF  -> LED test");
    KT_LOG_INFO("  FF 22 00 FF  -> Buzzer short beep test");
    KT_LOG_INFO("  FF 23 00 FF  -> Read PA0 button state");
    KT_LOG_INFO("  FF 24 00 FF  -> Print USART roles");
    KT_LOG_INFO("  FF 25 00 FF  -> Print GPIO init status");
    KT_LOG_INFO("  FF 30 00 FF  -> Read seat sensor raw levels");
    KT_LOG_INFO("  FF 31 00 FF  -> Print seat FREE/OCCUPIED status");
    KT_LOG_INFO("  FF 40 00 FF  -> RC522 init");
    KT_LOG_INFO("  FF 41 00 FF  -> RC522 read UID");
    KT_LOG_INFO("  FF 50 00 FF  -> OLED test text");
    KT_LOG_INFO("  FF 60 00 FF  -> DS1302 read time");
    KT_LOG_INFO("  FF 61 00 FF  -> DS1302 write test time");
    KT_LOG_INFO("  FF 62 YY FF  -> DS1302 pending year 20YY");
    KT_LOG_INFO("  FF 63 MM FF  -> DS1302 pending month");
    KT_LOG_INFO("  FF 64 DD FF  -> DS1302 pending day");
    KT_LOG_INFO("  FF 65 hh FF  -> DS1302 pending hour");
    KT_LOG_INFO("  FF 66 mm FF  -> DS1302 pending minute");
    KT_LOG_INFO("  FF 67 ss FF  -> DS1302 pending second");
    KT_LOG_INFO("  FF 68 00 FF  -> DS1302 write pending time");
    KT_LOG_INFO("  FF 69 00 FF  -> DS1302 print pending time");
    KT_LOG_INFO("  FF 70 00 FF  -> Stepper forward");
    KT_LOG_INFO("  FF 71 00 FF  -> Stepper reverse");
    KT_LOG_INFO("  FF 72 00 FF  -> Stepper stop");
    KT_LOG_INFO("  FF 80 00 FF  -> USART1 ZigBee test TX");
    KT_LOG_INFO("  FF 81 00 FF  -> USART1 ZigBee recent RX");
    KT_LOG_INFO("  FF 82 00 FF  -> USART1 ZigBee PING TX");
    KT_LOG_INFO("  FF 90 00 FF  -> USART3 ESP32S3 test TX");
    KT_LOG_INFO("  FF A0 00 FF  -> Main app status");
    KT_LOG_INFO("  FF A1 00 FF  -> Local card flow test");
    KT_LOG_INFO("  FF A2 00 FF  -> OLED home");
    KT_LOG_INFO("  FF A3 00 FF  -> OLED last card result");
    KT_LOG_INFO("  FF A4 00 FF  -> Buzzer success prompt");
    KT_LOG_INFO("  FF A5 00 FF  -> Buzzer fail prompt");
    KT_LOG_INFO("  FF A6 00 FF  -> Toggle simulated seats");
    KT_LOG_INFO("  FF B0 00 FF  -> Main key mapping");
    KT_LOG_INFO("  FF B1 00 FF  -> Main key raw levels");
    KT_LOG_INFO("  FF B2 00 FF  -> Last main key event");
    KT_LOG_INFO("  FF B3 00 FF  -> Enter time set UI");
    KT_LOG_INFO("  FF B4 00 FF  -> Enter card add UI");
    KT_LOG_INFO("  FF B5 00 FF  -> Enter card delete UI");
    KT_LOG_INFO("====================");
}
