# Protocol And Data Flow

## USART2 Debug Protocol

STM32 debug commands use:

```text
FF CMD DATA FF
```

USART2 is reserved for debug and must not be shared with ZigBee or ESP32S3.

STM32 main-controller registered cards are saved when `MAIN_CARD_DB_FLASH_ENABLE`
is `1`. Access logs are RAM only while `MAIN_ACCESS_LOG_FLASH_ENABLE` is `0`.
The long-term source of access records is the ESP32 -> backend `CARD_EVENT`
upload path.

Examples:

| Command | Purpose |
|---------|---------|
| `FF A1 00 FF` | Manual local card flow test |
| `FF B6 00 FF` | Print registered cards |
| `FF C0 00 FF` | Print current-boot RAM access logs |
| `FF D2 00 FF` | Print ESP32 link status |
| `FF 31 00 FF` | Seat node prints three seat states |
| `FF 35 00 FF` | Seat node prints LED map |
| `FF 36 00 FF` | Seat node refreshes/prints LED state |

## Main STM32 <-> ESP32S3 Binary Protocol

Frame:

```text
A5 5A VER TYPE SEQ_L SEQ_H LEN_L LEN_H PAYLOAD CRC_L CRC_H 0D
```

CRC is CRC16-Modbus over `VER` through payload. EOF `0D` is checked but not included in CRC.

Core message types:

| Type | Direction | Purpose |
|------|-----------|---------|
| `HEARTBEAT` | ESP32 -> STM32 | Link heartbeat |
| `HEARTBEAT_ACK` | STM32 -> ESP32 | Heartbeat response |
| `DEVICE_STATUS` | STM32 -> ESP32 | Cards/logs/time/seats/gate |
| `CARD_EVENT` | STM32 -> ESP32 | Card access event |
| `ACK` | ESP32 -> STM32 | Confirm non-heartbeat message |
| `ERR` | Both | Protocol error |

Heartbeat logs are suppressed by default. Card events and upload failures remain visible.

## Main STM32 <-> Seat Node ZigBee Protocol

CC2530 UART baud rate:

```text
38400 8N1
```

Frame:

```text
FA ADDRL ADDRH LEN DATA F5
```

Addresses are little-endian:

| Node | Short address | Frame bytes |
|------|---------------|-------------|
| Main coordinator | `0x0000` | `00 00` |
| Seat endpoint | `0x301E` | `1E 30` |

ZigBee uses binary payload only. ASCII text tunneling is not used.

Payload types:

```text
01 seq              PING
02 seq              PONG
10 S1 S2 S3         Seat status
```

State values:

```text
0 = FREE
1 = OCCUPIED
2 = UNKNOWN
```

The seat node sends `10 S1 S2 S3` once on boot and then only when the seat state is stable and changed. The main controller ignores duplicate seat payloads.

## ESP32 -> Backend HTTP Upload

ESP32 actively uploads:

| Endpoint | Period/Trigger |
|----------|----------------|
| `POST /api/iot/heartbeat` | Every 2 seconds |
| `POST /api/iot/device-status` | On STM32 status update |
| `POST /api/iot/card-event` | On card event |

HTTP failures are logged but do not affect local STM32 access control.

## Web -> Backend Read

The Web frontend reads:

| Endpoint | Purpose |
|----------|---------|
| `GET /api/iot/status` | Device, seats, gate, heartbeat, time |
| `GET /api/iot/card-events` | Recent card events |
| `POST /api/iot/clear-events` | Clear backend RAM event cache |

The page refreshes every 2 seconds. ESP32 is considered offline after 10 seconds without heartbeat.

## Full Demo Data Flow

```text
Card swipe
  -> STM32 RC522 reads UID
  -> local card table authorizes/denies
  -> OLED/buzzer/gate update locally
  -> STM32 sends CARD_EVENT to ESP32S3
  -> ESP32 POSTs card-event to backend
  -> Web polls backend and displays the record
```

Seat flow:

```text
Seat sensor changes
  -> seat node confirms stable state
  -> CC2530 ZigBee sends SEAT payload
  -> main STM32 updates seat state
  -> STM32 sends DEVICE_STATUS to ESP32S3
  -> ESP32 POSTs device-status to backend
  -> Web displays seat state
```
