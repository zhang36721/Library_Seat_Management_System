#ifndef KT_BOOT_COUNT_H
#define KT_BOOT_COUNT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void kt_boot_count_init(void);
uint32_t kt_boot_count_get(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_BOOT_COUNT_H */
