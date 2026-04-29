#ifndef MAIN_CONTROLLER_APP_H
#define MAIN_CONTROLLER_APP_H

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

#ifdef __cplusplus
}
#endif

#endif /* MAIN_CONTROLLER_APP_H */
