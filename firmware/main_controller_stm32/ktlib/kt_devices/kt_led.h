#ifndef KT_LED_H
#define KT_LED_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED device descriptor (multi-instance)
 *
 * Each physical LED is represented by one kt_led_t object.
 * Supports both active-high and active-low wiring.
 */
typedef struct
{
    GPIO_TypeDef  *port;            /**< GPIO port (e.g. GPIOC)             */
    uint16_t       pin;             /**< GPIO pin  (e.g. GPIO_PIN_13)       */
    GPIO_PinState  active_level;    /**< Level that means "LED ON"          */
    uint8_t        state;           /**< Logical state: 1=ON, 0=OFF         */

    uint8_t        blink_enable;    /**< 1 = blink active, 0 = off          */
    uint32_t       blink_period_ms; /**< Full blink cycle period (ms)       */
    uint32_t       last_toggle_ms;  /**< kt_tick_get_ms() at last toggle    */
} kt_led_t;

/**
 * @brief Initialize an LED instance
 * @param led          Pointer to LED descriptor
 * @param port         GPIO port (e.g. GPIOC)
 * @param pin          GPIO pin  (e.g. GPIO_PIN_13)
 * @param active_level Level that turns the LED ON (GPIO_PIN_RESET or GPIO_PIN_SET)
 */
void kt_led_init(kt_led_t *led,
                 GPIO_TypeDef *port,
                 uint16_t pin,
                 GPIO_PinState active_level);

/**
 * @brief Turn LED on
 * @param led  LED descriptor (NULL-safe)
 */
void kt_led_on(kt_led_t *led);

/**
 * @brief Turn LED off
 * @param led  LED descriptor (NULL-safe)
 */
void kt_led_off(kt_led_t *led);

/**
 * @brief Toggle LED state
 * @param led  LED descriptor (NULL-safe)
 */
void kt_led_toggle(kt_led_t *led);

/**
 * @brief Set LED state directly
 * @param led  LED descriptor (NULL-safe)
 * @param on   1 = ON, 0 = OFF
 */
void kt_led_set(kt_led_t *led, uint8_t on);

/**
 * @brief Get current logical LED state
 * @param led  LED descriptor (NULL-safe)
 * @return 1 if ON, 0 if OFF (returns 0 on NULL)
 */
uint8_t kt_led_get_state(kt_led_t *led);

/**
 * @brief Start LED blinking (non-blocking)
 * @param led        LED descriptor (NULL-safe)
 * @param period_ms  Full blink cycle period in ms (e.g. 500)
 */
void kt_led_blink_start(kt_led_t *led, uint32_t period_ms);

/**
 * @brief Stop LED blinking (restores current logical state)
 * @param led  LED descriptor (NULL-safe)
 */
void kt_led_blink_stop(kt_led_t *led);

/**
 * @brief LED task — call periodically (e.g. every 10-20 ms) from scheduler.
 *        Handles non-blocking blink timing via kt_tick_get_ms().
 * @param led  LED descriptor (NULL-safe)
 */
void kt_led_task(kt_led_t *led);

#ifdef __cplusplus
}
#endif

#endif /* KT_LED_H */