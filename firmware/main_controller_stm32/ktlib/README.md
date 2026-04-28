# ktlib - Kento Debug Library for STM32 (v0.2)

## Overview

ktlib is a lightweight, modular debug serial library for STM32F103C8T6.  
**v0.2** introduces a clean layered architecture with interrupt-safe ring buffer,
protocol state machine, and command dispatch — all business logic runs in the
main loop, never inside ISRs.

| Module            | Responsibility |
|-------------------|----------------|
| `kt_config.h`     | Project identity & compile-time constants |
| `kt_port/uart`    | USART2 hardware abstraction (TX string/byte, ring‑buffer RX) |
| `kt_port/gpio`    | PC13 LED on/off/toggle (active‑low) |
| `kt_components/ringbuf` | Interrupt‑safe 256‑byte FIFO ring buffer |
| `kt_protocol`     | Frame parser state machine (`FF CMD DATA FF`) |
| `kt_cmd`          | Command dispatch table (LED, info, help, status) |
| `kt_log`          | Logging macros (`KT_LOG_INFO`, `KT_LOG_WARN`, etc.) |
| `kt_debug`        | Top‑level init, system info banner, main‑loop task |

## Architecture (v0.2)

```
HAL_UART_RxCpltCallback (ISR)
    └─► kt_port_uart_rx_push(byte)   // ring buffer write

main() loop
    └─► kt_debug_task()
          └─► kt_port_uart_rx_read(byte)
                └─► kt_protocol_input_byte(byte)
                      └─► [frame complete]
                            └─► kt_cmd_dispatch(cmd, data)
                                  └─► LOG / LED / info / help
```

**Key design principles:**
- **ISR only writes to ring buffer** — no parsing, no logging, no long operations
- **Main loop drains ring buffer** — all parsing and business logic runs at
  background priority
- **No legacy `kt_debug_protocol`** — replaced by separated `kt_protocol` (parse)
  and `kt_cmd` (execute)

## Directory Structure

```
ktlib/
├── README.md
├── kt_config.h
├── kt_cmd/
│   ├── kt_cmd.h
│   └── kt_cmd.c                   # Command dispatch table
├── kt_components/
│   ├── kt_ringbuf.h
│   └── kt_ringbuf.c               # ISR-safe ring buffer
├── kt_debug/
│   ├── kt_debug.h
│   └── kt_debug.c                 # Top-level init, system info, task
├── kt_log/
│   ├── kt_log.h
│   └── kt_log.c                   # Log formatting & output
├── kt_port/
│   ├── kt_port_uart.h
│   ├── kt_port_uart.c             # UART TX + ring-buffer RX
│   ├── kt_port_gpio.h
│   └── kt_port_gpio.c             # PC13 LED (active-low)
├── kt_protocol/
│   ├── kt_protocol.h
│   ├── kt_protocol.c              # Frame parser (FF CMD DATA FF)
│   ├── kt_debug_protocol.h        # (OBSOLETE – kept for reference)
│   └── kt_debug_protocol.c        # (OBSOLETE – kept for reference)
```

## Integration (Keil MDK-ARM)

### 1. Source files (already added to uvprojx v0.2)

| Keil Group              | Files |
|-------------------------|-------|
| `ktlib/ktlib_debug`     | `kt_debug.c` |
| `ktlib/ktlib_protocol`  | `kt_debug_protocol.c`, `kt_protocol.c` |
| `ktlib/ktlib_port`      | `kt_port_uart.c`, `kt_port_gpio.c` |
| `ktlib/ktlib_log`       | `kt_log.c` |
| `ktlib/ktlib_components` | `kt_ringbuf.c` |
| `ktlib/ktlib_cmd`       | `kt_cmd.c` |

### 2. Include Paths (already added to uvprojx v0.2)

```
..\ktlib
..\ktlib\kt_debug
..\ktlib\kt_log
..\ktlib\kt_protocol
..\ktlib\kt_port
..\ktlib\kt_components
..\ktlib\kt_cmd
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

// In main while loop:
while (1) {
    kt_debug_task();
    // ... other application code ...
}
```

### 4. HAL_UART_RxCpltCallback (in stm32f1xx_it.c)

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        kt_port_uart_rx_push(kt_port_uart_rx_get_byte());
        HAL_UART_Receive_IT(&huart2, &kt_port_uart_rx_byte, 1);
    }
}
```

**IMPORTANT:** You must remove any old implementation that calls
`kt_debug_uart_rx_callback()` — that function no longer exists in v0.2.
The ISR should ONLY push the byte into the ring buffer and restart
the interrupt receive.

## Protocol

Frame format: `FF CMD DATA FF`

| CMD  | DATA | Function                   |
|------|------|----------------------------|
| 0x01 | xx   | Print system info          |
| 0x02 | xx   | Turn ON  PC13 LED          |
| 0x03 | xx   | Turn OFF PC13 LED          |
| 0x04 | xx   | Print debug status         |

Note: v0.2 uses CMD 0x02 for LED ON and 0x03 for LED OFF (not 0x00 as in
the original spec). This makes each command semantically distinct.

## Testing

Serial terminal settings: `115200, 8, N, 1`

1. **Reset MCU** → System info banner printed
2. **Send `FF 02 00 FF`** → `[LOG] PC13 LED ON`, LED lights
3. **Send `FF 03 00 FF`** → `[LOG] PC13 LED OFF`, LED off
4. **Send `FF 01 00 FF`** → System info reprinted
5. **Send `FF 04 00 FF`** → Debug status printed
6. **Invalid frame** → `[LOG] Unknown cmd: 0xXX` or `[LOG] Unhandled data: 0xXX`