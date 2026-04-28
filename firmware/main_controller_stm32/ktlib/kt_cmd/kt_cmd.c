#include "kt_cmd.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_debug.h"
#include "kt_app/app_io.h"
#include "kt_modules/kt_hw_diag.h"
#include "kt_modules/kt_rc522.h"
#include "kt_modules/kt_oled.h"
#include "kt_modules/kt_ds1302.h"
#include "kt_modules/kt_stepper.h"
#include "kt_modules/kt_uart_links.h"

/**
 * @brief Initialize command dispatch
 */
void kt_cmd_init(void)
{
    /* Command dispatch has no runtime state in v0.6.1. */
}

/**
 * @brief Dispatch a command received from the protocol layer
 *
 * Supported commands:
 *   0x01  any     Print system information
 *   0x02  any     Turn on LED   (app_led)
 *   0x03  any     Turn off LED  (app_led)
 *   0x04  any     Print current debug status
 *   0x05  any     Toggle LED    (app_led)
 *   0x06  any     Start LED blink (500ms) (app_led)
 *   0x07  any     Stop LED blink  (app_led)
 *   0x08  any     Read button state (app_btn)
 *   0x09  any     Buzzer ON       (app_buzzer)
 *   0x0A  any     Buzzer OFF      (app_buzzer)
 *   0x0B  any     Buzzer beep 50ms (app_buzzer)
 *   0x0C  any     Buzzer beep 200ms (app_buzzer)
 *   0x0D  any     Buzzer cycle start (100ms on, 400ms off) (app_buzzer)
 *   0x0E  any     Buzzer cycle stop (app_buzzer)
 *   0x20  0x00    Print hardware resource status
 *   0x21  0x00    LED test
 *   0x22  0x00    Buzzer short beep test
 *   0x23  0x00    Read button state
 *   0x24  0x00    Print USART responsibilities
 *   0x25  0x00    Print GPIO initialization status
 *   0x30  0x00    Read 3 seat sensor raw levels
 *   0x31  0x00    Print 3 seat FREE/OCCUPIED status
 *   0x40  0x00    Initialize RC522
 *   0x41  0x00    Read RC522 UID
 *   0x50  0x00    OLED test text
 *   0x60  0x00    DS1302 read time
 *   0x61  0x00    DS1302 write test time
 *   0x70  0x00    Stepper forward test
 *   0x71  0x00    Stepper reverse test
 *   0x72  0x00    Stepper stop
 *   0x80  0x00    USART1 ZigBee test TX
 *   0x81  0x00    Print USART1 ZigBee recent RX
 *   0x82  0x00    USART1 ZigBee PING TX
 *   0x90  0x00    USART3 ESP32S3 test TX
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
        kt_led_on(&app_led);
        break;

    case 0x03:
        KT_LOG_INFO("CMD: LED OFF");
        kt_led_off(&app_led);
        break;

    case 0x04:
        KT_LOG_INFO("CMD: Print status");
        kt_cmd_print_status();
        break;

    case 0x05:
        KT_LOG_INFO("CMD: LED Toggle");
        kt_led_toggle(&app_led);
        break;

    case 0x06:
        KT_LOG_INFO("CMD: LED Blink Start (500ms)");
        kt_led_blink_start(&app_led, 500);
        break;

    case 0x07:
        KT_LOG_INFO("CMD: LED Blink Stop");
        kt_led_blink_stop(&app_led);
        break;

    case 0x08:
        KT_LOG_INFO("CMD: Read button state");
        KT_LOG_INFO("Button pressed: %u, click event: %u",
                    (unsigned int)kt_button_is_pressed(&app_btn),
                    (unsigned int)kt_button_get_click_event(&app_btn));
        break;

    /* Buzzer commands */
    case 0x09:
        KT_LOG_INFO("CMD: Buzzer ON");
        kt_buzzer_on(&app_buzzer);
        break;

    case 0x0A:
        KT_LOG_INFO("CMD: Buzzer OFF");
        kt_buzzer_off(&app_buzzer);
        break;

    case 0x0B:
        KT_LOG_INFO("CMD: Buzzer beep 50ms");
        kt_buzzer_beep(&app_buzzer, 50);
        break;

    case 0x0C:
        KT_LOG_INFO("CMD: Buzzer beep 200ms");
        kt_buzzer_beep(&app_buzzer, 200);
        break;

    case 0x0D:
        KT_LOG_INFO("CMD: Buzzer cycle start (100ms/400ms)");
        kt_buzzer_cycle_start(&app_buzzer, 100, 400);
        break;

    case 0x0E:
        KT_LOG_INFO("CMD: Buzzer cycle stop");
        kt_buzzer_cycle_stop(&app_buzzer);
        break;

    case 0x20:
        KT_LOG_INFO("CMD 0x20: Hardware resource status");
        kt_hw_diag_print_hardware_resources();
        break;

    case 0x21:
        KT_LOG_INFO("CMD 0x21: LED test on PC13");
        kt_led_toggle(&app_led);
        KT_LOG_INFO("PC13 LED logical state: %s",
                    kt_led_get_state(&app_led) ? "ON" : "OFF");
        break;

    case 0x22:
        KT_LOG_INFO("CMD 0x22: Buzzer short beep on PA4");
        KT_LOG_WARN("Buzzer active level must be verified on real hardware");
        kt_buzzer_beep(&app_buzzer, 80);
        break;

    case 0x23:
        KT_LOG_INFO("CMD 0x23: PA0 button state");
        kt_hw_diag_read_button_state();
        break;

    case 0x24:
        KT_LOG_INFO("CMD 0x24: USART roles");
        kt_hw_diag_print_uart_roles();
        break;

    case 0x25:
        KT_LOG_INFO("CMD 0x25: GPIO init status");
        kt_hw_diag_print_gpio_init_status();
        break;

    case 0x30:
        KT_LOG_INFO("CMD 0x30: Seat raw level");
        kt_hw_diag_print_seat_raw_levels();
        break;

    case 0x31:
        KT_LOG_INFO("CMD 0x31: Seat status");
        kt_hw_diag_print_seat_occupied_status();
        break;

    case 0x40:
        KT_LOG_INFO("CMD 0x40: RC522 init");
        kt_rc522_init();
        break;

    case 0x41:
        KT_LOG_INFO("CMD 0x41: RC522 read UID");
        {
            uint8_t uid[5];
            (void)kt_rc522_read_uid(uid);
        }
        break;

    case 0x50:
        KT_LOG_INFO("CMD 0x50: OLED test text");
        kt_oled_test();
        break;

    case 0x60:
        KT_LOG_INFO("CMD 0x60: DS1302 read time");
        kt_ds1302_print_time();
        break;

    case 0x61:
        KT_LOG_INFO("CMD 0x61: DS1302 write test time");
        kt_ds1302_write_test_time();
        break;

    case 0x70:
        KT_LOG_INFO("CMD 0x70: Stepper forward");
        kt_stepper_forward_test();
        break;

    case 0x71:
        KT_LOG_INFO("CMD 0x71: Stepper reverse");
        kt_stepper_reverse_test();
        break;

    case 0x72:
        KT_LOG_INFO("CMD 0x72: Stepper stop");
        kt_stepper_stop();
        break;

    case 0x80:
        KT_LOG_INFO("CMD 0x80: USART1 ZigBee test TX");
        kt_zigbee_send_test();
        break;

    case 0x81:
        KT_LOG_INFO("CMD 0x81: USART1 ZigBee recent RX");
        kt_zigbee_print_recent_rx();
        break;

    case 0x82:
        KT_LOG_INFO("CMD 0x82: USART1 ZigBee PING");
        kt_zigbee_send_ping();
        break;

    case 0x90:
        KT_LOG_INFO("CMD 0x90: USART3 ESP32S3 test TX");
        kt_esp32s3_send_test();
        break;

    default:
        KT_LOG_WARN("Unknown command: cmd=0x%02X data=0x%02X", cmd, data);
        break;
    }
}

/**
 * @brief Print current debug / command status (v1.0 multi-instance)
 *
 *        Includes protocol statistics to help diagnose frame loss,
 *        UART overflows, partial-frame timeouts, and invalid frames.
 */
void kt_cmd_print_status(void)
{
    kt_hw_diag_print_debug_status();
}
