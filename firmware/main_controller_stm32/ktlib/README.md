# ktlib - Kento Debug Library for STM32

## Overview

ktlib is a lightweight debug serial library for STM32F103C8T6, providing:

- USART2 debug output at 115200 baud
- `printf` retargeting to USART2
- A custom 4-byte protocol (`FF CMD VALUE FF`) for runtime commands
- PC13 LED control

## Directory Structure

```
ktlib/
├── README.md
├── kt_config.h                      # Project info and configuration
├── kt_debug/
│   ├── kt_debug.h
│   └── kt_debug.c                   # Debug subsystem: init, system info, help, command dispatch
├── kt_protocol/
│   ├── kt_debug_protocol.h
│   └── kt_debug_protocol.c          # Protocol state machine: FF CMD VALUE FF parser
└── kt_port/
    ├── kt_port_uart.h
    ├── kt_port_uart.c               # UART TX/RX hardware abstraction
    ├── kt_port_gpio.h
    └── kt_port_gpio.c               # GPIO (PC13 LED) hardware abstraction
```

## Integration

### 1. Add source files to Keil project

In Keil uVision:

1. Right-click the project → **Add Group...**:
   - `ktlib_debug`
   - `ktlib_protocol`
   - `ktlib_port`

2. Right-click each group → **Add Existing Files to Group...**:
   - `ktlib_debug`: `..\ktlib\kt_debug\kt_debug.c`
   - `ktlib_protocol`: `..\ktlib\kt_protocol\kt_debug_protocol.c`
   - `ktlib_port`: `..\ktlib\kt_port\kt_port_uart.c` + `..\ktlib\kt_port\kt_port_gpio.c`

### 2. Add Include Paths

**Options for Target → C/C++ → Include Paths**, add:

```
..\ktlib
..\ktlib\kt_debug
..\ktlib\kt_protocol
..\ktlib\kt_port
```

### 3. main.c integration

```c
#include "kt_debug.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"

// After MX_GPIO_Init() and MX_USART2_UART_Init():
kt_port_led_off();
kt_debug_init();
kt_debug_print_system_info();
kt_port_uart_start_receive_it();
```

### 4. HAL_UART_RxCpltCallback

Add to `Core/Src/main.c` or `Core/Src/stm32f1xx_it.c` in the USER CODE section:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    kt_port_uart_rx_callback(huart);
}
```

## Protocol

Frame format: `FF CMD VALUE FF`

| CMD  | VALUE | Function        |
|------|-------|-----------------|
| 0x00 | 0x00  | Turn OFF PC13 LED |
| 0x00 | 0x01  | Turn ON PC13 LED  |
| 0x01 | 0x00  | Print system info |
| 0x02 | 0x00  | Print help        |

## Testing

Serial terminal settings: `115200, 8, N, 1`

After reset, the MCU prints system information.

Send hex bytes `FF 00 01 FF` to turn on PC13 LED.
Send hex bytes `FF 00 00 FF` to turn off PC13 LED.