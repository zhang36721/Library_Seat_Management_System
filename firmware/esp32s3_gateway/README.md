# ESP32S3 Gateway v0.9

This firmware validates the local UART binary link between the ESP32S3 gateway
and the STM32 main controller. It does not perform HTTP upload in v0.9.

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
sending heartbeat frames so STM32 can be recovered automatically.

## Build

Use PlatformIO:

```bash
pio run
```

This repository environment may not have PlatformIO installed; install it before
building/flashing the ESP32S3 firmware.
