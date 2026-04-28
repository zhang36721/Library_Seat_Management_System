#ifndef KT_DS1302_H
#define KT_DS1302_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} kt_ds1302_time_t;

void kt_ds1302_write_test_time(void);
void kt_ds1302_read_time(kt_ds1302_time_t *time);
void kt_ds1302_print_time(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_DS1302_H */
