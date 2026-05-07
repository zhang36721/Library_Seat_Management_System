# ESP32S3 Gateway v1.2

This firmware provides the local UART binary link between the ESP32S3 gateway
and the STM32 main controller. In v1.2, the preferred Web model is cloud-style
active upload:

```text
STM32 -> ESP32S3 -> Backend HTTP API -> Web
```

The ESP32S3 can still expose its local HTTP console for diagnostics, but the
independent Web frontend should read the backend API instead of directly
requesting the ESP32S3 IP.

## UART Wiring

| STM32 main controller | Direction | ESP32S3 |
|-----------------------|-----------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | <-> | GND |

UART parameters: `115200 8N1`.

## Board LED Note

The firmware requests `GPIO48` low at the earliest point in `setup()` to turn
off a possible board RGB/WS2812 LED before UART init:

```cpp
#define BOARD_RGB_LED_PIN 48
```

If the bright LED is a power LED, software cannot turn it off. If the board RGB
LED is really connected to GPIO48, it may conflict with the required UART RX
pin. Keep the current v0.9.1 wiring first, then confirm on real hardware whether
UART traffic makes the LED flicker.

## WiFi Profiles

The gateway tries these profiles in order:

```cpp
constexpr WifiProfile WIFI_PROFILES[] = {
    {"B4-3408_2.4G", "Aa12345678"},
    {"310", "88888888"},
    {"301", "88888888"},
};
```

If all profiles fail, the gateway enters offline mode. UART binary protocol
continues to run regardless of WiFi state.

## Binary Frame

```text
A5 5A VER TYPE SEQ_L SEQ_H LEN_L LEN_H PAYLOAD... CRC_L CRC_H 0D
```

CRC is CRC16-Modbus over `VER` through payload, excluding `A5 5A`, CRC itself,
and the final `0D` tail byte.

## Heartbeat

ESP32S3 sends `HEARTBEAT` every 3000 ms. STM32 replies with
`HEARTBEAT_ACK`. After 3 missed ACKs, ESP32S3 marks STM32 offline but keeps
sending heartbeat frames so STM32 can be recovered automatically. Heartbeat
traffic is intentionally quiet in the serial log; use the Web status page or
STM32 PC13 link flash for visual confirmation.

## Local HTTP Console

After WiFi connects, open the printed ESP32 IP address in a browser:

```text
http://ESP32_IP/
```

Implemented local APIs:

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | Current STM32, WiFi, card count, log count, and DS1302 status |
| GET | `/api/card-events` | Recent CARD_EVENT records cached in ESP32 RAM, max 50 |
| GET | `/api/health` | Basic gateway health and uptime |
| POST | `/api/clear-events` | Clear ESP32 RAM card event cache |

The local Web console uses simple 2 second polling. The preferred v1.2.1 demo
path is now the backend Web console, not direct browser access to ESP32.

## Cloud Upload

Edit these constants in `src/cloud_client.h` before flashing. The ESP32 cannot
use `localhost` or `127.0.0.1`, because those addresses point back to the ESP32
itself. Use the backend computer LAN IP, server public IP, or domain name.

```cpp
constexpr const char *DEVICE_ID = "kento-main-001";
constexpr const char *SERVER_BASE_URL_3408 = "http://192.168.1.100:18080";
constexpr const char *SERVER_BASE_URL_301 = "http://192.168.141.236:18080";
constexpr const char *SERVER_BASE_URL_310 = "http://192.168.223.93:18080";
constexpr const char *SERVER_BASE_URL_PROD = "https://www.kento.top/library-seat";
constexpr bool CLOUD_USE_PROD_SERVER = true;
constexpr uint32_t CLOUD_HEARTBEAT_PERIOD_MS = 2000;
```

When `CLOUD_USE_PROD_SERVER` is `true`, the firmware always uploads to
`SERVER_BASE_URL_PROD`. Set it to `false` only for LAN demo mode, where the
firmware selects the backend URL from the connected WiFi SSID:

| WiFi SSID | Backend URL |
|-----------|-------------|
| `B4-3408_2.4G` | `http://192.168.1.100:18080` |
| `301` | `http://192.168.141.236:18080` |
| `310` | `http://192.168.223.93:18080` |

`www.kento.top/library-seat` is the formal deployment entry. The root path
`https://www.kento.top/` can keep running the existing project.

Start the backend on the PC/server with:

```bash
uvicorn backend.main:app --host 0.0.0.0 --port 18080
```

Implemented active upload endpoints:

| Direction | Method | Path |
|-----------|--------|------|
| ESP32S3 -> backend | POST | `/api/iot/heartbeat` |
| ESP32S3 -> backend | POST | `/api/iot/device-status` |
| ESP32S3 -> backend | POST | `/api/iot/card-event` |

HTTP upload uses a short timeout so backend/network failure does not block the
STM32 local access flow for long. Upload failures are counted and printed, while
UART communication continues.

## Build

Use PlatformIO:

```bash
pio run
```

This repository environment may not have PlatformIO installed; install it before
building/flashing the ESP32S3 firmware.

