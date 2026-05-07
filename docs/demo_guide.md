# v1.3 Demo Guide

This guide is for the final local demo and acceptance flow. The current demo
focuses on the path:

```text
STM32 main controller -> ESP32S3 gateway -> FastAPI backend -> Vue Web console
```

## 1. Hardware Wiring

### STM32 Main Controller To ESP32S3

| STM32F103C8T6 | Direction | ESP32S3 | Note |
|---------------|-----------|---------|------|
| PB10 USART3_TX | -> | GPIO48 UART_RX | 115200 8N1 |
| PB11 USART3_RX | <- | GPIO47 UART_TX | 115200 8N1 |
| GND | <-> | GND | Common ground required |

USART2 remains the STM32 debug port and must not be used by ESP32S3.

### Main Controller Local Modules

| Module | Status In Demo |
|--------|----------------|
| RC522 | Reads UID for local access flow |
| OLED | Shows home, access OK, access denied |
| DS1302 | Provides event time |
| Buzzer | Success/fail prompt |
| Stepper/ULN2003 | Simulates gate open/close |
| 8-key module | Local menu, card add/delete, time setting |
| Seat status | Currently simulated on main controller: S1 FREE, S2 OCCUPIED, S3 FREE |

## 2. Startup Order

1. Start the backend from the repository root:

   ```bash
   python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080
   ```

2. Start the frontend:

   ```bash
   cd frontend
   npm run dev -- --host 0.0.0.0 --port 15173
   ```

3. Power on the STM32 main controller.

4. Power on the ESP32S3 gateway.

5. Open the Web console:

   ```text
   http://localhost:15173/
   ```

## 3. ESP32 Backend URL

ESP32 cannot use `localhost` or `127.0.0.1` for the backend. Those addresses
point to the ESP32 itself. Use the backend PC LAN IP, public IP, or domain.

Current ESP32 URL selection in `firmware/esp32s3_gateway/src/cloud_client.h`:

| WiFi SSID | Backend URL |
|-----------|-------------|
| `B4-3408_2.4G` | `http://192.168.1.100:18080` |
| `301` | `http://192.168.141.236:18080` |
| `310` | `http://192.168.223.93:18080` |
| Production | `https://www.kento.top/library-seat` |

The production domain is reserved but not selected automatically yet.

## 4. Demo Steps

1. Confirm ESP32 serial log shows WiFi connected.
2. Confirm ESP32 serial log shows:

   ```text
   [CLOUD] heartbeat POST OK code=200
   ```

3. Confirm Web shows:

   - Backend service: normal
   - ESP32 gateway: online
   - STM32 main controller: online
   - Seat 1: free
   - Seat 2: occupied
   - Seat 3: free
   - Gate: closed or current gate state

4. Swipe a registered card.

   Expected:

   - OLED shows access OK
   - Buzzer gives success prompt
   - Gate state changes: opening -> open -> closing -> closed
   - Web shows a CHECK_IN or CHECK_OUT record

5. Swipe an unregistered card.

   Expected:

   - OLED shows access denied
   - Buzzer gives fail prompt
   - Gate does not open
   - Web shows a denied record

6. Disconnect ESP32 power or WiFi.

   Expected:

   - Web shows ESP32 gateway connection lost within about 10 seconds

7. Restore ESP32.

   Expected:

   - Web returns to ESP32 online after heartbeat upload resumes

8. Click Clear Events.

   Expected:

   - Recent Web card-event cache is cleared
   - Later swipes still appear normally

## 5. Common Issues

| Issue | Check |
|-------|-------|
| ESP32 upload fails | Confirm `SERVER_BASE_URL_*` uses backend PC LAN IP, not `localhost` |
| ESP32 cannot reach backend | Start backend with `--host 0.0.0.0` |
| Backend works locally but ESP32 cannot connect | Check Windows firewall allows inbound TCP 18080 |
| ESP32 firmware cannot build | Install PlatformIO first, then run `pio run` in `firmware/esp32s3_gateway` |
| Web says backend unavailable | Confirm backend is running and frontend backend address is correct |
| Web says ESP32 connection lost | Confirm ESP32 WiFi and cloud heartbeat POST logs |
| STM32 offline | Confirm USART3 wiring PB10/PB11 to ESP32 GPIO48/GPIO47 and common GND |
| Seat status not real | Current seat status is simulated on the main controller; replace with seat node/ZigBee data later |

## 6. Current Limits

- No login.
- No user management.
- No reservation workflow.
- No database persistence.
- Backend stores IoT status and recent card events in RAM only.
- Seat status is currently main-controller simulated data.
- ESP32 PlatformIO build must be verified on a machine with PlatformIO installed.
