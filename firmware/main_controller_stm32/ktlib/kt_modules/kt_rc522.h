#ifndef KT_RC522_H
#define KT_RC522_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t kt_rc522_init(void);
uint8_t kt_rc522_read_uid(uint8_t uid[5]);
uint8_t kt_rc522_read_uid_quiet(uint8_t uid[5]);

#ifdef __cplusplus
}
#endif

#endif /* KT_RC522_H */
