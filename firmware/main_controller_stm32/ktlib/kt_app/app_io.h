#ifndef APP_IO_H
#define APP_IO_H

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "kt_config.h"
#include "kt_devices/kt_led.h"
#include "kt_devices/kt_button.h"
#include "kt_devices/kt_buzzer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Application device instances — defined in app_io.c
 *
 * These are the "singleton" instances that represent physical hardware
 * on this board. kt_cmd.c and other app-level code use these instances
 * to call the multi-instance device APIs.
 *===========================================================================*/

/** System LED (PC13, active-low) */
extern kt_led_t    app_led;

/** User button (PA0, active-low, 20ms debounce) */
extern kt_button_t app_btn;

/** Buzzer (PA4, active level must be verified with FF 22) */
extern kt_buzzer_t app_buzzer;

/*===========================================================================
 * Initialization & Tasks
 *===========================================================================*/

/**
 * @brief Initialize all application I/O (LED, button, buzzer)
 *
 * Call once after HAL_Init() and GPIO clock enable.
 */
void app_io_init(void);

/**
 * @brief Run all non-blocking device tasks (button debounce, LED blink, buzzer cycle)
 *
 * Call periodically from main loop or scheduler (e.g. every 10-20 ms).
 */
void app_io_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_IO_H */
