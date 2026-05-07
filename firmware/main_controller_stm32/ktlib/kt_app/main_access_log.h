#ifndef MAIN_ACCESS_LOG_H
#define MAIN_ACCESS_LOG_H

#include "kt_modules/kt_ds1302.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAIN_ACCESS_CHECK_IN = 0,
    MAIN_ACCESS_CHECK_OUT = 1,
    MAIN_ACCESS_DENIED = 2
} main_access_type_t;

typedef struct {
    uint32_t index;
    uint8_t uid[4];
    main_access_type_t type;
    uint8_t allowed;
    kt_ds1302_time_t time;
} main_access_record_t;

void main_access_log_add(const uint8_t uid[4],
                         main_access_type_t type,
                         uint8_t allowed,
                         const kt_ds1302_time_t *time);
void main_access_log_init(void);
void main_access_log_task(uint8_t allow_flash_write);
void main_access_log_flush(void);
void main_access_log_clear(void);
void main_access_log_print_all(void);
void main_access_log_print_stats(void);
uint8_t main_access_log_count(void);
main_access_type_t main_access_log_next_type_for_uid(const uint8_t uid[4]);
const char *main_access_log_type_text(main_access_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_ACCESS_LOG_H */
