#ifndef KT_LOG_H
#define KT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Log Level Enumeration
 *===========================================================================*/
typedef enum
{
    KT_LOG_LEVEL_DEBUG = 0,
    KT_LOG_LEVEL_INFO  = 1,
    KT_LOG_LEVEL_WARN  = 2,
    KT_LOG_LEVEL_ERR   = 3,
    KT_LOG_LEVEL_NONE  = 4
} kt_log_level_t;

/*===========================================================================
 * Log Configuration (expected from kt_config.h)
 *
 *   KT_LOG_ENABLE       1 = enabled  / 0 = disabled (all macros become no-ops)
 *   KT_LOG_LEVEL        One of the kt_log_level_t values above
 *   KT_LOG_BUFFER_SIZE  Size of internal format buffer (recommended: 128)
 *
 * If not defined by kt_config.h, reasonable fallbacks are provided below.
 *===========================================================================*/
#ifndef KT_LOG_ENABLE
#define KT_LOG_ENABLE       1
#endif

#ifndef KT_LOG_LEVEL
#define KT_LOG_LEVEL        KT_LOG_LEVEL_DEBUG
#endif

#ifndef KT_LOG_BUFFER_SIZE
#define KT_LOG_BUFFER_SIZE  128
#endif

/*===========================================================================
 * Core function
 *===========================================================================*/

/**
 * @brief Format and output a log message via USART2
 *
 * @param level  Log level (KT_LOG_LEVEL_DEBUG / INFO / WARN / ERR)
 * @param file   Source file name (__FILE__), or NULL for INFO/DEBUG
 * @param line   Source line number (__LINE__), or 0 for INFO/DEBUG
 * @param fmt    printf-style format string (must not be NULL)
 * @param ...    Format arguments
 *
 * @note  Each message is automatically terminated with "\r\n".
 *        Messages exceeding KT_LOG_BUFFER_SIZE are truncated safely.
 */
void kt_log_output(kt_log_level_t level, const char *file, int line,
                   const char *fmt, ...);

/*===========================================================================
 * User-facing macros
 *
 *  - INFO / DEBUG  :  file and line are NOT included
 *  - WARN / ERR    :  file and line ARE included (__FILE__, __LINE__)
 *
 *  When KT_LOG_ENABLE == 0, all macros expand to nothing.
 *
 *  Keil AC6 (armclang) supports ##__VA_ARGS__ (GNU extension).
 *  The "fmt, ##__VA_ARGS__" pattern also handles the zero-argument
 *  case correctly (e.g. KT_LOG_INFO("hello") compiles fine).
 *===========================================================================*/

#if KT_LOG_ENABLE

#define KT_LOG_DEBUG(fmt, ...) \
    kt_log_output(KT_LOG_LEVEL_DEBUG, NULL, 0, fmt, ##__VA_ARGS__)

#define KT_LOG_INFO(fmt, ...) \
    kt_log_output(KT_LOG_LEVEL_INFO, NULL, 0, fmt, ##__VA_ARGS__)

#define KT_LOG_WARN(fmt, ...) \
    kt_log_output(KT_LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KT_LOG_ERR(fmt, ...) \
    kt_log_output(KT_LOG_LEVEL_ERR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else  /* KT_LOG_ENABLE == 0 */

#define KT_LOG_DEBUG(fmt, ...)
#define KT_LOG_INFO(fmt, ...)
#define KT_LOG_WARN(fmt, ...)
#define KT_LOG_ERR(fmt, ...)

#endif /* KT_LOG_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* KT_LOG_H */