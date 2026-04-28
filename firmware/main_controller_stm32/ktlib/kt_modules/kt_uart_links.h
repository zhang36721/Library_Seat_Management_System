#ifndef KT_UART_LINKS_H
#define KT_UART_LINKS_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void kt_zigbee_send_test(void);
void kt_zigbee_send_ping(void);
void kt_zigbee_print_recent_rx(void);
void kt_zigbee_uart_start_receive_it(void);
void kt_zigbee_uart_rx_callback(UART_HandleTypeDef *huart);
void kt_zigbee_task(void);
void kt_esp32s3_send_test(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_UART_LINKS_H */
