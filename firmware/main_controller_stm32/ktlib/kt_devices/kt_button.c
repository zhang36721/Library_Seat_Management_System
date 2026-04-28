#include "kt_button.h"
#include "kt_system/kt_tick.h"
#include <stddef.h>

/**
 * @brief Read raw GPIO level (0 = released, 1 = pressed in logical sense)
 */
static uint8_t kt_button_read_raw(kt_button_t *btn)
{
    if (btn == NULL) return 0;

    GPIO_PinState raw = HAL_GPIO_ReadPin(btn->port, btn->pin);
    return (raw == btn->active_level) ? 1 : 0;
}

/**
 * @brief Initialize a button instance
 */
void kt_button_init(kt_button_t *btn,
                    GPIO_TypeDef *port,
                    uint16_t pin,
                    GPIO_PinState active_level,
                    uint32_t debounce_ms)
{
    if (btn == NULL) return;

    btn->port           = port;
    btn->pin            = pin;
    btn->active_level   = active_level;
    btn->debounce_ms    = (debounce_ms > 0) ? debounce_ms : 20;

    btn->stable_state   = 0;
    btn->last_raw_state = 0;
    btn->click_event    = 0;
    btn->last_change_ms = 0;
}

/**
 * @brief Button task — non-blocking debounce with click detection
 *
 * State machine:
 *   IDLE → (raw change) → WAIT_STABLE → (debounce elapsed) → update stable_state
 *   If stable_state goes 0→1: press detected
 *   If stable_state goes 1→0: release → click_event = 1
 */
void kt_button_task(kt_button_t *btn)
{
    if (btn == NULL) return;

    uint8_t raw = kt_button_read_raw(btn);
    uint32_t now = kt_tick_get_ms();

    /* Detect raw level change */
    if (raw != btn->last_raw_state) {
        btn->last_raw_state = raw;
        btn->last_change_ms = now;
        return;  /* Wait for debounce period */
    }

    /* Check if debounce period has elapsed */
    if (kt_tick_is_timeout(btn->last_change_ms, btn->debounce_ms)) {
        /* Raw level is stable — update stable state */
        if (raw != btn->stable_state) {
            /* Rising edge (release): generate click */
            if (btn->stable_state == 1 && raw == 0) {
                btn->click_event = 1;
            }
            btn->stable_state = raw;
        }
    }
}

/**
 * @brief Check if button is currently pressed (after debounce)
 */
uint8_t kt_button_is_pressed(kt_button_t *btn)
{
    if (btn == NULL) return 0;
    return btn->stable_state;
}

/**
 * @brief Get a click event and clear it
 */
uint8_t kt_button_get_click_event(kt_button_t *btn)
{
    if (btn == NULL) return 0;
    uint8_t evt = btn->click_event;
    btn->click_event = 0;
    return evt;
}