# ktlib - Kento Debug Library for STM32 (v0.3)

## Overview

ktlib is a lightweight, modular debug serial library for STM32F103C8T6.  
**v0.3** adds a non-blocking cooperative task scheduler on top of v0.2.1's architecture:

- **Task scheduler** — statically-allocated periodic task dispatch
- **Tick abstraction** — `kt_tick_get_ms()` / `kt_tick_is_timeout()` wrapping `HAL_GetTick()`
- **Refactored main loop** — `kt_task_run()` drives all periodic work (debug protocol drain, heartbeat LED, status printing)
- **Zero malloc** — all task descriptors are static

| Module                   | Responsibility |
|--------------------------|----------------|
| `kt_config.h`            | Project identity & compile-time constants |
| `kt_system/tick`         | Millisecond tick abstraction (wrap `HAL_GetTick`) |
| `kt_task`                | Cooperative periodic task scheduler |
| `kt_port/uart`           | USART2 hardware abstraction (TX string/byte, ring‑buffer RX) |
| `kt_port/gpio`           | PC13 LED on/off/toggle (active‑low) |
| `kt_components/ringbuf`  | Interrupt‑safe 256‑byte FIFO ring buffer |
| `kt_protocol`            | Frame parser state machine (`FF CMD DATA FF`) with timeout watchdog |
| `kt_cmd`                 | Command dispatch table (LED, info, help, status) |
| `kt_log`                 | Logging macros (`KT_LOG_INFO`, `KT_LOG_WARN`, etc.) |
| `kt_debug`               | Top‑level init, system info banner, main‑loop task |

## Architecture (v0.3)

```
HAL_UART_RxCpltCallback (ISR)
    └─► kt_port_uart_rx_callback(huart)
          ├─► kt_ringbuf_put(byte)          // ISR: write only
          ├─► overflow counter (if full)     // ISR: silent count, no logging
          └─► HAL_UART_Receive_IT re-arm    // ISR: restart

main() loop
    └─► kt_task_run()
          ├─► iterates all registered tasks
          ├─► checks kt_tick_is_timeout() for each
          └─► calls task function if due
                ├─► "debug"     (1 ms)    → kt_debug_task()   — drain ring buffer, parse protocol
                ├─► "heartbeat" (500 ms)  → app_heartbeat_task() — toggle PC13 LED
                ├─► "status"    (3000 ms) → app_status_task()    — print uptime
                └─► (extensible up to 8 tasks)
```

**Key design principles (v0.3):**
- **Cooperative multitasking** — no RTOS, no interrupts, no stack switching
- **Single tick source** — `kt_tick_get_ms()` wraps `HAL_GetTick()`, all timing flows through it
- **Wrap-around safe** — `kt_tick_is_timeout()` uses unsigned subtraction for correct 49.7-day rollover
- **Debug task at 1ms** — ensures prompt UART protocol parsing without blocking other tasks
- **Extensible** — add your own tasks via `kt_task_register("name", func_ptr, period_ms)`

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
├── kt_system/
│   ├── kt_tick.h
│   └── kt_tick.c                  # Millisecond tick abstraction
└── kt_task/
    ├── kt_task.h
    └── kt_task.c                  # Cooperative periodic task scheduler
```

## Integration (Keil MDK-ARM)

### 1. Source files

| Keil Group                  | Files |
|-----------------------------|-------|
| `ktlib/ktlib_debug`         | `kt_debug.c` |
| `ktlib/ktlib_protocol`      | `kt_debug_protocol.c`, `kt_protocol.c` |
| `ktlib/ktlib_port`          | `kt_port_uart.c`, `kt_port_gpio.c` |
| `ktlib/ktlib_log`           | `kt_log.c` |
| `ktlib/ktlib_components`    | `kt_ringbuf.c` |
| `ktlib/ktlib_cmd`           | `kt_cmd.c` |
| `ktlib/ktlib_system`        | `kt_tick.c` |
| `ktlib/ktlib_task`          | `kt_task.c` |

### 2. Include Paths

```
..\ktlib
..\ktlib\kt_debug
..\ktlib\kt_log
..\ktlib\kt_protocol
..\ktlib\kt_port
..\ktlib\kt_components
..\ktlib\kt_cmd
..\ktlib\kt_system
..\ktlib\kt_task
```

### 3. main.c integration (v0.3)

```c
#include "kt_debug.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"
#include "kt_task/kt_task.h"

// After MX_GPIO_Init() and MX_USART2_UART_Init():
kt_port_led_off();
kt_debug_init();
kt_debug_print_system_info();
kt_port_uart_start_receive_it();

// v0.3: Task scheduler
kt_task_init();
kt_task_register("debug",     kt_debug_task,       1);
kt_task_register("heartbeat", app_heartbeat_task, 500);
kt_task_register("status",    app_status_task,    3000);

// In main while loop:
while (1) {
    kt_task_run();
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

## Timeout Configuration

| Macro                        | Default | Description |
|------------------------------|---------|-------------|
| `KT_PROTOCOL_RX_TIMEOUT_MS`  | 500     | Partial-frame timeout. If `FF xx ...` is not completed within this window, the parser resets to IDLE. |
| `KT_UART_TX_TIMEOUT_MS`      | 100     | TX timeout for `HAL_UART_Transmit`. Prevents blocking indefinitely if UART TX is stuck. |

## Diagnostics — FF 04 Status

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

## Testing

Serial terminal settings: `115200, 8, N, 1`

1. **Reset MCU** → System info banner printed
2. **Send `FF 02 00 FF`** → `[LOG] PC13 LED ON`, LED lights
3. **Send `FF 03 00 FF`** → `[LOG] PC13 LED OFF`, LED off
4. **Send `FF 01 00 FF`** → System info reprinted
5. **Send `FF 04 00 FF`** → Debug status with statistics
6. **Send invalid frame** (e.g., `FF 55 66 AA`) → `[LOG] Invalid frame: expected FF, got 0xAA`, error_drop_count increments
7. **Send partial frame then wait 500ms** → `[LOG] Protocol timeout: discarding partial frame`, timeout_drop_count increments
8. **Observe heartbeat** → PC13 LED toggles every 500ms
9. **Observe status** → Uptime printed every 3 seconds

## Changelog

### v0.3 (2026-04-28)
- Add `kt_system/kt_tick` — millisecond tick abstraction (`kt_tick_get_ms`, `kt_tick_is_timeout`)
- Add `kt_task` — cooperative periodic task scheduler (static allocation, 8 slots max)
- Refactor `main.c`: replace bare `kt_debug_task()` call with `kt_task_run()`
- Register three demo tasks: `debug` (1ms), `heartbeat` (500ms), `status` (3000ms)
- Heartbeat task toggles PC13 LED; status task prints uptime every 3 seconds
- Added `ktlib_system` and `ktlib_task` Keil groups and include paths in uvprojx

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