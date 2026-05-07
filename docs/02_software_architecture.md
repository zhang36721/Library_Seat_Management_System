# Software Architecture

## STM32 Main Controller

Main entry:

```text
Core/Src/main.c
```

Startup flow:

```text
HAL/Cube init
app_io_init()
kt_modules_init()
main_controller_app_init()
kt_task_register(...)
while (1) kt_task_run()
```

Main modules:

| Module | Responsibility |
|--------|----------------|
| `kt_cmd` | USART2 debug command dispatch only |
| `kt_modules` | Peripheral startup/task entry |
| `kt_app/main_controller_app` | Local card flow, OLED pages, gate state, seat state, buzzer pattern |
| `main_card_db` | Local registered-card table |
| `main_access_log` | Recent access records and Flash persistence |
| `main_keys` | 8-key scan and menu events |
| `kt_esp32_link` | USART3 binary protocol to ESP32S3 |
| `kt_uart_links` + `kt_zigbee` | USART1 CC2530 point-to-point seat link |

## Seat Node STM32

Entry:

```text
firmware/seat_node_stm32/Core/Src/main.c
```

Key modules:

| Module | Responsibility |
|--------|----------------|
| `seat_node_app` | HX711/IR/GPIO seat detection, LED state, ZigBee report |
| `kt_zigbee` | CC2530 `FA ADDR LEN DATA F5` frame send/parse |
| `kt_debug` | USART2 debug commands |
| `kt_port` | Basic UART/GPIO helpers |

The seat node confirms a seat change only after 3 consistent samples. It reports to the main controller on boot and on stable state changes.

## ESP32S3 Gateway

Entry:

```text
firmware/esp32s3_gateway/src/src.ino
```

Modules:

| Module | Responsibility |
|--------|----------------|
| `app_gateway` | High-level setup/task loop |
| `wifi_manager` | WiFi profile selection and state |
| `stm32_link` | UART binary protocol with STM32 |
| `binary_protocol` | CRC16 and binary frame helpers |
| `device_state` | RAM cache of status and card events |
| `cloud_client` | HTTP active upload to backend |
| `http_server` | Optional local ESP32 page/API |

## Backend

Entry:

```text
backend/main.py
```

Minimal IoT API:

| Endpoint | Direction | Purpose |
|----------|-----------|---------|
| `POST /api/iot/heartbeat` | ESP32 -> backend | Gateway heartbeat |
| `POST /api/iot/device-status` | ESP32 -> backend | STM32 device, seats, gate, time |
| `POST /api/iot/card-event` | ESP32 -> backend | Card access event |
| `GET /api/iot/status` | Web -> backend | Current device status |
| `GET /api/iot/card-events` | Web -> backend | Recent card events |
| `POST /api/iot/clear-events` | Web -> backend | Clear RAM event cache |

Data is kept in memory for the demo. No login or database is used in v1.3.

## Frontend

Main view:

```text
frontend/src/views/DeviceConsole.vue
```

API wrapper:

```text
frontend/src/api/iotBackend.js
```

The Web console polls the backend every 2 seconds, displays backend/ESP32/STM32 status, seats, gate state, DS1302 time, card count, log count, and recent card events. ESP32 offline timeout is 10 seconds to avoid visual flicker.
