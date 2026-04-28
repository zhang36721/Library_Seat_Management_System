#include "kt_cmd.h"
#include "kt_config.h"
#include <stdio.h>
#include "kt_log.h"
#include "kt_port_uart.h"
#include "kt_protocol.h"
#include "kt_debug.h"
#include "kt_app/app_io.h"

static const char *gpio_level_to_str(GPIO_PinState level)
{
    return (level == GPIO_PIN_SET) ? "HIGH" : "LOW";
}

static const char *active_level_to_str(GPIO_PinState level)
{
    return (level == GPIO_PIN_SET) ? "active-high" : "active-low";
}

static void kt_cmd_print_uart_roles(void)
{
    KT_LOG_INFO("USART1: %s", KT_USART1_ROLE);
    KT_LOG_INFO("USART2: %s", KT_USART2_ROLE);
    KT_LOG_INFO("USART3: %s", KT_USART3_ROLE);
    KT_LOG_WARN("ESP32S3 must not share USART2 debug protocol in v0.6");
}

static void kt_cmd_print_gpio_init_status(void)
{
    KT_LOG_INFO("GPIO PC13: output push-pull, default HIGH/OFF, LED active-low");
    KT_LOG_INFO("GPIO PA0 : input pull-up, button assumed active-low, verify with FF 23");
    KT_LOG_INFO("GPIO PA4 : output push-pull, default LOW/OFF, buzzer active level verify with FF 22");
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    KT_LOG_INFO("Seat sensor GPIO: initialized as input pull-up");
#else
    KT_LOG_WARN("Seat sensor GPIO: pins not confirmed; not initialized");
#endif
}

static void kt_cmd_print_hardware_resources(void)
{
    KT_LOG_INFO("=== v0.6 Hardware Resources ===");
    KT_LOG_INFO("MCU   : STM32F103C8T6");
    KT_LOG_INFO("LED   : PC13, output, active-low, default OFF, initialized");
    KT_LOG_INFO("Button: PA0, input pull-up, active-low pending real wiring check, initialized");
    KT_LOG_INFO("Buzzer: PA4, output, default OFF, active level pending FF 22 verification");
    KT_LOG_WARN("Seat sensors: 3 inputs reserved by macros, real pins pending confirmation");
    KT_LOG_WARN("RC522 SPI / OLED / DS1302 / Stepper: pin map pending confirmation");
    kt_cmd_print_uart_roles();
    KT_LOG_INFO("===============================");
}

static void kt_cmd_read_button_state(void)
{
    GPIO_PinState raw = HAL_GPIO_ReadPin(APP_BTN_DEFAULT_PORT, APP_BTN_DEFAULT_PIN);
    KT_LOG_INFO("Button PA0 raw level: %s", gpio_level_to_str(raw));
    KT_LOG_INFO("Button PA0 logical pressed: %u",
                (unsigned int)kt_button_is_pressed(&app_btn));
}

#if APP_SEAT_SENSOR_PINS_CONFIRMED
static const char *port_to_str(GPIO_TypeDef *port)
{
    if (port == GPIOA) return "GPIOA";
    if (port == GPIOB) return "GPIOB";
    if (port == GPIOC) return "GPIOC";
    if (port == GPIOD) return "GPIOD";
    return "UNCONFIRMED";
}

static uint8_t kt_cmd_read_seat_raw(uint8_t index, GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_PinState raw = HAL_GPIO_ReadPin(port, pin);
    KT_LOG_INFO("Seat%u raw: %s %s pin 0x%04X",
                (unsigned int)index,
                port_to_str(port),
                gpio_level_to_str(raw),
                (unsigned int)pin);
    return (raw == GPIO_PIN_SET) ? 1U : 0U;
}
#endif

static void kt_cmd_print_seat_raw_levels(void)
{
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    (void)kt_cmd_read_seat_raw(1, APP_SEAT1_SENSOR_PORT, APP_SEAT1_SENSOR_PIN);
    (void)kt_cmd_read_seat_raw(2, APP_SEAT2_SENSOR_PORT, APP_SEAT2_SENSOR_PIN);
    (void)kt_cmd_read_seat_raw(3, APP_SEAT3_SENSOR_PORT, APP_SEAT3_SENSOR_PIN);
#else
    KT_LOG_WARN("Seat sensor pins not configured");
    KT_LOG_INFO("Set APP_SEAT_SENSOR_PINS_CONFIRMED=1 after wiring confirmed");
#endif
}

static void kt_cmd_print_seat_occupied_status(void)
{
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    uint8_t s1 = kt_cmd_read_seat_raw(1, APP_SEAT1_SENSOR_PORT, APP_SEAT1_SENSOR_PIN);
    uint8_t s2 = kt_cmd_read_seat_raw(2, APP_SEAT2_SENSOR_PORT, APP_SEAT2_SENSOR_PIN);
    uint8_t s3 = kt_cmd_read_seat_raw(3, APP_SEAT3_SENSOR_PORT, APP_SEAT3_SENSOR_PIN);
    uint8_t active = (APP_SEAT_SENSOR_ACTIVE_LEVEL == GPIO_PIN_SET) ? 1U : 0U;

    KT_LOG_INFO("Seat sensor active level: %s",
                active_level_to_str(APP_SEAT_SENSOR_ACTIVE_LEVEL));
    KT_LOG_INFO("Seat1: %s", (s1 == active) ? "OCCUPIED" : "FREE");
    KT_LOG_INFO("Seat2: %s", (s2 == active) ? "OCCUPIED" : "FREE");
    KT_LOG_INFO("Seat3: %s", (s3 == active) ? "OCCUPIED" : "FREE");
#else
    KT_LOG_WARN("Seat sensor pins not configured");
#endif
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
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "Button pressed: %u, click event: %u",
                     (unsigned int)kt_button_is_pressed(&app_btn),
                     (unsigned int)kt_button_get_click_event(&app_btn));
            KT_LOG_INFO("%s", buf);
        }
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
        kt_cmd_print_hardware_resources();
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
        kt_cmd_read_button_state();
        break;

    case 0x24:
        KT_LOG_INFO("CMD 0x24: USART roles");
        kt_cmd_print_uart_roles();
        break;

    case 0x25:
        KT_LOG_INFO("CMD 0x25: GPIO init status");
        kt_cmd_print_gpio_init_status();
        break;

    case 0x30:
        KT_LOG_INFO("CMD 0x30: Seat raw level");
        kt_cmd_print_seat_raw_levels();
        break;

    case 0x31:
        KT_LOG_INFO("CMD 0x31: Seat status");
        kt_cmd_print_seat_occupied_status();
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
    /* Buffer for single-line formatted output */
    char buf[64];

    KT_LOG_INFO("=== Debug Status (v1.0) ===");
    KT_LOG_INFO("LED State      : %s", kt_led_get_state(&app_led) ? "ON" : "OFF");

    KT_LOG_INFO("Protocol       : FF CMD DATA FF");
    KT_LOG_INFO("Uptime         : %lu s", (unsigned long)(HAL_GetTick() / 1000U));

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
