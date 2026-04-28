#include "kt_buzzer.h"
#include "kt_system/kt_tick.h"
#include <stddef.h>

/*---------------------------------------------------------------------------
 * Internal: set GPIO to "buzzer on" level
 *---------------------------------------------------------------------------*/
static void buzzer_gpio_on(kt_buzzer_t *buz)
{
    HAL_GPIO_WritePin(buz->port, buz->pin, buz->active_level);
}

/*---------------------------------------------------------------------------
 * Internal: set GPIO to "buzzer off" level
 *---------------------------------------------------------------------------*/
static void buzzer_gpio_off(kt_buzzer_t *buz)
{
    GPIO_PinState off_level = (buz->active_level == GPIO_PIN_RESET)
                              ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(buz->port, buz->pin, off_level);
}

/*---------------------------------------------------------------------------
 * Public API
 *---------------------------------------------------------------------------*/

void kt_buzzer_init(kt_buzzer_t *buz,
                    GPIO_TypeDef *port,
                    uint16_t pin,
                    GPIO_PinState active_level)
{
    if (buz == NULL) return;

    buz->port            = port;
    buz->pin             = pin;
    buz->active_level    = active_level;
    buz->state           = 0;
    buz->blink_enable    = 0;
    buz->blink_on_ms     = 100;
    buz->blink_off_ms    = 400;
    buz->last_toggle_ms  = 0;

    buzzer_gpio_off(buz);
}

void kt_buzzer_on(kt_buzzer_t *buz)
{
    if (buz == NULL) return;

    buz->blink_enable = 0;
    buz->state = 1;
    buzzer_gpio_on(buz);
}

void kt_buzzer_off(kt_buzzer_t *buz)
{
    if (buz == NULL) return;

    buz->blink_enable = 0;
    buz->state = 0;
    buzzer_gpio_off(buz);
}

void kt_buzzer_set(kt_buzzer_t *buz, uint8_t on)
{
    if (on) {
        kt_buzzer_on(buz);
    } else {
        kt_buzzer_off(buz);
    }
}

void kt_buzzer_cycle_start(kt_buzzer_t *buz, uint32_t on_ms, uint32_t off_ms)
{
    if (buz == NULL) return;

    if (on_ms == 0)  on_ms  = 100;
    if (off_ms == 0) off_ms = 400;

    buz->blink_enable   = 1;
    buz->blink_on_ms    = on_ms;
    buz->blink_off_ms   = off_ms;
    buz->last_toggle_ms = kt_tick_get_ms();

    /* Start in ON state */
    buz->state = 1;
    buzzer_gpio_on(buz);
}

void kt_buzzer_cycle_stop(kt_buzzer_t *buz)
{
    if (buz == NULL) return;
    kt_buzzer_off(buz);
}

void kt_buzzer_beep(kt_buzzer_t *buz, uint32_t on_ms)
{
    if (buz == NULL) return;

    /* One-shot: ON for on_ms, then OFF (off_ms=0 signals one-shot mode) */
    if (on_ms == 0) on_ms = 100;

    buz->blink_enable   = 1;
    buz->blink_on_ms    = on_ms;
    buz->blink_off_ms   = 0;  /* Zero means one-shot */
    buz->last_toggle_ms = kt_tick_get_ms();

    buz->state = 1;
    buzzer_gpio_on(buz);
}

void kt_buzzer_task(kt_buzzer_t *buz)
{
    if (buz == NULL) return;
    if (!buz->blink_enable) return;

    uint32_t now = kt_tick_get_ms();

    /* One-shot mode (off_ms == 0): auto-stop after on_ms */
    if (buz->blink_off_ms == 0) {
        if (kt_tick_is_timeout(buz->last_toggle_ms, buz->blink_on_ms)) {
            kt_buzzer_off(buz);
        }
        return;
    }

    /* Cycling mode */
    if (buz->state) {
        /* Currently ON: wait for on_ms then switch OFF */
        if (kt_tick_is_timeout(buz->last_toggle_ms, buz->blink_on_ms)) {
            buz->state = 0;
            buzzer_gpio_off(buz);
            buz->last_toggle_ms = now;
        }
    } else {
        /* Currently OFF: wait for off_ms then switch ON */
        if (kt_tick_is_timeout(buz->last_toggle_ms, buz->blink_off_ms)) {
            buz->state = 1;
            buzzer_gpio_on(buz);
            buz->last_toggle_ms = now;
        }
    }
}