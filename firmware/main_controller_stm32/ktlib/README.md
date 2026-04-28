# ktlib - Kento Debug Library for STM32 (v0.2.1)

## Overview

ktlib is a lightweight, modular debug serial library for STM32F103C8T6.  
**v0.2.1** builds on v0.2's clean layered architecture with these additions:

- **Configurable timeouts** — partial-frame RX timeout + finite TX timeout
- **Protocol statistics** — timeout drops, error drops, RX overflow counter
- **Expanded FF 04 status** — real-time debugging diagnostics
- **Robust ring buffer** — null/size guards against invalid init
- **ISR hardening** — no logging in ISR, overflow silently counted

| Module                   | Responsibility |
|--------------------------|----------------|
| `kt_config.h`            | Project identity & compile-time constants |
| `kt_port/uart`           | USART2 hardware abstraction (TX string/byte, ring‑buffer RX) |
| `kt_port/gpio`           | PC13 LED on/off/toggle (active‑low) |
| `kt_components/ringbuf`  | Interrupt‑safe 256‑byte FIFO ring buffer |
| `kt_protocol`            | Frame parser state machine (`FF CMD DATA FF`) with timeout watchdog |
| `kt_cmd`                 | Command dispatch table (LED, info, help, status) |
| `kt_log`                 | Logging macros (`KT_LOG_INFO`, `KT_LOG_WARN`, etc.) |
| `kt_debug`               | Top‑level init, system info banner, main‑loop task |

## Architecture (v0.2.1)

```
HAL_UART_RxCpltCallback (ISR)
    └─► kt_port_uart_rx_callback(huart)
          ├─► kt_ringbuf_put(byte)          // ISR: write only
          └─► overflow counter (if full)     // ISR: silent count, no logging
          └─► HAL_UART_Receive_IT re-arm    // ISR: restart

main() loop
    └─► kt_debug_task()
          ├─► HAL_GetTick()                 // One timestamp per iteration
          ├─► kt_protocol_check_timeout()   // Discard stalled partial frames
          └─► drain ring buffer:
                kt_ringbuf_get(byte)
                  └─► kt_protocol_input_byte(byte, now_ms)
                        └─► [frame complete]
                              └─► kt_cmd_dispatch(cmd, data)
                                    └─► LOG / LED / info / help / status
```

**Key design principles:**
- **ISR only writes to ring buffer** — no parsing, no callbacks, no logging, no blocking
- **Main loop drains ring buffer** — all parsing and business logic at background priority
- **Timeout protection** — partial frames auto-reset after `KT_PROTOCOL_RX_TIMEOUT_MS`
- **Finite TX timeout** — `HAL_UART_Transmit` will not block indefinitely
- **No legacy `kt_debug_protocol`** — replaced by separated `kt_protocol` (parse) + `kt_cmd` (execute)

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
│   ├── kt_protocol.c              # Frame parser (FF CMD DATA FF) + timeout
│   ├── kt_debug_protocol.h        # (OBSOLETE – kept for reference)
│   └── kt_debug_protocol.c        # (OBSOLETE – kept for reference)
```

## Integration (Keil MDK-ARM)

### 1. Source files (already added to uvprojx v0.2)

| Keil Group                  | Files |
|-----------------------------|-------|
| `ktlib/ktlib_debug`         | `kt_debug.c` |
| `ktlib/ktlib_protocol`      | `kt_debug_protocol.c`, `kt_protocol.c` |
| `ktlib/ktlib_port`          | `kt_port_uart.c`, `kt_port_gpio.c` |
| `ktlib/ktlib_log`           | `kt_log.c` |
| `ktlib/ktlib_components`    | `kt_ringbuf.c` |
| `ktlib/ktlib_cmd`           | `kt_cmd.c` |

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

### 4. HAL_UART_RxCpltCallback (in stm32f1xx_it.c or main.c)

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    kt_port_uart_rx_callback(huart);
}
```

**IMPORTANT:** The ISR callback no longer takes a `byte` argument. v0.2.1 moves
the volatile `kt_uart_rx_byte` inside `kt_port_uart.c` and the callback checks
`huart->Instance == USART2` internally.  All ISR operations (ring‑buffer write,
overflow counting, and UART re‑arming) happen within `kt_port_uart_rx_callback()`.

Do NOT call any logging or callback functions from ISR context.

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

## Timeout Configuration (v0.2.1)

| Macro                        | Default | Description |
|------------------------------|---------|-------------|
| `KT_PROTOCOL_RX_TIMEOUT_MS`  | 500     | Partial-frame timeout. If `FF xx ...` is not completed within this window, the parser resets to IDLE. |
| `KT_UART_TX_TIMEOUT_MS`      | 100     | TX timeout for `HAL_UART_Transmit`. Prevents blocking indefinitely if UART TX is stuck. |

Both are defined in `kt_config.h` and can be overridden before including.

## Diagnostics — FF 04 Status (v0.2.1 expanded)

Sending `FF 04 00 FF` prints real‑time diagnostics:

```
[LOG] === Debug Status (v0.2.1) ===
[LOG] LED State      : OFF
[LOG] Protocol       : FF CMD DATA FF
[LOG] Build          : Apr 28 2026 15:21:12
[LOG] RX ring avail  : 0
[LOG] RX overflow    : 0
[LOG] Timeout drops  : 0
[LOG] Error drops    : 0
[LOG] ===============================
```

| Field            | Meaning |
|------------------|---------|
| RX ring avail    | Bytes in ring buffer waiting to be parsed |
| RX overflow      | Bytes lost because ring buffer was full (ISR drops) |
| Timeout drops    | Partial frames discarded after timeout (no trailing FF) |
| Error drops      | Frames with incorrect trailer byte (not 0xFF) |

These counters keep rolling — they indicate communication quality issues.

## Testing

Serial terminal settings: `115200, 8, N, 1`

1. **Reset MCU** → System info banner printed
2. **Send `FF 02 00 FF`** → `[LOG] PC13 LED ON`, LED lights
3. **Send `FF 03 00 FF`** → `[LOG] PC13 LED OFF`, LED off
4. **Send `FF 01 00 FF`** → System info reprinted
5. **Send `FF 04 00 FF`** → Debug status with v0.2.1 statistics
6. **Send invalid frame** (e.g., `FF 55 66 AA`) → `[LOG] Invalid frame: expected FF, got 0xAA`, error_drop_count increments
7. **Send partial frame then wait 500ms** → `[LOG] Protocol timeout: discarding partial frame`, timeout_drop_count increments

## Changelog

### v0.2.1 (2026-04-28)
- Add `KT_PROTOCOL_RX_TIMEOUT_MS` (500ms default) — state machine resets partial frames
- Add `KT_UART_TX_TIMEOUT_MS` (100ms default) — finite TX timeout replaces HAL_MAX_DELAY
- Protocol parser tracks elapsed time per state transition
- Protocol statistics: `kt_protocol_get_timeout_drop_count()`, `kt_protocol_get_error_drop_count()`
- UART overflow counter: `kt_port_uart_rx_overflow_count()`
- FF 04 status expanded to show all diagnostics (ring usage, overflow, timeout drops, error drops)
- `kt_protocol_input_byte(byte, now_ms)` — requires timestamp from `HAL_GetTick()`
- `kt_port_uart_rx_callback` simplified: ISR does ring‑buffer push + overflow count + re‑arm; no byte parameter, no logging
- Ring buffer full validation: `kt_ringbuf_init/put/get` reject NULL pointers, buffer size < 2
- `kt_debug_task()` calls `HAL_GetTick()` once per iteration and passes to protocol layer
- `kt_debug_print_system_info()` uses `snprintf` + `kt_port_uart_tx_string` (no printf/semihosting dependency)

### v0.2 (2026-04-28)
- Complete rewrite with ring buffer, protocol state machine, command dispatch
- ISR only writes to ring buffer; main loop does all parsing
- Separated `kt_protocol` (parse) from `kt_cmd` (execute)
- Added `kt_log` module for consistent log formatting
- Removed old `kt_debug_uart_rx_callback` pattern