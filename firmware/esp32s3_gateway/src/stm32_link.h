#ifndef STM32_LINK_H
#define STM32_LINK_H

#include "binary_protocol.h"
#include "wifi_manager.h"
#include <Arduino.h>

#define UART_VERBOSE_LOG 0

void stm32_link_begin();
void stm32_link_task();
void stm32_link_send_wifi_status(const WifiStatus &status);
bool stm32_link_online();

#endif
