#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

struct WifiProfile {
    const char *ssid;
    const char *password;
};

struct WifiStatus {
    bool connected = false;
    int8_t rssi = 0;
    String ssid;
    uint8_t ip[4]{};
};

void wifi_manager_begin();
void wifi_manager_task();
WifiStatus wifi_manager_status();

#endif
