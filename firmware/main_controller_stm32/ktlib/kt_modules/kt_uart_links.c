#include "kt_uart_links.h"
#include "kt_config.h"
#include "kt_log.h"
#include "usart.h"
#include <string.h>

#define ZIGBEE_RX_BUF_SIZE 96

static volatile uint8_t zigbee_rx_byte;
static volatile uint8_t zigbee_rx_len = 0;
static char zigbee_rx_line[ZIGBEE_RX_BUF_SIZE];
static char zigbee_recent_line[ZIGBEE_RX_BUF_SIZE];
static volatile uint8_t zigbee_line_ready = 0;

static void zigbee_send_text(const char *msg)
{
    char log_msg[ZIGBEE_RX_BUF_SIZE];
    size_t len = strlen(msg);
    HAL_StatusTypeDef st = HAL_UART_Transmit(&huart1, (uint8_t *)msg,
                                             (uint16_t)len,
                                             KT_UART_TX_TIMEOUT_MS);
    if (len >= sizeof(log_msg)) {
        len = sizeof(log_msg) - 1U;
    }
    memcpy(log_msg, msg, len);
    log_msg[len] = '\0';
    while (len > 0U && (log_msg[len - 1U] == '\r' || log_msg[len - 1U] == '\n')) {
        log_msg[--len] = '\0';
    }
    KT_LOG_INFO("ZigBee TX: %s", log_msg);
    KT_LOG_INFO("USART1 ZigBee test TX: %s", (st == HAL_OK) ? "OK" : "FAIL");
}

void kt_zigbee_uart_start_receive_it(void)
{
    zigbee_rx_len = 0;
    zigbee_line_ready = 0;
    zigbee_recent_line[0] = '\0';
    (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zigbee_rx_byte, 1);
}

void kt_zigbee_uart_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uint8_t b = zigbee_rx_byte;

        if (b == '\r' || b == '\n') {
            if (zigbee_rx_len > 0U) {
                zigbee_rx_line[zigbee_rx_len] = '\0';
                memcpy(zigbee_recent_line, zigbee_rx_line, zigbee_rx_len + 1U);
                zigbee_line_ready = 1U;
                zigbee_rx_len = 0U;
            }
        } else if (zigbee_rx_len < (ZIGBEE_RX_BUF_SIZE - 1U)) {
            zigbee_rx_line[zigbee_rx_len++] = (char)b;
        } else {
            zigbee_rx_len = 0U;
        }

        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zigbee_rx_byte, 1);
    }
}

void kt_zigbee_task(void)
{
    if (zigbee_line_ready) {
        zigbee_line_ready = 0U;
        KT_LOG_INFO("ZigBee RX: %s", zigbee_recent_line);
    }
}

void kt_zigbee_send_test(void)
{
    zigbee_send_text("STM32_MAIN_ZIGBEE_TEST\r\n");
}

void kt_zigbee_send_ping(void)
{
    zigbee_send_text("MAIN,PING,1\r\n");
}

void kt_zigbee_print_recent_rx(void)
{
    if (zigbee_recent_line[0] == '\0') {
        KT_LOG_WARN("ZigBee RX buffer empty");
    } else {
        KT_LOG_INFO("ZigBee recent RX: %s", zigbee_recent_line);
    }
}

void kt_esp32s3_send_test(void)
{
    const char *msg = "STM32_MAIN_ESP32S3_TEST\r\n";
    HAL_StatusTypeDef st = HAL_UART_Transmit(&huart3, (uint8_t *)msg,
                                             (uint16_t)strlen(msg),
                                             KT_UART_TX_TIMEOUT_MS);
    KT_LOG_INFO("USART3 ESP32S3 test TX: %s", (st == HAL_OK) ? "OK" : "FAIL");
}
