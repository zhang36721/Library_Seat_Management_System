#ifndef KT_CONFIG_H
#define KT_CONFIG_H

#include "stm32f1xx_hal.h"

/*===========================================================================
 * Project Information
 *===========================================================================*/
#define KT_PROJECT_NAME        "Library Seat Management System"
#define KT_PROJECT_VERSION     "v0.8.2"
#define KT_FW_NAME             "Main Controller STM32"
#define KT_FIRMWARE_ROLE       KT_FW_NAME
#define KT_AUTHOR              "Kento"
#define KT_STUDENT_ID          "TODO_STUDENT_ID"

/*===========================================================================
 * Debug UART Configuration
 *===========================================================================*/
#define KT_DEBUG_UART_BAUDRATE 115200
#define KT_DEBUG_UART           USART2

#define KT_USART1_ROLE          "ZigBee test UART, 115200"
#define KT_USART2_ROLE          "Debug UART, 115200, FF CMD DATA FF protocol"
#define KT_USART3_ROLE          "ESP32S3 test UART, 115200"

/*===========================================================================
 * Protocol Frame Marker
 *===========================================================================*/
#define KT_PROTOCOL_FRAME_MARKER  0xFF

/* Protocol RX Timeout - discard partial frame after this many ms */
#define KT_PROTOCOL_RX_TIMEOUT_MS 100

/* UART TX Timeout - max blocking time per transmit call */
#define KT_UART_TX_TIMEOUT_MS     100

/*===========================================================================
 * Persistent Boot Count
 *
 * STM32F103C8T6 has 64KB Flash from 0x08000000 to 0x0800FFFF.
 * The last 1KB page starts at 0x0800FC00 and is reserved for boot count.
 *===========================================================================*/
#define KT_BOOT_COUNT_FLASH_PAGE_ADDR  0x0800FC00U
#define KT_BOOT_COUNT_MAGIC            0x4B544243U  /* "KTBC" */

/*===========================================================================
 * Log System Configuration (used by kt_log.h / kt_log.c)
 *===========================================================================*/
#define KT_LOG_ENABLE       1
#define KT_LOG_LEVEL        0   /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERR, 4=NONE */
#define KT_LOG_BUFFER_SIZE  128

/* Periodic uptime logs are disabled for concise v0.6.1 hardware validation. */
#define KT_ENABLE_UPTIME_LOG 0

/*===========================================================================
 * Application I/O Defaults (overridable in app_io.h)
 *===========================================================================*/
#define APP_LED_DEFAULT_PORT         GPIOC
#define APP_LED_DEFAULT_PIN          GPIO_PIN_13
#define APP_LED_DEFAULT_ACTIVE_LEVEL GPIO_PIN_RESET  /* PC13 active-low */

#define APP_BTN_DEFAULT_PORT         GPIOA
#define APP_BTN_DEFAULT_PIN          GPIO_PIN_0
#define APP_BTN_DEFAULT_ACTIVE_LEVEL GPIO_PIN_RESET
#define APP_BTN_DEFAULT_DEBOUNCE_MS  20

#define APP_BUZZER_DEFAULT_PORT         GPIOA
#define APP_BUZZER_DEFAULT_PIN          GPIO_PIN_4
#define APP_BUZZER_DEFAULT_ACTIVE_LEVEL GPIO_PIN_SET    /* Verify with FF 22 */

/* Keep PC13 available for manual USART2 debug tests in v0.6. */
#define APP_HEARTBEAT_LED_ENABLE     0

/*===========================================================================
 * v0.6 Seat Sensor Input Placeholders
 *
 * Real wiring is not confirmed yet. Keep these macros explicit, but do not
 * initialize/read them until APP_SEAT_SENSOR_PINS_CONFIRMED is set to 1 after
 * hardware confirmation.
 *===========================================================================*/
#define APP_SEAT_SENSOR_PINS_CONFIRMED 0

#define APP_SEAT1_SENSOR_PORT        ((GPIO_TypeDef *)0)
#define APP_SEAT1_SENSOR_PIN         GPIO_PIN_0
#define APP_SEAT2_SENSOR_PORT        ((GPIO_TypeDef *)0)
#define APP_SEAT2_SENSOR_PIN         GPIO_PIN_0
#define APP_SEAT3_SENSOR_PORT        ((GPIO_TypeDef *)0)
#define APP_SEAT3_SENSOR_PIN         GPIO_PIN_0

#define APP_SEAT_SENSOR_ACTIVE_LEVEL GPIO_PIN_SET

/*===========================================================================
 * v0.7 Main Controller Peripheral Test Pins
 *
 * These are the current wiring-plan pins for independent module validation.
 * Update the macros after real wiring is confirmed; USART2 debug remains
 * independent and must not be shared by ZigBee or ESP32S3.
 *===========================================================================*/
#define KT_RC522_SCK_PORT       GPIOA
#define KT_RC522_SCK_PIN        GPIO_PIN_5
#define KT_RC522_MISO_PORT      GPIOA
#define KT_RC522_MISO_PIN       GPIO_PIN_6
#define KT_RC522_MOSI_PORT      GPIOA
#define KT_RC522_MOSI_PIN       GPIO_PIN_7
#define KT_RC522_NSS_PORT       GPIOB
#define KT_RC522_NSS_PIN        GPIO_PIN_12
#define KT_RC522_RST_PORT       GPIOB
#define KT_RC522_RST_PIN        GPIO_PIN_13

#define KT_OLED_SCL_PORT        GPIOB
#define KT_OLED_SCL_PIN         GPIO_PIN_6
#define KT_OLED_SDA_PORT        GPIOB
#define KT_OLED_SDA_PIN         GPIO_PIN_7
#define KT_OLED_I2C_ADDR        0x78U

#define KT_DS1302_CLK_PORT      GPIOB
#define KT_DS1302_CLK_PIN       GPIO_PIN_0
#define KT_DS1302_DAT_PORT      GPIOB
#define KT_DS1302_DAT_PIN       GPIO_PIN_1
#define KT_DS1302_RST_PORT      GPIOB
#define KT_DS1302_RST_PIN       GPIO_PIN_5

#define KT_STEPPER_IN1_PORT     GPIOB
#define KT_STEPPER_IN1_PIN      GPIO_PIN_8
#define KT_STEPPER_IN2_PORT     GPIOB
#define KT_STEPPER_IN2_PIN      GPIO_PIN_9
#define KT_STEPPER_IN3_PORT     GPIOB
#define KT_STEPPER_IN3_PIN      GPIO_PIN_14
#define KT_STEPPER_IN4_PORT     GPIOB
#define KT_STEPPER_IN4_PIN      GPIO_PIN_15
#define KT_STEPPER_STEP_DELAY_MS 3U

#define KT_ZIGBEE_TEST_BAUDRATE 115200
#define KT_ESP32S3_TEST_BAUDRATE 115200

#endif /* KT_CONFIG_H */
