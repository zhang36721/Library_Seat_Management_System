#ifndef KT_CONFIG_H
#define KT_CONFIG_H

/* Project Information */
#define KT_PROJECT_NAME        "Library Seat Management System"
#define KT_FIRMWARE_ROLE       "Main Controller STM32"
#define KT_AUTHOR              "Kento"
#define KT_STUDENT_ID          "TODO_STUDENT_ID"

/* Debug UART Configuration */
#define KT_DEBUG_UART_BAUDRATE 115200
#define KT_DEBUG_UART           USART2

/* Protocol Frame Marker */
#define KT_PROTOCOL_FRAME_MARKER  0xFF

/* Protocol RX Timeout - discard partial frame after this many ms */
#define KT_PROTOCOL_RX_TIMEOUT_MS 100

/* UART TX Timeout - max blocking time per transmit call */
#define KT_UART_TX_TIMEOUT_MS     100

/* Boot Count - first version fixed to 1, Flash persistence to be added later */
#define KT_BOOT_COUNT             1

/* Log System Configuration (used by kt_log.h / kt_log.c) */
#define KT_LOG_ENABLE       1
#define KT_LOG_LEVEL        0   /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERR, 4=NONE */
#define KT_LOG_BUFFER_SIZE  128

#endif /* KT_CONFIG_H */
