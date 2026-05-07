#ifndef KT_CONFIG_H
#define KT_CONFIG_H

#include "stm32f1xx_hal.h"

/* Project Information */
#define KT_PROJECT_NAME        "Library Seat Management System"
#define KT_PROJECT_VERSION     "v0.8"
#define KT_FIRMWARE_ROLE       "Seat Node STM32"
#define KT_AUTHOR              "Kento"
#define KT_STUDENT_ID          "TODO_STUDENT_ID"

/* Debug UART Configuration */
#define KT_DEBUG_UART_BAUDRATE 115200
#define KT_DEBUG_UART           USART2

/* Protocol Frame Marker */
#define KT_PROTOCOL_FRAME_MARKER  0xFF

/* Boot Count - first version fixed to 1, Flash persistence to be added later */
#define KT_BOOT_COUNT             1

/* Log System Configuration (used by kt_log.h / kt_log.c) */
#define KT_LOG_ENABLE       1
#define KT_LOG_LEVEL        0   /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERR, 4=NONE */
#define KT_LOG_BUFFER_SIZE  128
#define KT_LOG_VERBOSE_ENABLE 0
#define KT_LOG_HEARTBEAT_ENABLE 0
#define KT_LOG_UART_FRAME_ENABLE 0
#define KT_UART_TX_TIMEOUT_MS 100U

/* Seat node hardware */
#define SEAT_NODE_ID                  1

/* Seat 1: HX711 pressure sensor + infrared sensor */
#define SEAT1_HX711_DT_PORT           GPIOA
#define SEAT1_HX711_DT_PIN            GPIO_PIN_4
#define SEAT1_HX711_SCK_PORT          GPIOA
#define SEAT1_HX711_SCK_PIN           GPIO_PIN_5
#define SEAT1_HX711_THRESHOLD         10000L
#define SEAT1_HX711_TARE_SAMPLES      3U
#define SEAT1_HX711_READY_TIMEOUT     2U  /* ms */
#define SEAT1_HX711_CAL_UNIT_GRAM     10U
#define SEAT1_HX711_FLASH_ADDR        0x0800FC00U
#define SEAT1_HX711_FLASH_MAGIC       0x4B544831U
#define SEAT1_HX711_DEFAULT_ENABLE    1U
#define SEAT1_HX711_DEFAULT_OFFSET    (-250747L)
#define SEAT1_HX711_DEFAULT_SCALE_X100 40667L
#define SEAT1_OCCUPIED_WEIGHT_GRAM    50L
#define SEAT1_IR_PORT                 GPIOB
#define SEAT1_IR_PIN                  GPIO_PIN_9
#define SEAT1_IR_ACTIVE_LEVEL         GPIO_PIN_SET

/* Seat 2/3: GPIO high level means pressure detected */
#define SEAT2_GPIO_PORT               GPIOB
#define SEAT2_GPIO_PIN                GPIO_PIN_8
#define SEAT3_GPIO_PORT               GPIOB
#define SEAT3_GPIO_PIN                GPIO_PIN_7
#define SEAT_GPIO_ACTIVE_LEVEL        GPIO_PIN_SET
#define SEAT_NODE_POLL_PERIOD_MS      200U
#define SEAT_NODE_STATE_CONFIRM_COUNT 3U
#define SEAT_NODE_KEEPALIVE_REPORT_MS 1000U
#define SEAT_NODE_ZIGBEE_RX_BYTES_PER_TASK 32U

#define SEAT_LED_ACTIVE_LEVEL         GPIO_PIN_SET
#define SEAT1_LED_RED_PORT            GPIOB
#define SEAT1_LED_RED_PIN             GPIO_PIN_5
#define SEAT1_LED_GREEN_PORT          GPIOB
#define SEAT1_LED_GREEN_PIN           GPIO_PIN_6
#define SEAT2_LED_RED_PORT            GPIOB
#define SEAT2_LED_RED_PIN             GPIO_PIN_3
#define SEAT2_LED_GREEN_PORT          GPIOB
#define SEAT2_LED_GREEN_PIN           GPIO_PIN_4
#define SEAT3_LED_RED_PORT            GPIOA
#define SEAT3_LED_RED_PIN             GPIO_PIN_12
#define SEAT3_LED_GREEN_PORT          GPIOA
#define SEAT3_LED_GREEN_PIN           GPIO_PIN_15
#define SEAT_IR_LED_PORT              GPIOB
#define SEAT_IR_LED_PIN               GPIO_PIN_14
#define SEAT_WEIGHT_LED_PORT          GPIOB
#define SEAT_WEIGHT_LED_PIN           GPIO_PIN_15

#define KT_ZIGBEE_UART_BAUDRATE       38400
#define ZIGBEE_ADDR_SELF              0x301EU
#define ZIGBEE_ADDR_MAIN_CTRL         0x0000U

#endif /* KT_CONFIG_H */
