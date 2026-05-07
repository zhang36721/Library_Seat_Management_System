#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#include "wifi_manager.h"
#include <Arduino.h>

constexpr size_t DEVICE_CARD_EVENT_MAX = 50;

struct DeviceStatus {
    bool stm32_online = false;
    bool wifi_connected = false;
    String ssid;
    int rssi = 0;
    String ip;
    uint8_t card_count = 0;
    uint8_t log_count = 0;
    bool ds1302_valid = false;
    String current_time = "---- -- -- --:--:--";
    String seats[3] = {"UNKNOWN", "UNKNOWN", "UNKNOWN"};
    String gate_state = "UNKNOWN";
    uint8_t last_card_result = 0;
    uint32_t heartbeat_count = 0;
    uint32_t last_heartbeat_ms = 0;
    uint32_t last_card_event_ms = 0;
};

struct CardEvent {
    String uid;
    String type;
    bool allowed = false;
    String time;
    uint32_t received_ms = 0;
};

void device_state_begin();
void device_state_set_wifi(const WifiStatus &status);
void device_state_set_stm32_online(bool online);
void device_state_note_heartbeat();
void device_state_update_device_status(const uint8_t *payload, uint16_t len);
void device_state_add_card_event(const uint8_t *payload, uint16_t len);
DeviceStatus device_state_get_status();
size_t device_state_card_event_count();
CardEvent device_state_get_card_event(size_t index);
bool device_state_get_latest_card_event(CardEvent &event);
void device_state_clear_card_events();

#endif
