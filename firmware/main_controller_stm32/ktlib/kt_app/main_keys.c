#include "main_keys.h"
#include "main_controller_app.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_modules/kt_oled.h"
#include "kt_system/kt_tick.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

typedef enum {
    MENU_SCREEN_HOME = 0,
    MENU_SCREEN_MAIN,
    MENU_SCREEN_TIME_SET,
    MENU_SCREEN_CARD_ADD,
    MENU_SCREEN_CARD_DEL
} menu_screen_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    const char *name;
    const char *func;
    uint8_t raw_pressed;
    uint8_t debounced_pressed;
    uint8_t last_raw_pressed;
    uint8_t long_reported;
    uint32_t last_change_ms;
    uint32_t press_start_ms;
} main_key_state_t;

static main_key_state_t keys[8] = {
    {MAIN_KEY1_PORT, MAIN_KEY1_PIN, "K1", "MENU", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY2_PORT, MAIN_KEY2_PIN, "K2", "UP", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY3_PORT, MAIN_KEY3_PIN, "K3", "DOWN", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY4_PORT, MAIN_KEY4_PIN, "K4", "LEFT", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY5_PORT, MAIN_KEY5_PIN, "K5", "RIGHT", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY6_PORT, MAIN_KEY6_PIN, "K6", "OK", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY7_PORT, MAIN_KEY7_PIN, "K7", "CARD ADD", 0, 0, 0, 0, 0, 0},
    {MAIN_KEY8_PORT, MAIN_KEY8_PIN, "K8", "CARD DEL", 0, 0, 0, 0, 0, 0}
};

static main_key_id_t last_key = MAIN_KEY_NONE;
static main_key_event_type_t last_event = MAIN_KEY_EVENT_NONE;
static menu_screen_t current_screen = MENU_SCREEN_HOME;
static uint8_t menu_index = 0;
static uint8_t time_field = 0;
static uint8_t edit_time[6] = {26U, 4U, 29U, 11U, 28U, 0U};

static uint8_t key_index(main_key_id_t key)
{
    return (uint8_t)key - 1U;
}

static uint8_t read_pressed(main_key_state_t *key)
{
    GPIO_PinState raw = HAL_GPIO_ReadPin(key->port, key->pin);
    return (raw == MAIN_KEY_ACTIVE_LEVEL) ? 1U : 0U;
}

static void show_main_menu(void)
{
    static const char *items[4] = {
        "CARD TEST",
        "TIME SET",
        "CARD ADD",
        "CARD DEL"
    };
    char line[18];
    uint8_t i;

    current_screen = MENU_SCREEN_MAIN;
    kt_oled_clear();
    for (i = 0; i < 4U; i++) {
        snprintf(line, sizeof(line), "%c %s", (i == menu_index) ? '>' : ' ', items[i]);
        kt_oled_print_line(i, line);
    }
}

static void show_time_set(void)
{
    static const char *fields[6] = {"YY", "MM", "DD", "HH", "MI", "SS"};
    char line[18];

    current_screen = MENU_SCREEN_TIME_SET;
    kt_oled_clear();
    kt_oled_print_line(0, "TIME SET");
    snprintf(line, sizeof(line), "%s %02u", fields[time_field], (unsigned int)edit_time[time_field]);
    kt_oled_print_line(1, line);
    kt_oled_print_line(2, "K4 LEFT K5 RIGHT");
    kt_oled_print_line(3, "K2 PLUS K3 MINUS");
}

static uint8_t time_field_min(uint8_t field)
{
    return (field == 1U || field == 2U) ? 1U : 0U;
}

static uint8_t time_field_max(uint8_t field)
{
    static const uint8_t max_values[6] = {99U, 12U, 31U, 23U, 59U, 59U};
    return max_values[field];
}

static void adjust_time_field(uint8_t plus)
{
    uint8_t min_value = time_field_min(time_field);
    uint8_t max_value = time_field_max(time_field);

    if (plus) {
        edit_time[time_field] = (edit_time[time_field] >= max_value) ? min_value : (uint8_t)(edit_time[time_field] + 1U);
    } else {
        edit_time[time_field] = (edit_time[time_field] <= min_value) ? max_value : (uint8_t)(edit_time[time_field] - 1U);
    }
    KT_LOG_INFO("Time Set field %u value %02u",
                (unsigned int)(time_field + 1U),
                (unsigned int)edit_time[time_field]);
    show_time_set();
}

static void show_card_add(void)
{
    current_screen = MENU_SCREEN_CARD_ADD;
    kt_oled_clear();
    kt_oled_print_line(0, "CARD ADD");
    kt_oled_print_line(1, "PLACE CARD");
    kt_oled_print_line(2, "K6 CONFIRM");
    KT_LOG_INFO("Menu: Card Add");
}

static void show_card_del(void)
{
    current_screen = MENU_SCREEN_CARD_DEL;
    kt_oled_clear();
    kt_oled_print_line(0, "CARD DEL");
    kt_oled_print_line(1, "PLACE CARD");
    kt_oled_print_line(2, "K6 CONFIRM");
    KT_LOG_INFO("Menu: Card Del");
}

static void handle_ok(void)
{
    kt_ds1302_time_t time;

    if (current_screen == MENU_SCREEN_MAIN) {
        switch (menu_index) {
        case 0:
            KT_LOG_INFO("Menu: Card Test");
            main_controller_app_run_card_flow();
            break;
        case 1:
            main_keys_enter_time_set();
            break;
        case 2:
            main_keys_enter_card_add();
            break;
        default:
            main_keys_enter_card_del();
            break;
        }
    } else if (current_screen == MENU_SCREEN_TIME_SET) {
        time.year = edit_time[0];
        time.month = edit_time[1];
        time.day = edit_time[2];
        time.hour = edit_time[3];
        time.minute = edit_time[4];
        time.second = edit_time[5];
        main_controller_app_set_time(&time);
    } else if (current_screen == MENU_SCREEN_CARD_ADD) {
        main_controller_app_card_add_current();
    } else if (current_screen == MENU_SCREEN_CARD_DEL) {
        main_controller_app_card_del_current();
    }
}

static void handle_key_event(main_key_id_t key, main_key_event_type_t event)
{
    last_key = key;
    last_event = event;
    KT_LOG_INFO("Last key: %s %s", main_key_name(key), main_key_event_name(event));

    if (event == MAIN_KEY_EVENT_LONG && key == MAIN_KEY_1) {
        current_screen = MENU_SCREEN_HOME;
        main_controller_app_show_home();
        return;
    }

    if (event != MAIN_KEY_EVENT_SHORT) {
        return;
    }

    switch (key) {
    case MAIN_KEY_1:
        menu_index = 0;
        show_main_menu();
        break;
    case MAIN_KEY_2:
        if (current_screen == MENU_SCREEN_MAIN && menu_index > 0U) {
            menu_index--;
            show_main_menu();
        } else if (current_screen == MENU_SCREEN_TIME_SET) {
            adjust_time_field(1U);
        }
        break;
    case MAIN_KEY_3:
        if (current_screen == MENU_SCREEN_MAIN && menu_index < 3U) {
            menu_index++;
            show_main_menu();
        } else if (current_screen == MENU_SCREEN_TIME_SET) {
            adjust_time_field(0U);
        }
        break;
    case MAIN_KEY_4:
        if (current_screen == MENU_SCREEN_TIME_SET) {
            time_field = (time_field == 0U) ? 5U : (uint8_t)(time_field - 1U);
            show_time_set();
        }
        break;
    case MAIN_KEY_5:
        if (current_screen == MENU_SCREEN_TIME_SET) {
            time_field = (uint8_t)((time_field + 1U) % 6U);
            show_time_set();
        }
        break;
    case MAIN_KEY_6:
        handle_ok();
        break;
    case MAIN_KEY_7:
        main_keys_enter_card_add();
        break;
    case MAIN_KEY_8:
        main_keys_enter_card_del();
        break;
    default:
        break;
    }
}

void main_keys_init(void)
{
    uint8_t i;
    uint32_t now = kt_tick_get_ms();

    for (i = 0; i < 8U; i++) {
        keys[i].raw_pressed = read_pressed(&keys[i]);
        keys[i].last_raw_pressed = keys[i].raw_pressed;
        keys[i].debounced_pressed = keys[i].raw_pressed;
        keys[i].last_change_ms = now;
        keys[i].press_start_ms = now;
        keys[i].long_reported = 0U;
    }

    last_key = MAIN_KEY_NONE;
    last_event = MAIN_KEY_EVENT_NONE;
    KT_LOG_INFO("Main 8-key module init: OK");
}

void main_keys_task(void)
{
    uint8_t i;
    uint8_t raw_pressed;
    uint32_t now = kt_tick_get_ms();

    for (i = 0; i < 8U; i++) {
        raw_pressed = read_pressed(&keys[i]);
        keys[i].raw_pressed = raw_pressed;

        if (raw_pressed != keys[i].last_raw_pressed) {
            keys[i].last_raw_pressed = raw_pressed;
            keys[i].last_change_ms = now;
        }

        if (kt_tick_is_timeout(keys[i].last_change_ms, MAIN_KEY_DEBOUNCE_MS) &&
            raw_pressed != keys[i].debounced_pressed) {
            keys[i].debounced_pressed = raw_pressed;
            if (raw_pressed) {
                keys[i].press_start_ms = now;
                keys[i].long_reported = 0U;
            } else if (!keys[i].long_reported) {
                handle_key_event((main_key_id_t)(i + 1U), MAIN_KEY_EVENT_SHORT);
            }
        }

        if (keys[i].debounced_pressed && !keys[i].long_reported &&
            kt_tick_is_timeout(keys[i].press_start_ms, MAIN_KEY_LONG_PRESS_MS)) {
            keys[i].long_reported = 1U;
            handle_key_event((main_key_id_t)(i + 1U), MAIN_KEY_EVENT_LONG);
        }
    }
}

void main_keys_print_mapping(void)
{
    KT_LOG_INFO("8 independent keys, input pull-up, active-low");
    KT_LOG_INFO("K1 PA1  MENU / long back");
    KT_LOG_INFO("K2 PA8  UP / plus");
    KT_LOG_INFO("K3 PA11 DOWN / minus");
    KT_LOG_INFO("K4 PA12 LEFT / previous field");
    KT_LOG_INFO("K5 PA15 RIGHT / next field");
    KT_LOG_INFO("K6 PB3  OK / confirm");
    KT_LOG_INFO("K7 PB4  CARD ADD");
    KT_LOG_INFO("K8 PC14 CARD DEL");
}

void main_keys_print_raw_levels(void)
{
    uint8_t i;
    GPIO_PinState raw;
    uint8_t pressed;

    for (i = 0; i < 8U; i++) {
        raw = HAL_GPIO_ReadPin(keys[i].port, keys[i].pin);
        pressed = (raw == MAIN_KEY_ACTIVE_LEVEL) ? 1U : 0U;
        KT_LOG_INFO("%s raw=%u pressed=%u",
                    keys[i].name,
                    (raw == GPIO_PIN_SET) ? 1U : 0U,
                    pressed);
    }
}

void main_keys_print_last_event(void)
{
    KT_LOG_INFO("Last key: %s %s", main_key_name(last_key), main_key_event_name(last_event));
}

void main_keys_enter_time_set(void)
{
    time_field = 0;
    show_time_set();
    KT_LOG_INFO("Menu: Time Set");
}

void main_keys_enter_card_add(void)
{
    show_card_add();
}

void main_keys_enter_card_del(void)
{
    show_card_del();
}

uint8_t main_keys_is_home(void)
{
    return (current_screen == MENU_SCREEN_HOME) ? 1U : 0U;
}

const char *main_key_name(main_key_id_t key)
{
    if (key >= MAIN_KEY_1 && key <= MAIN_KEY_8) {
        return keys[key_index(key)].name;
    }
    return "NONE";
}

const char *main_key_event_name(main_key_event_type_t event)
{
    switch (event) {
    case MAIN_KEY_EVENT_SHORT:
        return "SHORT";
    case MAIN_KEY_EVENT_LONG:
        return "LONG";
    default:
        return "NONE";
    }
}
