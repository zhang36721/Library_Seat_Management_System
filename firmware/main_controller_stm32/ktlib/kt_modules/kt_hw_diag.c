#include "kt_hw_diag.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_app/app_io.h"
#include "kt_port_uart.h"
#include "kt_protocol.h"
#include <stdint.h>
#include <stdio.h>

static const char *gpio_level_to_str(GPIO_PinState level)
{
    return (level == GPIO_PIN_SET) ? "HIGH" : "LOW";
}

static const char *active_level_to_str(GPIO_PinState level)
{
    return (level == GPIO_PIN_SET) ? "active-high" : "active-low";
}

void kt_hw_diag_print_uart_roles(void)
{
    KT_LOG_INFO("USART1: %s", KT_USART1_ROLE);
    KT_LOG_INFO("USART2: %s", KT_USART2_ROLE);
    KT_LOG_INFO("USART3: %s", KT_USART3_ROLE);
    KT_LOG_WARN("ESP32S3 and ZigBee must not share USART2 debug protocol");
}

void kt_hw_diag_print_debug_status(void)
{
    char buf[64];

    KT_LOG_INFO("=== Debug Status (v0.7) ===");
    KT_LOG_INFO("LED State      : %s", kt_led_get_state(&app_led) ? "ON" : "OFF");
    KT_LOG_INFO("Protocol       : FF CMD DATA FF");
    KT_LOG_INFO("Uptime         : %lu s", (unsigned long)(HAL_GetTick() / 1000U));
    KT_LOG_INFO("Build          : %s %s", __DATE__, __TIME__);

    snprintf(buf, sizeof(buf), "RX ring avail  : %u", kt_port_uart_rx_available());
    KT_LOG_INFO("%s", buf);

    snprintf(buf, sizeof(buf), "RX overflow    : %lu",
             (unsigned long)kt_port_uart_rx_overflow_count());
    KT_LOG_INFO("%s", buf);

    snprintf(buf, sizeof(buf), "Timeout drops  : %lu",
             (unsigned long)kt_protocol_get_timeout_drop_count());
    KT_LOG_INFO("%s", buf);

    snprintf(buf, sizeof(buf), "Error drops    : %lu",
             (unsigned long)kt_protocol_get_error_drop_count());
    KT_LOG_INFO("%s", buf);

    KT_LOG_INFO("===========================");
}

void kt_hw_diag_print_gpio_init_status(void)
{
    KT_LOG_INFO("GPIO PC13: output push-pull, default HIGH/OFF, LED active-low");
    KT_LOG_INFO("GPIO PA0 : input pull-up, button assumed active-low, verify with FF 23");
    KT_LOG_INFO("GPIO PA4 : output push-pull, default LOW/OFF, buzzer active level verify with FF 22");
    KT_LOG_INFO("GPIO RC522: PA5/PA7/PB12/PB13 outputs, PA6 input");
    KT_LOG_INFO("GPIO OLED : PB6/PB7 open-drain software I2C");
    KT_LOG_INFO("GPIO DS1302: PB0/PB5 outputs, PB1 bidirectional");
    KT_LOG_INFO("GPIO Stepper: PB8/PB9/PB14/PB15 outputs default LOW");
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    KT_LOG_INFO("Seat sensor GPIO: initialized as input pull-up");
#else
    KT_LOG_WARN("Seat sensor GPIO: pins not confirmed; not initialized");
#endif
}

void kt_hw_diag_print_hardware_resources(void)
{
    KT_LOG_INFO("=== v0.7 Hardware Resources ===");
    KT_LOG_INFO("MCU   : STM32F103C8T6");
    KT_LOG_INFO("LED   : PC13, output, active-low, default OFF, initialized");
    KT_LOG_INFO("Button: PA0, input pull-up, active-low pending real wiring check, initialized");
    KT_LOG_INFO("Buzzer: PA4, output, default OFF, active level pending FF 22 verification");
    KT_LOG_WARN("Seat sensors: 3 inputs reserved by macros, real pins pending confirmation");
    KT_LOG_INFO("RC522: PA5/PA6/PA7/PB12/PB13 software SPI");
    KT_LOG_INFO("OLED : PB6/PB7 software I2C");
    KT_LOG_INFO("DS1302: PB0/PB1/PB5 GPIO");
    KT_LOG_INFO("Stepper: PB8/PB9/PB14/PB15 GPIO");
    kt_hw_diag_print_uart_roles();
    KT_LOG_INFO("===============================");
}

void kt_hw_diag_read_button_state(void)
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

static uint8_t read_seat_raw(uint8_t index, GPIO_TypeDef *port, uint16_t pin)
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

void kt_hw_diag_print_seat_raw_levels(void)
{
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    (void)read_seat_raw(1, APP_SEAT1_SENSOR_PORT, APP_SEAT1_SENSOR_PIN);
    (void)read_seat_raw(2, APP_SEAT2_SENSOR_PORT, APP_SEAT2_SENSOR_PIN);
    (void)read_seat_raw(3, APP_SEAT3_SENSOR_PORT, APP_SEAT3_SENSOR_PIN);
#else
    KT_LOG_WARN("Seat sensor pins not configured");
    KT_LOG_INFO("Set APP_SEAT_SENSOR_PINS_CONFIRMED=1 after wiring confirmed");
#endif
}

void kt_hw_diag_print_seat_occupied_status(void)
{
#if APP_SEAT_SENSOR_PINS_CONFIRMED
    uint8_t s1 = read_seat_raw(1, APP_SEAT1_SENSOR_PORT, APP_SEAT1_SENSOR_PIN);
    uint8_t s2 = read_seat_raw(2, APP_SEAT2_SENSOR_PORT, APP_SEAT2_SENSOR_PIN);
    uint8_t s3 = read_seat_raw(3, APP_SEAT3_SENSOR_PORT, APP_SEAT3_SENSOR_PIN);
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
