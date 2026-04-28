#include "app_io.h"
#include "kt_system/kt_tick.h"

/*===========================================================================
 * Application device instances (global, multi-instance descriptors)
 *
 * These are the concrete hardware mappings for this board.
 * Changing wiring? Change these instances, not the device drivers.
 *===========================================================================*/

kt_led_t    app_led;
kt_button_t app_btn;
kt_buzzer_t app_buzzer;

/*===========================================================================
 * app_io_init
 *===========================================================================*/
void app_io_init(void)
{
    /* Initialize LED: PC13, active-low */
    kt_led_init(&app_led,
                APP_LED_DEFAULT_PORT,
                APP_LED_DEFAULT_PIN,
                APP_LED_DEFAULT_ACTIVE_LEVEL);

    /* Initialize Button: PA0, active-low, 20ms debounce */
    kt_button_init(&app_btn,
                   APP_BTN_DEFAULT_PORT,
                   APP_BTN_DEFAULT_PIN,
                   APP_BTN_DEFAULT_ACTIVE_LEVEL,
                   APP_BTN_DEFAULT_DEBOUNCE_MS);

    /* Initialize Buzzer: PA4, active level must be verified with FF 22 */
    kt_buzzer_init(&app_buzzer,
                   APP_BUZZER_DEFAULT_PORT,
                   APP_BUZZER_DEFAULT_PIN,
                   APP_BUZZER_DEFAULT_ACTIVE_LEVEL);
}

/*===========================================================================
 * app_io_tasks
 *===========================================================================*/
void app_io_tasks(void)
{
    /* Non-blocking LED blink timing */
    kt_led_task(&app_led);

    /* Non-blocking button debounce + click detection */
    kt_button_task(&app_btn);

    /* Non-blocking buzzer cycling / beep timing */
    kt_buzzer_task(&app_buzzer);
}
