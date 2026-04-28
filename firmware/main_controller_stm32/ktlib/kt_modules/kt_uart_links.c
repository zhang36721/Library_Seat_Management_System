#include "kt_uart_links.h"
#include "kt_config.h"
#include "kt_log.h"
#include "usart.h"
#include <string.h>

void kt_zigbee_send_test(void)
{
    const char *msg = "STM32_MAIN_ZIGBEE_TEST\r\n";
    HAL_StatusTypeDef st = HAL_UART_Transmit(&huart1, (uint8_t *)msg,
                                             (uint16_t)strlen(msg),
                                             KT_UART_TX_TIMEOUT_MS);
    KT_LOG_INFO("USART1 ZigBee test TX: %s", (st == HAL_OK) ? "OK" : "FAIL");
}

void kt_esp32s3_send_test(void)
{
    const char *msg = "STM32_MAIN_ESP32S3_TEST\r\n";
    HAL_StatusTypeDef st = HAL_UART_Transmit(&huart3, (uint8_t *)msg,
                                             (uint16_t)strlen(msg),
                                             KT_UART_TX_TIMEOUT_MS);
    KT_LOG_INFO("USART3 ESP32S3 test TX: %s", (st == HAL_OK) ? "OK" : "FAIL");
}
