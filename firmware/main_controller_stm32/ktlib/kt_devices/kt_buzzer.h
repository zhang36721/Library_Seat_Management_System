#ifndef KT_BUZZER_H
#define KT_BUZZER_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Buzzer device descriptor (multi-instance)
 *
 * Each physical buzzer is represented by one kt_buzzer_t object.
 * Supports both active-high and active-low wiring.
 *
 * Blink timing is independent per instance.
 */
typedef struct
{
    GPIO_TypeDef  *port;             /**< GPIO port (e.g. GPIOA)             */
    uint16_t       pin;              /**< GPIO pin  (e.g. GPIO_PIN_4)        */
    GPIO_PinState  active_level;     /**< Level that means "BUZZER ON"       */

    uint8_t        state;            /**< Logical state: 1=ON, 0=OFF         */
    uint8_t        blink_enable;     /**< 1 = blink active, 0 = off          */
    uint32_t       blink_on_ms;      /**< ON time per cycle (ms)             */
    uint32_t       blink_off_ms;     /**< OFF time per cycle (ms)            */
    uint32_t       last_toggle_ms;   /**< kt_tick_get_ms() at last toggle    */
} kt_buzzer_t;

/**
 * @brief Initialize a buzzer instance
 * @param buz          Pointer to buzzer descriptor
 * @param port         GPIO port (e.g. GPIOA)
 * @param pin          GPIO pin  (e.g. GPIO_PIN_4)
 * @param active_level Level that turns the buzzer ON (GPIO_PIN_RESET or GPIO_PIN_SET)
 */
void kt_buzzer_init(kt_buzzer_t *buz,
                    GPIO_TypeDef *port,
                    uint16_t pin,
                    GPIO_PinState active_level);

/**
 * @brief Turn buzzer on (continuously, no blink)
 * @param buz  Buzzer descriptor (NULL-safe)
 */
void kt_buzzer_on(kt_buzzer_t *buz);

/**
 * @brief Turn buzzer off
 * @param buz  Buzzer descriptor (NULL-safe)
 */
void kt_buzzer_off(kt_buzzer_t *buz);

/**
 * @brief Set buzzer state directly
 * @param buz  Buzzer descriptor (NULL-safe)
 * @param on   1 = ON, 0 = OFF
 */
void kt_buzzer_set(kt_buzzer_t *buz, uint8_t on);

/**
 * @brief Start cycling buzzer on/off (non-blocking)
 * @param buz        Buzzer descriptor (NULL-safe)
 * @param on_ms      ON time in ms per cycle (e.g. 100)
 * @param off_ms     OFF time in ms per cycle (e.g. 400)
 */
void kt_buzzer_cycle_start(kt_buzzer_t *buz, uint32_t on_ms, uint32_t off_ms);

/**
 * @brief Stop buzzer cycling (turns buzzer OFF)
 * @param buz  Buzzer descriptor (NULL-safe)
 */
void kt_buzzer_cycle_stop(kt_buzzer_t *buz);

/**
 * @brief Beep once for a specified duration (one-shot, non-blocking)
 *
 *        After on_ms elapses, the buzzer automatically turns OFF.
 *        This is a convenience wrapper around kt_buzzer_cycle_start
 *        with off_ms = 0 which means "auto-stop after one ON pulse".
 *
 * @param buz    Buzzer descriptor (NULL-safe)
 * @param on_ms  ON duration in ms (e.g. 50)
 */
void kt_buzzer_beep(kt_buzzer_t *buz, uint32_t on_ms);

/**
 * @brief Buzzer task — call periodically (e.g. every 10-20 ms) from scheduler.
 *        Handles non-blocking cycling timing.
 * @param buz  Buzzer descriptor (NULL-safe)
 */
void kt_buzzer_task(kt_buzzer_t *buz);

#ifdef __cplusplus
}
#endif

#endif /* KT_BUZZER_H */