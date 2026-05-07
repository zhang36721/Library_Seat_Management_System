#include "app_gateway.h"
#include "cloud_client.h"
#include "device_state.h"
#include "http_server.h"
#include "stm32_link.h"
#include "wifi_manager.h"
#include <Arduino.h>

static bool wifi_status_sent = false;
static bool last_connected = false;

void app_gateway_begin()
{
    device_state_begin();
    stm32_link_begin();
    wifi_manager_begin();
    WifiStatus status = wifi_manager_status();
    device_state_set_wifi(status);
    last_connected = status.connected;
    stm32_link_send_wifi_status(status);
    wifi_status_sent = true;
    cloud_client_begin();
    http_server_begin();
}

void app_gateway_task()
{
    stm32_link_task();
    wifi_manager_task();
    http_server_task();
    stm32_link_task();
    cloud_client_task();
    stm32_link_task();

    WifiStatus status = wifi_manager_status();
    device_state_set_wifi(status);
    if (!wifi_status_sent || status.connected != last_connected) {
        last_connected = status.connected;
        stm32_link_send_wifi_status(status);
        wifi_status_sent = true;
    }
}
