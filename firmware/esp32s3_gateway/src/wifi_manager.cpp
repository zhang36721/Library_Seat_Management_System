#include "wifi_manager.h"
#include <WiFi.h>

constexpr WifiProfile WIFI_PROFILES[] = {
    {"B4-3408_2.4G", "Aa12345678"},
    {"310", "88888888"},
    {"301", "88888888"},
};

static WifiStatus current_status;

static void update_status()
{
    current_status.connected = (WiFi.status() == WL_CONNECTED);
    if (current_status.connected) {
        current_status.ssid = WiFi.SSID();
        current_status.rssi = static_cast<int8_t>(WiFi.RSSI());
        IPAddress ip = WiFi.localIP();
        current_status.ip[0] = ip[0];
        current_status.ip[1] = ip[1];
        current_status.ip[2] = ip[2];
        current_status.ip[3] = ip[3];
    } else {
        current_status.ssid = "";
        current_status.rssi = 0;
        memset(current_status.ip, 0, sizeof(current_status.ip));
    }
}

void wifi_manager_begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(100);

    for (const auto &profile : WIFI_PROFILES) {
        Serial.printf("[WiFi] connecting ssid=%s\n", profile.ssid);
        WiFi.begin(profile.ssid, profile.password);
        uint32_t start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
            delay(200);
            Serial.print(".");
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED) {
            update_status();
            Serial.printf("[WiFi] connected ssid=%s ip=%u.%u.%u.%u rssi=%d\n",
                          current_status.ssid.c_str(),
                          current_status.ip[0], current_status.ip[1],
                          current_status.ip[2], current_status.ip[3],
                          current_status.rssi);
            return;
        }
        WiFi.disconnect(true, true);
        delay(100);
    }

    update_status();
    Serial.println("[WiFi] offline mode");
}

void wifi_manager_task()
{
    static wl_status_t last_status = WL_IDLE_STATUS;
    wl_status_t now = WiFi.status();
    if (now != last_status) {
        last_status = now;
        update_status();
    }
}

WifiStatus wifi_manager_status()
{
    update_status();
    return current_status;
}
