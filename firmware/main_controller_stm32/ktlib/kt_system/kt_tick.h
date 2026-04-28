#ifndef KT_TICK_H
#define KT_TICK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Get system tick in milliseconds
 * @return Current tick count (ms)
 */
uint32_t kt_tick_get_ms(void);

/**
 * @brief  Check if a period has elapsed since a start time
 * @param  start_ms   Start timestamp (from kt_tick_get_ms)
 * @param  period_ms  Timeout period in milliseconds
 * @return 1 if timeout, 0 otherwise
 */
uint8_t kt_tick_is_timeout(uint32_t start_ms, uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif /* KT_TICK_H */