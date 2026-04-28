#ifndef KT_TASK_H
#define KT_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Task function pointer type
 */
typedef void (*kt_task_func_t)(void);

/**
 * @brief  Task descriptor (static allocation, no malloc)
 */
typedef struct {
    const char      *name;          /**< Task name (for debug)        */
    kt_task_func_t   func;          /**< Task callback function       */
    uint32_t         period_ms;     /**< Execution period in ms       */
    uint32_t         last_run_ms;   /**< Last execution timestamp     */
    uint8_t          enabled;       /**< 1 = active, 0 = disabled     */
} kt_task_t;

/**
 * @brief  Max number of registered tasks
 */
#define KT_TASK_MAX  8

/**
 * @brief  Initialize task scheduler (clears all slots)
 */
void kt_task_init(void);

/**
 * @brief  Register a task
 * @param  name       Task name (debug only)
 * @param  func       Task callback (must not be NULL)
 * @param  period_ms  Execution period in ms (must be > 0)
 * @return 0 on success, -1 on failure (full, NULL func, or zero period)
 */
int kt_task_register(const char *name, kt_task_func_t func, uint32_t period_ms);

/**
 * @brief  Run all due tasks (call in main loop, does not block)
 */
void kt_task_run(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_TASK_H */