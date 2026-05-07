# Hardware Overview

## System Composition

This project is a local library seat management demo system. The current main demo chain is:

```text
STM32 main controller -> ESP32S3 gateway -> backend -> Web console
```

Hardware modules:

| Module | Role | Current status |
|--------|------|----------------|
| STM32F103C8T6 main controller | Local access control, RC522/OLED/DS1302/buzzer/keys/gate/ESP32 link | Implemented and rebuild passes |
| ESP32S3 gateway | WiFi, UART binary protocol, HTTP active upload | Implemented; PlatformIO build depends on local environment |
| RC522 | IC card UID read | Tested: UID read OK |
| OLED | Local UI and card result display | Tested OK |
| DS1302 | Local time source | Tested valid read/write |
| Buzzer | Access success/fail indication | Success one short beep; failure three short beeps |
| 8-key module | Local menu and card/time operations | Tested OK |
| ULN2003 stepper gate | Gate open/close simulation | Control chain OK; motor mechanics remain hardware-dependent |
| Seat node STM32 | Seat sensor collection and ZigBee report | Implemented; live ZigBee acceptance pending |
| CC2530 ZigBee | Main/seat point-to-point wireless link | USART1 38400, short-address frame |

## Main Controller Wiring

| Function | STM32 pin | Notes |
|----------|-----------|-------|
| Debug USART2 TX/RX | PA2 / PA3 | 115200, `FF CMD DATA FF` |
| ZigBee USART1 TX/RX | PA9 / PA10 | 38400, CC2530 point-to-point frame |
| ESP32S3 USART3 TX/RX | PB10 / PB11 | 115200, binary protocol |
| RC522 SPI | PA5 / PA6 / PA7 / PB12 / PB13 | SCK/MISO/MOSI/NSS/RST |
| OLED I2C | PB6 / PB7 | Software I2C |
| DS1302 | PB0 / PB1 / PB5 | CLK/DAT/RST |
| Buzzer | PA4 | Default off at boot |
| Run LED | PC13 | Active-low |
| Stepper ULN2003 | PB8 / PB9 / PB14 / PB15 | IN1-IN4 |

## Seat Node Wiring

| Function | STM32 pin | Notes |
|----------|-----------|-------|
| Debug USART2 TX/RX | PA2 / PA3 | 115200 |
| ZigBee USART1 TX/RX | PA9 / PA10 | 38400 |
| Seat 1 HX711 | PA4 DT / PA5 SCK | Pressure/weight |
| Seat 1 IR | PB9 | IR active level currently high |
| Seat 2 input | PB8 | High = occupied |
| Seat 3 input | PB7 | High = occupied |

Seat 1 is occupied only when weight is greater than 50g and IR is detected. Seat 2 and seat 3 use high-level GPIO detection.

## Seat LED Wiring

| Indicator | Pin | Rule |
|-----------|-----|------|
| Seat 1 red | PB5 | Occupied |
| Seat 1 green | PB6 | Free |
| Seat 2 red | PB3 | Occupied |
| Seat 2 green | PB4 | Free |
| Seat 3 red | PA12 | Occupied |
| Seat 3 green | PA15 | Free |
| IR detected | PB14 | Seat 1 IR active |
| Weight detected | PB15 | Seat 1 weight > 50g |

PA15/PB3/PB4 require JTAG disabled while keeping SWD enabled. The seat-node GPIO initialization applies `__HAL_AFIO_REMAP_SWJ_NOJTAG()`.

## ESP32S3 Wiring

| STM32 main controller | ESP32S3 |
|-----------------------|---------|
| PB10 USART3_TX | GPIO48 UART_RX |
| PB11 USART3_RX | GPIO47 UART_TX |
| GND | GND |

GPIO48 may conflict with a board RGB LED on some ESP32S3 boards. If UART causes LED flicker, use another available UART RX pin or handle the board LED physically.

## Tested / Pending

Tested: RC522 UID read, OLED display, DS1302 valid time, buzzer, key module, local card flow, ESP32 active upload path.

Pending live confirmation: seat LED wiring, CC2530 wireless stability, stepper mechanical rotation under final power/phase wiring.
