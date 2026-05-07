#include "kt_cmd.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_debug.h"
#include "kt_app/app_io.h"
#include "kt_app/main_controller_app.h"
#include "kt_app/main_keys.h"
#include "kt_modules/kt_hw_diag.h"
#include "kt_modules/kt_rc522.h"
#include "kt_modules/kt_oled.h"
#include "kt_modules/kt_ds1302.h"
#include "kt_modules/kt_stepper.h"
#include "kt_modules/kt_uart_links.h"
#include "kt_modules/kt_esp32_link.h"
#include "kt_system/kt_system_health.h"

static kt_ds1302_time_t pending_ds1302_time = {
    26U, 4U, 29U, 11U, 28U, 0U
};

static void print_pending_ds1302_time(void)
{
    KT_LOG_INFO("DS1302 pending time: 20%02u-%02u-%02u %02u:%02u:%02u",
                (unsigned int)pending_ds1302_time.year,
                (unsigned int)pending_ds1302_time.month,
                (unsigned int)pending_ds1302_time.day,
                (unsigned int)pending_ds1302_time.hour,
                (unsigned int)pending_ds1302_time.minute,
                (unsigned int)pending_ds1302_time.second);
}

static void set_pending_year(uint8_t data)
{
    if (data > 99U) {
        KT_LOG_WARN("Invalid DS1302 year: %02X", data);
        return;
    }
    pending_ds1302_time.year = data;
    KT_LOG_INFO("DS1302 pending year: 20%02u", (unsigned int)data);
}

static void set_pending_month(uint8_t data)
{
    if (data < 1U || data > 12U) {
        KT_LOG_WARN("Invalid DS1302 month: %02X", data);
        return;
    }
    pending_ds1302_time.month = data;
    KT_LOG_INFO("DS1302 pending month: %02u", (unsigned int)data);
}

static void set_pending_day(uint8_t data)
{
    if (data < 1U || data > 31U) {
        KT_LOG_WARN("Invalid DS1302 day: %02X", data);
        return;
    }
    pending_ds1302_time.day = data;
    KT_LOG_INFO("DS1302 pending day: %02u", (unsigned int)data);
}

static void set_pending_hour(uint8_t data)
{
    if (data > 23U) {
        KT_LOG_WARN("Invalid DS1302 hour: %02X", data);
        return;
    }
    pending_ds1302_time.hour = data;
    KT_LOG_INFO("DS1302 pending hour: %02u", (unsigned int)data);
}

static void set_pending_minute(uint8_t data)
{
    if (data > 59U) {
        KT_LOG_WARN("Invalid DS1302 minute: %02X", data);
        return;
    }
    pending_ds1302_time.minute = data;
    KT_LOG_INFO("DS1302 pending minute: %02u", (unsigned int)data);
}

static void set_pending_second(uint8_t data)
{
    if (data > 59U) {
        KT_LOG_WARN("Invalid DS1302 second: %02X", data);
        return;
    }
    pending_ds1302_time.second = data;
    KT_LOG_INFO("DS1302 pending second: %02u", (unsigned int)data);
}

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
 *   0x62  YY      Set pending DS1302 year, 0-99 means 20YY
 *   0x63  MM      Set pending DS1302 month, 1-12
 *   0x64  DD      Set pending DS1302 day, 1-31
 *   0x65  hh      Set pending DS1302 hour, 0-23
 *   0x66  mm      Set pending DS1302 minute, 0-59
 *   0x67  ss      Set pending DS1302 second, 0-59
 *   0x68  0x00    Commit pending DS1302 time
 *   0x69  0x00    Print pending DS1302 time
 *   0x70  0x00    Stepper forward test
 *   0x71  0x00    Stepper reverse test
 *   0x72  0x00    Stepper stop
 *   0x80  0x00    USART1 ZigBee test TX
 *   0x81  0x00    Print USART1 ZigBee recent RX
 *   0x82  0x00    USART1 ZigBee PING TX
 *   0x90  0x00    USART3 ESP32S3 test TX
 *   0xA0  0x00    Print local main controller app status
 *   0xA1  0x00    Run local card flow test
 *   0xA2  0x00    Show OLED home page
 *   0xA3  0x00    Show OLED last card result
 *   0xA4  0x00    Buzzer success prompt
 *   0xA5  0x00    Buzzer fail prompt
 *   0xA6  0x00    Toggle simulated seat status
 *   0xB0  0x00    Print 8-key mapping
 *   0xB1  0x00    Print 8-key raw levels
 *   0xB2  0x00    Print last key event
 *   0xB3  0x00    Enter time set UI
 *   0xB4  0x00    Enter card add UI
 *   0xB5  0x00    Enter card delete UI
 *   0xB6  0x00    Print local registered card list
 *   0xB7  0x00    Clear RAM card list
 *   0xC0  0x00    Print current-boot RAM access log
 *   0xC1  0x00    Clear current-boot RAM access log
 *   0xC2  0x00    Print current-boot RAM access log stats
 *   0xD0  0x00    Send ESP32 binary PING
 *   0xD1  0x00    Print ESP32 recent binary RX
 *   0xD2  0x00    Print ESP32 link status
 *   0xD3  0x00    Send mock CARD_EVENT to ESP32
 *   0xD4  0x00    Send latest real CARD_EVENT to ESP32
 *   0xD5  0x00    Send bad CRC test frame to ESP32
 */
void kt_cmd_dispatch(uint8_t cmd, uint8_t data)
{
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

    case 0x62:
        set_pending_year(data);
        break;

    case 0x63:
        set_pending_month(data);
        break;

    case 0x64:
        set_pending_day(data);
        break;

    case 0x65:
        set_pending_hour(data);
        break;

    case 0x66:
        set_pending_minute(data);
        break;

    case 0x67:
        set_pending_second(data);
        break;

    case 0x68:
        KT_LOG_INFO("CMD 0x68: DS1302 commit pending time");
        (void)kt_ds1302_set_time(&pending_ds1302_time);
        break;

    case 0x69:
        KT_LOG_INFO("CMD 0x69: DS1302 pending time");
        print_pending_ds1302_time();
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

    case 0x83:
        KT_LOG_INFO("CMD 0x83: USART1 ZigBee link info + ping");
        kt_zigbee_print_link_info();
        kt_zigbee_send_ping();
        break;

    case 0x84:
        KT_LOG_INFO("CMD 0x84: USART1 ZigBee statistics");
        kt_zigbee_print_link_info();
        break;

    case 0x85:
        KT_LOG_INFO("CMD 0x85: Clear USART1 ZigBee statistics");
        kt_zigbee_clear_stats();
        break;

    case 0x86:
        KT_LOG_INFO("CMD 0x86: USART1 ZigBee binary PING");
        kt_zigbee_send_ping();
        break;

    case 0x90:
        KT_LOG_INFO("CMD 0x90: USART3 ESP32S3 test TX");
        kt_esp32s3_send_test();
        break;

    case 0xA0:
        KT_LOG_INFO("CMD 0xA0: Main app status");
        main_controller_app_print_status();
        break;

    case 0xA1:
        KT_LOG_INFO("CMD 0xA1: Local card flow test");
        main_controller_app_run_card_flow();
        break;

    case 0xA2:
        KT_LOG_INFO("CMD 0xA2: OLED home");
        main_keys_set_home_state();
        main_controller_app_show_home();
        break;

    case 0xA3:
        KT_LOG_INFO("CMD 0xA3: OLED last card result");
        main_controller_app_show_last_card();
        break;

    case 0xA4:
        KT_LOG_INFO("CMD 0xA4: Buzzer success prompt");
        main_controller_app_beep_success();
        break;

    case 0xA5:
        KT_LOG_INFO("CMD 0xA5: Buzzer fail prompt");
        main_controller_app_beep_fail();
        break;

    case 0xA6:
        KT_LOG_INFO("CMD 0xA6: Simulate seat status");
        main_controller_app_toggle_seat_state();
        break;

    case 0xB0:
        KT_LOG_INFO("CMD 0xB0: Main key mapping");
        main_keys_print_mapping();
        break;

    case 0xB1:
        KT_LOG_INFO("CMD 0xB1: Main key raw levels");
        main_keys_print_raw_levels();
        break;

    case 0xB2:
        KT_LOG_INFO("CMD 0xB2: Last main key event");
        main_keys_print_last_event();
        break;

    case 0xB3:
        KT_LOG_INFO("CMD 0xB3: Enter time set UI");
        main_keys_enter_time_set();
        break;

    case 0xB4:
        KT_LOG_INFO("CMD 0xB4: Enter card add UI");
        main_keys_enter_card_add();
        break;

    case 0xB5:
        KT_LOG_INFO("CMD 0xB5: Enter card delete UI");
        main_keys_enter_card_del();
        break;

    case 0xB6:
        KT_LOG_INFO("CMD 0xB6: Print local card DB");
        main_controller_app_print_card_db();
        break;

    case 0xB7:
        KT_LOG_INFO("CMD 0xB7: Clear local card DB");
        main_controller_app_clear_card_db();
        break;

    case 0xC0:
        KT_LOG_INFO("CMD 0xC0: Print RAM access log");
        main_controller_app_print_access_log();
        break;

    case 0xC1:
        KT_LOG_INFO("CMD 0xC1: Clear RAM access log");
        main_controller_app_clear_access_log();
        break;

    case 0xC2:
        KT_LOG_INFO("CMD 0xC2: RAM access log stats");
        main_controller_app_print_access_stats();
        break;

    case 0xD0:
        KT_LOG_INFO("CMD 0xD0: ESP32 binary PING");
        kt_esp32_link_send_ping();
        break;

    case 0xD1:
        KT_LOG_INFO("CMD 0xD1: ESP32 recent RX");
        kt_esp32_link_print_recent_rx();
        break;

    case 0xD2:
        KT_LOG_INFO("CMD 0xD2: ESP32 link status");
        kt_esp32_link_print_status();
        break;

    case 0xD3:
        KT_LOG_INFO("CMD 0xD3: ESP32 mock CARD_EVENT");
        kt_esp32_link_send_mock_card_event();
        break;

    case 0xD4:
        KT_LOG_INFO("CMD 0xD4: ESP32 latest CARD_EVENT");
        main_controller_app_send_last_card_event_to_esp32();
        break;

    case 0xD5:
        KT_LOG_INFO("CMD 0xD5: ESP32 bad CRC test");
        kt_esp32_link_send_bad_crc_test();
        break;

    case 0xE0:
        KT_LOG_INFO("CMD 0xE0: System health");
        kt_system_health_print();
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
