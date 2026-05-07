#ifndef KT_SYSTEM_HEALTH_H
#define KT_SYSTEM_HEALTH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void kt_system_health_init(void);
void kt_system_health_note_main_loop(void);
void kt_system_health_note_main_app_task(void);
void kt_system_health_note_esp32_task(void);
void kt_system_health_note_zigbee_task(void);
void kt_system_health_note_flash_save(uint32_t cost_ms);
void kt_system_health_note_card_flow(uint32_t cost_ms);
void kt_system_health_note_fault(const char *name);
void kt_system_health_print(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_SYSTEM_HEALTH_H */
