#ifndef SEAT_NODE_APP_H
#define SEAT_NODE_APP_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void seat_node_app_init(void);
void seat_node_app_task(void);
void seat_node_print_hardware_status(void);
void seat_node_print_uart_roles(void);
void seat_node_print_raw_levels(void);
void seat_node_print_status(void);
void seat_node_send_zigbee_status(void);
void seat_node_send_zigbee_pong(void);
void seat_node_zigbee_rx_callback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* SEAT_NODE_APP_H */
