#ifndef KT_STEPPER_H
#define KT_STEPPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void kt_stepper_forward_test(void);
void kt_stepper_reverse_test(void);
void kt_stepper_start_forward(uint16_t steps);
void kt_stepper_start_reverse(uint16_t steps);
void kt_stepper_task(void);
uint8_t kt_stepper_is_busy(void);
void kt_stepper_stop(void);
void kt_stepper_init(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_STEPPER_H */
