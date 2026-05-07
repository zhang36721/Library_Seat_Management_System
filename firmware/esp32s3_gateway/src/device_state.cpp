#include "device_state.h"

static DeviceStatus status_cache;
static CardEvent event_ring[DEVICE_CARD_EVENT_MAX];
static size_t event_start = 0;
static size_t event_count = 0;

static String format_time(const uint8_t *p)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "20%02u-%02u-%02u %02u:%02u:%02u",
             static_cast<unsigned int>(p[0]),
             static_cast<unsigned int>(p[1]),
             static_cast<unsigned int>(p[2]),
             static_cast<unsigned int>(p[3]),
             static_cast<unsigned int>(p[4]),
             static_cast<unsigned int>(p[5]));
    return String(buf);
}

static String uid_to_string(const uint8_t *uid)
{
    char buf[9];
    snprintf(buf, sizeof(buf), "%02X%02X%02X%02X",
             static_cast<unsigned int>(uid[0]),
             static_cast<unsigned int>(uid[1]),
             static_cast<unsigned int>(uid[2]),
             static_cast<unsigned int>(uid[3]));
    return String(buf);
}

static String access_type_text(uint8_t type)
{
    if (type == 0) return "CHECK_IN";
    if (type == 1) return "CHECK_OUT";
    return "DENIED";
}

static String seat_state_text(uint8_t state)
{
    if (state == 0) return "FREE";
    if (state == 1) return "OCCUPIED";
    return "UNKNOWN";
}

static String gate_state_text(uint8_t state)
{
    if (state == 0) return "CLOSED";
    if (state == 1) return "OPENING";
    if (state == 2) return "OPEN";
    if (state == 3) return "CLOSING";
    if (state == 4) return "ERROR";
    return "UNKNOWN";
}

void device_state_begin()
{
    status_cache = DeviceStatus();
    event_start = 0;
    event_count = 0;
}

void device_state_set_wifi(const WifiStatus &status)
{
    status_cache.wifi_connected = status.connected;
    status_cache.ssid = status.ssid;
    status_cache.rssi = status.rssi;
    char ip[16];
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
             static_cast<unsigned int>(status.ip[0]),
             static_cast<unsigned int>(status.ip[1]),
             static_cast<unsigned int>(status.ip[2]),
             static_cast<unsigned int>(status.ip[3]));
    status_cache.ip = status.connected ? String(ip) : String("0.0.0.0");
}

void device_state_set_stm32_online(bool online)
{
    status_cache.stm32_online = online;
}

void device_state_note_heartbeat()
{
    status_cache.heartbeat_count++;
    status_cache.last_heartbeat_ms = millis();
}

void device_state_update_device_status(const uint8_t *payload, uint16_t len)
{
    if (len < 15 || payload == nullptr) {
        return;
    }
    status_cache.card_count = payload[4];
    status_cache.log_count = payload[5];
    status_cache.ds1302_valid = payload[8] ? true : false;
    status_cache.current_time = format_time(&payload[9]);
    if (len >= 20) {
        status_cache.seats[0] = seat_state_text(payload[15]);
        status_cache.seats[1] = seat_state_text(payload[16]);
        status_cache.seats[2] = seat_state_text(payload[17]);
        status_cache.gate_state = gate_state_text(payload[18]);
        status_cache.last_card_result = payload[19];
    }
}

void device_state_add_card_event(const uint8_t *payload, uint16_t len)
{
    if (len < 12 || payload == nullptr) {
        return;
    }

    CardEvent event;
    event.uid = uid_to_string(payload);
    event.type = access_type_text(payload[4]);
    event.allowed = payload[5] ? true : false;
    event.time = format_time(&payload[6]);
    event.received_ms = millis();

    size_t index = 0;
    if (event_count < DEVICE_CARD_EVENT_MAX) {
        index = (event_start + event_count) % DEVICE_CARD_EVENT_MAX;
        event_count++;
    } else {
        index = event_start;
        event_start = (event_start + 1) % DEVICE_CARD_EVENT_MAX;
    }
    event_ring[index] = event;
    status_cache.last_card_event_ms = event.received_ms;
}

DeviceStatus device_state_get_status()
{
    return status_cache;
}

size_t device_state_card_event_count()
{
    return event_count;
}

CardEvent device_state_get_card_event(size_t index)
{
    if (index >= event_count) {
        return CardEvent();
    }
    size_t ring_index = (event_start + index) % DEVICE_CARD_EVENT_MAX;
    return event_ring[ring_index];
}

bool device_state_get_latest_card_event(CardEvent &event)
{
    if (event_count == 0) {
        return false;
    }
    size_t ring_index = (event_start + event_count - 1) % DEVICE_CARD_EVENT_MAX;
    event = event_ring[ring_index];
    return true;
}

void device_state_clear_card_events()
{
    event_start = 0;
    event_count = 0;
    status_cache.last_card_event_ms = 0;
}
