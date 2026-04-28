#ifndef KT_PORT_GPIO_H
#define KT_PORT_GPIO_H

#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Turn on PC13 LED (active low: GPIO_PIN_RESET = ON)
 */
void kt_port_led_on(void);

/**
 * @brief Turn off PC13 LED (GPIO_PIN_SET = OFF)
 */
void kt_port_led_off(void);

/**
 * @brief Toggle PC13 LED
 */
void kt_port_led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_PORT_GPIO_H */