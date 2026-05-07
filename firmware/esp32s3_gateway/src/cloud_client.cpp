#include "cloud_client.h"
#include "wifi_manager.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static uint32_t last_heartbeat_post_ms = 0;
static uint32_t last_success_upload_ms = 0;
static uint32_t failed_upload_count = 0;
static bool device_status_pending = false;
static uint32_t last_device_status_try_ms = 0;
static uint32_t last_card_event_try_ms = 0;

constexpr size_t CLOUD_CARD_EVENT_QUEUE_MAX = 8;
static CardEvent card_event_queue[CLOUD_CARD_EVENT_QUEUE_MAX];
static size_t card_event_head = 0;
static size_t card_event_tail = 0;
static size_t card_event_count = 0;
static uint32_t card_event_drop_count = 0;
static WiFiClient plain_client;
static WiFiClientSecure secure_client;

static const char *server_base_url()
{
    if (CLOUD_USE_PROD_SERVER) {
        return SERVER_BASE_URL_PROD;
    }

    WifiStatus status = wifi_manager_status();
    if (status.ssid == "301") {
        return SERVER_BASE_URL_301;
    }
    if (status.ssid == "310") {
        return SERVER_BASE_URL_310;
    }
    if (status.ssid == "B4-3408_2.4G") {
        return SERVER_BASE_URL_3408;
    }
    return SERVER_BASE_URL_3408;
}

static String json_escape(const String &value)
{
    String out;
    out.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); ++i) {
        char c = value[i];
        if (c == '\\' || c == '"') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else {
            out += c;
        }
    }
    return out;
}

static bool post_json(const char *name, const char *path, const String &body, const String &extra = "")
{
    if (WiFi.status() != WL_CONNECTED) {
        failed_upload_count++;
        Serial.printf("[CLOUD] %s POST FAIL code=-1 reason=wifi_offline failed=%lu\n",
                      name, static_cast<unsigned long>(failed_upload_count));
        return false;
    }

    HTTPClient http;
    String url = String(server_base_url()) + path;
    http.setTimeout(CLOUD_HTTP_TIMEOUT_MS);
    bool begin_ok;
    if (url.startsWith("https://")) {
        secure_client.setInsecure();
        begin_ok = http.begin(secure_client, url);
    } else {
        begin_ok = http.begin(plain_client, url);
    }

    if (!begin_ok) {
        failed_upload_count++;
        Serial.printf("[CLOUD] %s POST FAIL code=-1 reason=begin_failed failed=%lu\n",
                      name, static_cast<unsigned long>(failed_upload_count));
        return false;
    }
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);
    http.end();

    if (code >= 200 && code < 300) {
        last_success_upload_ms = millis();
        if (String(name) == "card-event") {
            Serial.printf("[CLOUD] %s POST OK code=%d %s last_upload_ms=%lu\n",
                          name, code, extra.c_str(), static_cast<unsigned long>(last_success_upload_ms));
        } else if (CLOUD_VERBOSE_LOG) {
            Serial.printf("[CLOUD] %s POST OK code=%d last_upload_ms=%lu\n",
                          name, code, static_cast<unsigned long>(last_success_upload_ms));
        }
        return true;
    }

    failed_upload_count++;
    String reason = HTTPClient::errorToString(code);
    Serial.printf("[CLOUD] %s POST FAIL code=%d reason=%s failed=%lu\n",
                  name, code, reason.c_str(), static_cast<unsigned long>(failed_upload_count));
    return false;
}

static String status_bool(bool value)
{
    return value ? "true" : "false";
}

static void post_heartbeat()
{
    DeviceStatus status = device_state_get_status();
    String body = "{";
    body += "\"device_id\":\"";
    body += DEVICE_ID;
    body += "\",\"esp_uptime_ms\":";
    body += String(millis());
    body += ",\"stm32_online\":";
    body += status_bool(status.stm32_online);
    body += ",\"wifi_connected\":";
    body += status_bool(status.wifi_connected);
    body += ",\"ssid\":\"";
    body += json_escape(status.ssid);
    body += "\",\"rssi\":";
    body += String(status.rssi);
    body += "}";
    post_json("heartbeat", "/api/iot/heartbeat", body);
}

void cloud_client_begin()
{
    last_heartbeat_post_ms = 0;
    last_success_upload_ms = 0;
    failed_upload_count = 0;
    device_status_pending = false;
    last_device_status_try_ms = 0;
    last_card_event_try_ms = 0;
    card_event_head = 0;
    card_event_tail = 0;
    card_event_count = 0;
    card_event_drop_count = 0;
    Serial.printf("[CLOUD] server=%s device=%s\n", server_base_url(), DEVICE_ID);
}

void cloud_client_task()
{
    uint32_t now = millis();
    if (last_heartbeat_post_ms == 0 || now - last_heartbeat_post_ms >= CLOUD_HEARTBEAT_PERIOD_MS) {
        last_heartbeat_post_ms = now;
        post_heartbeat();
        return;
    }

    if (card_event_count > 0 && (last_card_event_try_ms == 0 ||
                                 now - last_card_event_try_ms >= CLOUD_PENDING_RETRY_MS)) {
        last_card_event_try_ms = now;
        if (cloud_client_upload_card_event(card_event_queue[card_event_tail])) {
            card_event_tail = (card_event_tail + 1U) % CLOUD_CARD_EVENT_QUEUE_MAX;
            card_event_count--;
        }
        return;
    }

    if (device_status_pending && (last_device_status_try_ms == 0 ||
                                  now - last_device_status_try_ms >= CLOUD_PENDING_RETRY_MS)) {
        last_device_status_try_ms = now;
        if (cloud_client_upload_device_status()) {
            device_status_pending = false;
        }
    }
}

bool cloud_client_upload_device_status()
{
    DeviceStatus status = device_state_get_status();
    String body = "{";
    body += "\"device_id\":\"";
    body += DEVICE_ID;
    body += "\",\"stm32_online\":";
    body += status_bool(status.stm32_online);
    body += ",\"card_count\":";
    body += String(status.card_count);
    body += ",\"log_count\":";
    body += String(status.log_count);
    body += ",\"ds1302_valid\":";
    body += status_bool(status.ds1302_valid);
    body += ",\"current_time\":\"";
    body += json_escape(status.current_time);
    body += "\",\"seats\":[";
    for (uint8_t i = 0; i < 3; ++i) {
        if (i > 0) body += ",";
        body += "{\"id\":";
        body += String(i + 1);
        body += ",\"state\":\"";
        body += json_escape(status.seats[i]);
        body += "\"}";
    }
    body += "],\"gate\":{\"state\":\"";
    body += json_escape(status.gate_state);
    body += "\"}}";
    return post_json("device-status", "/api/iot/device-status", body);
}

bool cloud_client_upload_card_event(const CardEvent &event)
{
    String body = "{";
    body += "\"device_id\":\"";
    body += DEVICE_ID;
    body += "\",\"uid\":\"";
    body += json_escape(event.uid);
    body += "\",\"type\":\"";
    body += json_escape(event.type);
    body += "\",\"allowed\":";
    body += status_bool(event.allowed);
    body += ",\"time\":\"";
    body += json_escape(event.time);
    body += "\",\"received_ms\":";
    body += String(event.received_ms);
    body += "}";
    return post_json("card-event", "/api/iot/card-event", body, "uid=" + event.uid);
}

void cloud_client_request_device_status_upload()
{
    device_status_pending = true;
}

void cloud_client_request_card_event_upload(const CardEvent &event)
{
    if (card_event_count >= CLOUD_CARD_EVENT_QUEUE_MAX) {
        card_event_tail = (card_event_tail + 1U) % CLOUD_CARD_EVENT_QUEUE_MAX;
        card_event_count--;
        card_event_drop_count++;
        Serial.printf("[CLOUD] card-event queue full, drop oldest drops=%lu\n",
                      static_cast<unsigned long>(card_event_drop_count));
    }

    card_event_queue[card_event_head] = event;
    card_event_head = (card_event_head + 1U) % CLOUD_CARD_EVENT_QUEUE_MAX;
    card_event_count++;
}

uint32_t cloud_client_failed_count()
{
    return failed_upload_count;
}

uint32_t cloud_client_last_upload_ms()
{
    return last_success_upload_ms;
}
