#ifndef KT_BUTTON_H
#define KT_BUTTON_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Button device descriptor (multi-instance)
 *
 * Each physical button is represented by one kt_button_t object.
 * Supports both active-low (GND when pressed) and active-high (VCC when pressed).
 */
typedef struct
{
    GPIO_TypeDef  *port;            /**< GPIO port (e.g. GPIOA)             */
    uint16_t       pin;             /**< GPIO pin  (e.g. GPIO_PIN_0)        */
    GPIO_PinState  active_level;    /**< Level that means "pressed"         */

    uint8_t        stable_state;    /**< Debounced state: 1=pressed, 0=idle */
    uint8_t        last_raw_state;  /**< Last raw GPIO read for edge detect */
    uint8_t        click_event;     /**< 1 = click occurred (cleared on read) */

    uint32_t       last_change_ms;  /**< kt_tick_get_ms() at last raw change */
    uint32_t       debounce_ms;     /**< Debounce time in ms (e.g. 20)      */
} kt_button_t;

/**
 * @brief Initialize a button instance
 * @param btn          Pointer to button descriptor
 * @param port         GPIO port (e.g. GPIOA)
 * @param pin          GPIO pin  (e.g. GPIO_PIN_0)
 * @param active_level Level that means "pressed" (GPIO_PIN_RESET or GPIO_PIN_SET)
 * @param debounce_ms  Debounce time in ms (e.g. 20)
 */
void kt_button_init(kt_button_t *btn,
                    GPIO_TypeDef *port,
                    uint16_t pin,
                    GPIO_PinState active_level,
                    uint32_t debounce_ms);

/**
 * @brief Button task — call periodically (e.g. every 10 ms) from scheduler.
 *        Handles non-blocking debounce and click detection.
 * @param btn  Button descriptor (NULL-safe)
 */
void kt_button_task(kt_button_t *btn);

/**
 * @brief Check if button is currently pressed (after debounce)
 * @param btn  Button descriptor (NULL-safe)
 * @return 1 if pressed, 0 if released
 */
uint8_t kt_button_is_pressed(kt_button_t *btn);

/**
 * @brief Get a click event (press + release) and clear it
 * @param btn  Button descriptor (NULL-safe)
 * @return 1 if a click occurred since last read, 0 otherwise
 */
uint8_t kt_button_get_click_event(kt_button_t *btn);

#ifdef __cplusplus
}
#endif

#endif /* KT_BUTTON_H */