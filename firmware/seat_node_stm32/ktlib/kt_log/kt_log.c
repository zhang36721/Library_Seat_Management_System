#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "kt_config.h"
#include "kt_log.h"
#include "kt_port_uart.h"

/**
 * @brief Format and output a log message via USART2
 *
 * Implementation notes:
 *  - Uses two buffers: msg_buf for the user message, out_buf for the final line
 *  - vsnprintf ensures no buffer overflow (truncates silently)
 *  - Each log line is terminated with "\r\n" automatically
 *  - If KT_LOG_ENABLE == 0, the compiler will still compile this function
 *    but all KT_LOG_* macros expand to nothing, so it's never called.
 *    (Alternatively the level check at the top returns immediately.)
 */
void kt_log_output(kt_log_level_t level, const char *file, int line,
                   const char *fmt, ...)
{
    /* Quick bail-out if logging is completely disabled at compile time.
     * The KT_LOG_LEVEL check is still needed below for runtime filtering. */
#if !KT_LOG_ENABLE
    return;
#else
    /* Filter by configured log level */
    if (level < (kt_log_level_t)KT_LOG_LEVEL) {
        return;
    }

    if (fmt == NULL) {
        return;
    }

    char msg_buf[KT_LOG_BUFFER_SIZE];
    char out_buf[KT_LOG_BUFFER_SIZE + 64];

    /* Format user message */
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    /* Build final output line with prefix */
    switch (level) {
    case KT_LOG_LEVEL_DEBUG:
        snprintf(out_buf, sizeof(out_buf), "[DBG ] %s\r\n", msg_buf);
        break;

    case KT_LOG_LEVEL_INFO:
        snprintf(out_buf, sizeof(out_buf), "[INFO] %s\r\n", msg_buf);
        break;

    case KT_LOG_LEVEL_WARN:
        snprintf(out_buf, sizeof(out_buf), "[WARN] %s:%d | %s\r\n",
                 file, line, msg_buf);
        break;

    case KT_LOG_LEVEL_ERR:
        snprintf(out_buf, sizeof(out_buf), "[ERR ] %s:%d | %s\r\n",
                 file, line, msg_buf);
        break;

    default:
        /* Unknown level - silently ignore */
        return;
    }

    kt_port_uart_tx_string(out_buf);
#endif /* KT_LOG_ENABLE */
}