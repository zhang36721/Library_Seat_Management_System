#include "kt_port_gpio.h"

/**
 * @brief Turn on PC13 LED
 *        PC13 is active low: GPIO_PIN_RESET = LED ON
 */
void kt_port_led_on(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

/**
 * @brief Turn off PC13 LED
 *        PC13 is active low: GPIO_PIN_SET = LED OFF
 */
void kt_port_led_off(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief Toggle PC13 LED
 */
void kt_port_led_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}