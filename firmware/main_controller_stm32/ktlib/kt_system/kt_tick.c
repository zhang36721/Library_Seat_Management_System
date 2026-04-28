#include "kt_tick.h"
#include "main.h"

/**
 * @brief  Get system tick in milliseconds
 * @return Current tick count (ms)
 */
uint32_t kt_tick_get_ms(void)
{
    return HAL_GetTick();
}

/**
 * @brief  Check if a period has elapsed since a start time
 * @param  start_ms   Start timestamp (from kt_tick_get_ms)
 * @param  period_ms  Timeout period in milliseconds
 * @return 1 if timeout, 0 otherwise
 *
 * @note   Handles uint32_t wrap-around correctly.
 */
uint8_t kt_tick_is_timeout(uint32_t start_ms, uint32_t period_ms)
{
    uint32_t now = HAL_GetTick();
    return ((now - start_ms) >= period_ms) ? 1 : 0;
}