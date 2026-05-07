#ifndef CLOUD_CLIENT_H
#define CLOUD_CLIENT_H

#include "device_state.h"

constexpr const char *DEVICE_ID = "kento-main-001";
constexpr const char *SERVER_BASE_URL_3408 = "http://192.168.1.100:18080";
constexpr const char *SERVER_BASE_URL_301 = "http://192.168.141.236:18080";
constexpr const char *SERVER_BASE_URL_310 = "http://192.168.223.93:18080";
constexpr const char *SERVER_BASE_URL_PROD = "https://www.kento.top/library-seat";
constexpr bool CLOUD_USE_PROD_SERVER = true;
constexpr uint32_t CLOUD_HEARTBEAT_PERIOD_MS = 2000;
constexpr uint32_t CLOUD_HTTP_TIMEOUT_MS = 500;
constexpr uint32_t CLOUD_PENDING_RETRY_MS = 2000;
#define CLOUD_VERBOSE_LOG 0

void cloud_client_begin();
void cloud_client_task();
bool cloud_client_upload_device_status();
bool cloud_client_upload_card_event(const CardEvent &event);
void cloud_client_request_device_status_upload();
void cloud_client_request_card_event_upload(const CardEvent &event);
uint32_t cloud_client_failed_count();
uint32_t cloud_client_last_upload_ms();

#endif
