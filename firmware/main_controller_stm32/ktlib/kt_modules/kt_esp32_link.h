#ifndef KT_ESP32_LINK_H
#define KT_ESP32_LINK_H

#include "kt_modules/kt_ds1302.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void kt_esp32_link_init(void);
void kt_esp32_link_task(void);
void kt_esp32_link_uart_rx_callback(UART_HandleTypeDef *huart);

void kt_esp32_link_send_ping(void);
void kt_esp32_link_send_bad_crc_test(void);
void kt_esp32_link_send_mock_card_event(void);
void kt_esp32_link_send_card_event(const uint8_t uid[4],
                                   uint8_t access_type,
                                   uint8_t allowed,
                                   const kt_ds1302_time_t *time);
void kt_esp32_link_print_recent_rx(void);
void kt_esp32_link_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_ESP32_LINK_H */
