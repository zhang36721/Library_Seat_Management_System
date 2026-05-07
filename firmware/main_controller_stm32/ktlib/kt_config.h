#ifndef KT_CONFIG_H
#define KT_CONFIG_H

#include "stm32f1xx_hal.h"

/*===========================================================================
 * Project Information
 *===========================================================================*/
#define KT_PROJECT_NAME        "Library Seat Management System"
#define KT_PROJECT_VERSION     "v0.8.5"
#define KT_FW_NAME             "Main Controller STM32"
#define KT_FIRMWARE_ROLE       KT_FW_NAME
#define KT_AUTHOR              "Kento"
#define KT_STUDENT_ID          "191110245"

/*===========================================================================
 * Debug UART Configuration
 *===========================================================================*/
#define KT_DEBUG_UART_BAUDRATE 115200
#define KT_DEBUG_UART           USART2

#define KT_USART1_ROLE          "ZigBee CC2530 point-to-point UART, 38400"
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

#define ZIGBEE_ADDR_SELF       0x0000U
#define ZIGBEE_ADDR_SEAT_NODE  0x301EU

/*===========================================================================
 * Persistent Boot Count
 *
 * STM32F103C8T6 has 64KB Flash from 0x08000000 to 0x0800FFFF.
 * The last 1KB page starts at 0x0800FC00 and is reserved for boot count.
 *===========================================================================*/
#define KT_BOOT_COUNT_FLASH_PAGE_ADDR  0x0800FC00U
#define KT_BOOT_COUNT_MAGIC            0x4B544243U  /* "KTBC" */

/* The second last 1KB page is reserved for registered card database.
 * Keep the Keil IROM size at or below 0xF800 so firmware code does not
 * overlap 0x0800F800-0x0800FFFF.
 */
#define MAIN_CARD_DB_FLASH_PAGE_ADDR    0x0800F800U
#define MAIN_CARD_DB_FLASH_MAGIC        0x4B544344U  /* "KTCD" */

/* v1.3 stability policy:
 * Card DB is configuration data and is allowed to persist in Flash.
 * Access logs are business data and are RAM only; cloud/backend is the
 * long-term record source.
 */
#define MAIN_CARD_DB_FLASH_ENABLE       1U
#define MAIN_ACCESS_LOG_FLASH_ENABLE    0U

/*===========================================================================
 * Log System Configuration (used by kt_log.h / kt_log.c)
 *===========================================================================*/
#define KT_LOG_ENABLE       1
#define KT_LOG_LEVEL        1   /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERR, 4=NONE */
#define KT_LOG_BUFFER_SIZE  128
#define KT_LOG_VERBOSE_ENABLE 0
#define KT_LOG_HEARTBEAT_ENABLE 0
#define KT_LOG_UART_FRAME_ENABLE 0

/*===========================================================================
 * System watchdog
 *===========================================================================*/
#define KT_WATCHDOG_ENABLE              1U
#define KT_WATCHDOG_PR_VALUE            4U      /* IWDG prescaler /64 */
#define KT_WATCHDOG_RELOAD_VALUE        4095U   /* about 6.5s at 40kHz LSI */
#define KT_WATCHDOG_TASK_TIMEOUT_MS     5000U

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
#define KT_STEPPER_STEP_DELAY_MS 8U
#define KT_STEPPER_GATE_STEPS    200U
#define KT_STEPPER_WAVE_DRIVE_ENABLE 1U
/*
 * Keep automatic access flow stable first. Manual FF 70/71/72 stepper tests
 * still drive the motor; the card access gate cycle only reports/simulates the
 * gate state when this is 0.
 */
#define MAIN_GATE_STEPPER_ENABLE 1U
#define MAIN_GATE_MOTOR_RUN_MS   800U

#define KT_ZIGBEE_TEST_BAUDRATE 38400
#define KT_ESP32S3_TEST_BAUDRATE 115200

/*===========================================================================
 * v0.9 STM32 <-> ESP32S3 Binary UART Protocol
 *===========================================================================*/
#define KT_BIN_PROTOCOL_VERSION       0x01U
#define KT_BIN_MAX_PAYLOAD_LEN        128U

#define KT_MSG_PING                   0x01U
#define KT_MSG_PONG                   0x02U
#define KT_MSG_ACK                    0x03U
#define KT_MSG_ERR                    0x04U
#define KT_MSG_HEARTBEAT              0x05U
#define KT_MSG_HEARTBEAT_ACK          0x06U
#define KT_MSG_WIFI_STATUS            0x10U
#define KT_MSG_CARD_EVENT             0x20U
#define KT_MSG_ACCESS_LOG             0x21U
#define KT_MSG_DEVICE_STATUS          0x30U
#define KT_MSG_BOOT_SYNC              0x31U
#define KT_MSG_SYNC_ACK               0x32U

#define KT_ESP32_HEARTBEAT_TIMEOUT_MS 10000U
#define KT_ESP32_LINK_LED_PULSE_MS    50U
#define KT_ESP32_DEVICE_STATUS_MIN_MS 1000U
#define KT_ESP32_TX_GAP_MS            5U
#define KT_ESP32_TX_TIMEOUT_MS        30U
#define KT_ESP32_RX_BYTES_PER_TASK    128U
#define KT_ESP32_TX_QUEUE_LEN         12U
#define KT_ESP32_ASCII_PUSH_ENABLE    1U
#define KT_ESP32_BINARY_PUSH_ENABLE   0U

#define KT_ZIGBEE_RX_BYTES_PER_TASK   32U

#define APP_CARD_PROFILE_ENABLE       0U

/*===========================================================================
 * v0.8.3 Main Controller 8-key Module
 *===========================================================================*/
#define MAIN_KEY_ACTIVE_LEVEL      GPIO_PIN_RESET
#define MAIN_KEY_DEBOUNCE_MS       10U
#define MAIN_KEY_LONG_PRESS_MS     800U

#define MAIN_KEY1_PORT             GPIOA
#define MAIN_KEY1_PIN              GPIO_PIN_1
#define MAIN_KEY2_PORT             GPIOA
#define MAIN_KEY2_PIN              GPIO_PIN_8
#define MAIN_KEY3_PORT             GPIOA
#define MAIN_KEY3_PIN              GPIO_PIN_11
#define MAIN_KEY4_PORT             GPIOA
#define MAIN_KEY4_PIN              GPIO_PIN_12
#define MAIN_KEY5_PORT             GPIOA
#define MAIN_KEY5_PIN              GPIO_PIN_15
#define MAIN_KEY6_PORT             GPIOB
#define MAIN_KEY6_PIN              GPIO_PIN_3
#define MAIN_KEY7_PORT             GPIOB
#define MAIN_KEY7_PIN              GPIO_PIN_4
#define MAIN_KEY8_PORT             GPIOC
#define MAIN_KEY8_PIN              GPIO_PIN_14

/*===========================================================================
 * v0.8.5 Main Controller Local Access Flow
 *===========================================================================*/
#define MAIN_APP_TASK_PERIOD_MS       10U
#define MAIN_CARD_POLL_PERIOD_MS      100U
#define MAIN_CARD_REPEAT_GUARD_MS     2500U
#define MAIN_GATE_HOLD_MS             3000U
#define MAIN_OLED_IDLE_HOME_MS        3000U
#define MAIN_ACCESS_LOG_MAX_COUNT     50U

#endif /* KT_CONFIG_H */
