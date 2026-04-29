#ifndef MAIN_KEYS_H
#define MAIN_KEYS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAIN_KEY_NONE = 0,
    MAIN_KEY_1,
    MAIN_KEY_2,
    MAIN_KEY_3,
    MAIN_KEY_4,
    MAIN_KEY_5,
    MAIN_KEY_6,
    MAIN_KEY_7,
    MAIN_KEY_8
} main_key_id_t;

typedef enum {
    MAIN_KEY_EVENT_NONE = 0,
    MAIN_KEY_EVENT_SHORT,
    MAIN_KEY_EVENT_LONG
} main_key_event_type_t;

void main_keys_init(void);
void main_keys_task(void);
void main_keys_print_mapping(void);
void main_keys_print_raw_levels(void);
void main_keys_print_last_event(void);
void main_keys_enter_time_set(void);
void main_keys_enter_card_add(void);
void main_keys_enter_card_del(void);
const char *main_key_name(main_key_id_t key);
const char *main_key_event_name(main_key_event_type_t event);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_KEYS_H */
