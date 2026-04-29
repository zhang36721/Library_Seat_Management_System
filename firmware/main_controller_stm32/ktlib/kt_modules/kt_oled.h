#ifndef KT_OLED_H
#define KT_OLED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t kt_oled_init_startup(void);
void kt_oled_test(void);
void kt_oled_clear(void);
void kt_oled_print_line(uint8_t line, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* KT_OLED_H */
