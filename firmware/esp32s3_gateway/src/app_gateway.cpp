#include "app_gateway.h"
#include "stm32_link.h"
#include "wifi_manager.h"
#include <Arduino.h>

static bool wifi_status_sent = false;
static bool last_connected = false;

void app_gateway_begin()
{
    stm32_link_begin();
    wifi_manager_begin();
    WifiStatus status = wifi_manager_status();
    last_connected = status.connected;
    stm32_link_send_wifi_status(status);
    wifi_status_sent = true;
}

void app_gateway_task()
{
    stm32_link_task();
    wifi_manager_task();

    WifiStatus status = wifi_manager_status();
    if (!wifi_status_sent || status.connected != last_connected) {
        last_connected = status.connected;
        stm32_link_send_wifi_status(status);
        wifi_status_sent = true;
    }
}
