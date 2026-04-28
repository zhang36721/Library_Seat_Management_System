#include "kt_led.h"
#include "kt_system/kt_tick.h"
#include <stddef.h>

/**
 * @brief Initialize an LED instance
 */
void kt_led_init(kt_led_t *led,
                 GPIO_TypeDef *port,
                 uint16_t pin,
                 GPIO_PinState active_level)
{
    if (led == NULL) return;

    led->port           = port;
    led->pin            = pin;
    led->active_level   = active_level;
    led->state          = 0;
    led->blink_enable   = 0;
    led->blink_period_ms = 500;
    led->last_toggle_ms  = 0;

    /* Turn LED off initially */
    GPIO_PinState off_level = (active_level == GPIO_PIN_RESET)
                              ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(led->port, led->pin, off_level);
}

/**
 * @brief Turn LED on
 */
void kt_led_on(kt_led_t *led)
{
    if (led == NULL) return;

    led->blink_enable = 0;
    led->state = 1;
    HAL_GPIO_WritePin(led->port, led->pin, led->active_level);
}

/**
 * @brief Turn LED off
 */
void kt_led_off(kt_led_t *led)
{
    if (led == NULL) return;

    led->blink_enable = 0;
    led->state = 0;
    GPIO_PinState off_level = (led->active_level == GPIO_PIN_RESET)
                              ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(led->port, led->pin, off_level);
}

/**
 * @brief Toggle LED state
 */
void kt_led_toggle(kt_led_t *led)
{
    if (led == NULL) return;

    led->blink_enable = 0;
    HAL_GPIO_TogglePin(led->port, led->pin);
    led->state = !led->state;
}

/**
 * @brief Set LED state directly
 */
void kt_led_set(kt_led_t *led, uint8_t on)
{
    if (on) {
        kt_led_on(led);
    } else {
        kt_led_off(led);
    }
}

/**
 * @brief Get current logical LED state
 */
uint8_t kt_led_get_state(kt_led_t *led)
{
    if (led == NULL) return 0;
    return led->state;
}

/**
 * @brief Start LED blinking (non-blocking)
 */
void kt_led_blink_start(kt_led_t *led, uint32_t period_ms)
{
    if (led == NULL) return;

    if (period_ms == 0) {
        period_ms = 500;
    }
    led->blink_enable    = 1;
    led->blink_period_ms = period_ms;
    led->last_toggle_ms  = kt_tick_get_ms();
}

/**
 * @brief Stop LED blinking
 */
void kt_led_blink_stop(kt_led_t *led)
{
    if (led == NULL) return;
    led->blink_enable = 0;
}

/**
 * @brief LED task — handle non-blocking blink timing
 */
void kt_led_task(kt_led_t *led)
{
    if (led == NULL) return;
    if (!led->blink_enable) return;

    uint32_t now = kt_tick_get_ms();
    uint32_t half_period = led->blink_period_ms / 2;

    if (kt_tick_is_timeout(led->last_toggle_ms, half_period)) {
        HAL_GPIO_TogglePin(led->port, led->pin);
        led->state = !led->state;
        led->last_toggle_ms = now;
    }
}