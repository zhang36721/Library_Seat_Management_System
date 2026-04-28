#ifndef KT_HW_DIAG_H
#define KT_HW_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

void kt_hw_diag_print_uart_roles(void);
void kt_hw_diag_print_debug_status(void);
void kt_hw_diag_print_gpio_init_status(void);
void kt_hw_diag_print_hardware_resources(void);
void kt_hw_diag_read_button_state(void);
void kt_hw_diag_print_seat_raw_levels(void);
void kt_hw_diag_print_seat_occupied_status(void);

#ifdef __cplusplus
}
#endif

#endif /* KT_HW_DIAG_H */
