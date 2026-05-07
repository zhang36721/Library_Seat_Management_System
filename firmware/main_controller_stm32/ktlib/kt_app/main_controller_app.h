#ifndef MAIN_CONTROLLER_APP_H
#define MAIN_CONTROLLER_APP_H

#include "kt_modules/kt_ds1302.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void main_controller_app_print_status(void);
void main_controller_app_init(void);
void main_controller_app_task(void);
void main_controller_app_run_card_flow(void);
void main_controller_app_show_home(void);
void main_controller_app_show_last_card(void);
void main_controller_app_beep_success(void);
void main_controller_app_beep_fail(void);
void main_controller_app_toggle_seat_state(void);
uint8_t main_controller_app_update_seat_states(uint8_t seat1, uint8_t seat2, uint8_t seat3);
void main_controller_app_set_time(const kt_ds1302_time_t *time);
void main_controller_app_card_add_current(void);
void main_controller_app_card_del_current(void);
void main_controller_app_print_card_db(void);
void main_controller_app_clear_card_db(void);
void main_controller_app_print_access_log(void);
void main_controller_app_clear_access_log(void);
void main_controller_app_print_access_stats(void);
void main_controller_app_send_last_card_event_to_esp32(void);
uint8_t main_controller_app_get_seat_state(uint8_t index);
uint8_t main_controller_app_get_gate_state(void);
uint8_t main_controller_app_get_last_card_result(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_CONTROLLER_APP_H */
