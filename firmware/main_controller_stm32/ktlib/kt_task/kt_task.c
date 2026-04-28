#include "kt_task.h"
#include "kt_system/kt_tick.h"
#include <stddef.h>

/**
 * @brief  Static task table (no malloc)
 */
static kt_task_t g_kt_tasks[KT_TASK_MAX];
static uint8_t   g_kt_task_count = 0;

/**
 * @brief  Initialize task scheduler (clears all slots)
 */
void kt_task_init(void)
{
    uint8_t i;

    g_kt_task_count = 0;
    for (i = 0; i < KT_TASK_MAX; i++) {
        g_kt_tasks[i].name        = NULL;
        g_kt_tasks[i].func        = NULL;
        g_kt_tasks[i].period_ms   = 0;
        g_kt_tasks[i].last_run_ms = 0;
        g_kt_tasks[i].enabled     = 0;
    }
}

/**
 * @brief  Register a task
 * @param  name       Task name (debug only)
 * @param  func       Task callback (must not be NULL)
 * @param  period_ms  Execution period in ms (must be > 0)
 * @return 0 on success, -1 on failure
 */
int kt_task_register(const char *name, kt_task_func_t func, uint32_t period_ms)
{
    if (func == NULL || period_ms == 0) {
        return -1;
    }

    if (g_kt_task_count >= KT_TASK_MAX) {
        return -1;
    }

    g_kt_tasks[g_kt_task_count].name        = name;
    g_kt_tasks[g_kt_task_count].func        = func;
    g_kt_tasks[g_kt_task_count].period_ms   = period_ms;
    g_kt_tasks[g_kt_task_count].last_run_ms = kt_tick_get_ms();
    g_kt_tasks[g_kt_task_count].enabled     = 1;
    g_kt_task_count++;

    return 0;
}

/**
 * @brief  Run all due tasks (call in main loop, does not block)
 */
void kt_task_run(void)
{
    uint8_t i;
    uint32_t now_ms;

    for (i = 0; i < g_kt_task_count; i++) {
        if (!g_kt_tasks[i].enabled) {
            continue;
        }
        if (g_kt_tasks[i].func == NULL) {
            continue;
        }

        now_ms = kt_tick_get_ms();
        if (kt_tick_is_timeout(g_kt_tasks[i].last_run_ms, g_kt_tasks[i].period_ms)) {
            g_kt_tasks[i].last_run_ms = now_ms;
            g_kt_tasks[i].func();
        }
    }
}