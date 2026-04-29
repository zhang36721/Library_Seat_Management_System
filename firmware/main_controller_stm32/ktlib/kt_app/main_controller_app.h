#ifndef MAIN_CONTROLLER_APP_H
#define MAIN_CONTROLLER_APP_H

#include "kt_modules/kt_ds1302.h"

#ifdef __cplusplus
extern "C" {
#endif

void main_controller_app_print_status(void);
void main_controller_app_run_card_flow(void);
void main_controller_app_show_home(void);
void main_controller_app_show_last_card(void);
void main_controller_app_beep_success(void);
void main_controller_app_beep_fail(void);
void main_controller_app_toggle_seat_state(void);
void main_controller_app_set_time(const kt_ds1302_time_t *time);
void main_controller_app_card_add_current(void);
void main_controller_app_card_del_current(void);
void main_controller_app_print_card_db(void);
void main_controller_app_clear_card_db(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_CONTROLLER_APP_H */
