#include "main_controller_app.h"
#include "app_io.h"
#include "main_access_log.h"
#include "main_card_db.h"
#include "main_keys.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_modules/kt_rc522.h"
#include "kt_modules/kt_oled.h"
#include "kt_modules/kt_ds1302.h"
#include "kt_modules/kt_stepper.h"
#include "kt_modules/kt_esp32_link.h"
#include "kt_system/kt_tick.h"
#include <stdio.h>
#include <string.h>

typedef enum
{
    CARD_EVENT_NONE = 0,
    CARD_EVENT_OK,
    CARD_EVENT_NO_CARD,
    CARD_EVENT_BAD_TIME,
    CARD_EVENT_DENIED
} card_event_t;

typedef enum
{
    GATE_STATE_IDLE = 0,
    GATE_STATE_OPEN,
    GATE_STATE_HOLD,
    GATE_STATE_CLOSE
} gate_state_t;

static uint8_t seat_occupied[3] = {0U, 1U, 0U};
static card_event_t last_card_event = CARD_EVENT_NONE;
static main_access_type_t last_access_type = MAIN_ACCESS_CHECK_IN;
static uint8_t last_uid[5] = {0};
static kt_ds1302_time_t last_time = {0};
static uint8_t card_wait_remove;
static uint32_t last_card_ms;
static uint8_t last_auto_uid[4];
static uint8_t result_screen_active;
static uint32_t result_screen_ms;
static gate_state_t gate_state = GATE_STATE_IDLE;
static uint32_t gate_hold_start_ms;

static const char *seat_text(uint8_t occupied)
{
    return occupied ? "OCC" : "FREE";
}

static void uid_to_text(const uint8_t uid[5], char *out, uint8_t out_size)
{
    if (out == 0 || out_size < 9U) {
        return;
    }
    snprintf(out, out_size, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
}

static void log_uid4(const char *prefix, const uint8_t uid[4])
{
    KT_LOG_INFO("%s: %02X %02X %02X %02X", prefix, uid[0], uid[1], uid[2], uid[3]);
}

static void time_to_hm_text(const kt_ds1302_time_t *time, char *out, uint8_t out_size)
{
    if (out == 0 || out_size < 6U || time == 0) {
        return;
    }
    snprintf(out, out_size, "%02u:%02u",
             (unsigned int)time->hour,
             (unsigned int)time->minute);
}

static void time_to_hms_text(const kt_ds1302_time_t *time, char *out, uint8_t out_size)
{
    if (out == 0 || out_size < 9U || time == 0) {
        return;
    }
    snprintf(out, out_size, "%02u:%02u:%02u",
             (unsigned int)time->hour,
             (unsigned int)time->minute,
             (unsigned int)time->second);
}

static uint8_t uid4_equal(const uint8_t a[4], const uint8_t b[4])
{
    return (memcmp(a, b, 4U) == 0) ? 1U : 0U;
}

static void read_valid_time_or_zero(kt_ds1302_time_t *time)
{
    if (time == 0) {
        return;
    }

    kt_ds1302_read_time(time);
    if (!kt_ds1302_time_is_valid(time)) {
        memset(time, 0, sizeof(*time));
    }
}

static void oled_show_card_fail(const char *reason)
{
    kt_oled_clear();
    kt_oled_print_line(0, "CARD FAIL");
    kt_oled_print_line(1, reason);
}

static void oled_show_three_lines(const char *line0, const char *line1, const char *line2)
{
    kt_oled_clear();
    kt_oled_print_line(0, line0);
    kt_oled_print_line(1, line1);
    if (line2 != 0) {
        kt_oled_print_line(2, line2);
    }
}

static void mark_result_screen(void)
{
    result_screen_active = 1U;
    result_screen_ms = kt_tick_get_ms();
}

static void oled_show_card_ok(const uint8_t uid[5],
                              const kt_ds1302_time_t *time,
                              main_access_type_t type)
{
    char uid_text[12];
    char time_text[12];

    uid_to_text(uid, uid_text, sizeof(uid_text));
    time_to_hms_text(time, time_text, sizeof(time_text));

    kt_oled_clear();
    kt_oled_print_line(0, "ACCESS OK");
    kt_oled_print_line(1, uid_text);
    kt_oled_print_line(2, type == MAIN_ACCESS_CHECK_OUT ? "CHECK OUT" : "CHECK IN");
    kt_oled_print_line(3, time_text);
    mark_result_screen();
}

static void oled_show_access_denied(const uint8_t uid[5])
{
    char uid_text[12];

    uid_to_text(uid, uid_text, sizeof(uid_text));
    kt_oled_clear();
    kt_oled_print_line(0, "ACCESS DENIED");
    kt_oled_print_line(1, uid_text);
    mark_result_screen();
}

static void start_gate_cycle(void)
{
    gate_state = GATE_STATE_OPEN;
    KT_LOG_INFO("Gate cycle: OPEN");
}

static void process_gate_task(void)
{
    if (gate_state == GATE_STATE_OPEN) {
        kt_stepper_forward_test();
        gate_hold_start_ms = kt_tick_get_ms();
        gate_state = GATE_STATE_HOLD;
        KT_LOG_INFO("Gate cycle: HOLD %u ms", (unsigned int)MAIN_GATE_HOLD_MS);
    } else if (gate_state == GATE_STATE_HOLD) {
        if (kt_tick_is_timeout(gate_hold_start_ms, MAIN_GATE_HOLD_MS)) {
            gate_state = GATE_STATE_CLOSE;
            KT_LOG_INFO("Gate cycle: CLOSE");
        }
    } else if (gate_state == GATE_STATE_CLOSE) {
        kt_stepper_reverse_test();
        kt_stepper_stop();
        gate_state = GATE_STATE_IDLE;
        KT_LOG_INFO("Gate cycle: STOP");
    }
}

static void process_card_uid(const uint8_t uid[5], uint8_t start_gate)
{
    kt_ds1302_time_t now;
    main_access_type_t access_type;

    memcpy(last_uid, uid, sizeof(last_uid));
    read_valid_time_or_zero(&now);
    last_time = now;

    if (!main_card_db_contains(uid)) {
        last_card_event = CARD_EVENT_DENIED;
        main_access_log_add(uid, MAIN_ACCESS_DENIED, 0U, &now);
        kt_esp32_link_send_card_event(uid, MAIN_ACCESS_DENIED, 0U, &now);
        oled_show_access_denied(uid);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD EVENT: DENIED UID=%02X %02X %02X %02X",
                    uid[0], uid[1], uid[2], uid[3]);
        return;
    }

    if (!kt_ds1302_time_is_valid(&now)) {
        last_card_event = CARD_EVENT_BAD_TIME;
        oled_show_card_fail("BAD TIME");
        mark_result_screen();
        main_controller_app_beep_fail();
        KT_LOG_INFO("CARD UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
        KT_LOG_WARN("CARD EVENT: INVALID_TIME");
        return;
    }

    access_type = main_access_log_next_type_for_uid(uid);
    main_access_log_add(uid, access_type, 1U, &now);
    last_card_event = CARD_EVENT_OK;
    last_access_type = access_type;
    kt_esp32_link_send_card_event(uid, (uint8_t)access_type, 1U, &now);

    oled_show_card_ok(uid, &now, access_type);
    main_controller_app_beep_success();

    KT_LOG_INFO("TIME: 20%02u-%02u-%02u %02u:%02u:%02u",
                (unsigned int)now.year,
                (unsigned int)now.month,
                (unsigned int)now.day,
                (unsigned int)now.hour,
                (unsigned int)now.minute,
                (unsigned int)now.second);
    KT_LOG_INFO("CARD EVENT: %s OK UID=%02X %02X %02X %02X",
                main_access_log_type_text(access_type),
                uid[0], uid[1], uid[2], uid[3]);

    if (start_gate) {
        start_gate_cycle();
    }
}

void main_controller_app_print_status(void)
{
    KT_LOG_INFO("Main app: v0.8.5 local access terminal");
    KT_LOG_INFO("Seats: S1 %s, S2 %s, S3 %s",
                seat_text(seat_occupied[0]),
                seat_text(seat_occupied[1]),
                seat_text(seat_occupied[2]));
    KT_LOG_INFO("Cards: %u/%u, Logs: %u/%u",
                (unsigned int)main_card_db_count(),
                (unsigned int)MAIN_CARD_MAX_COUNT,
                (unsigned int)main_access_log_count(),
                (unsigned int)MAIN_ACCESS_LOG_MAX_COUNT);

    if (last_card_event == CARD_EVENT_OK) {
        KT_LOG_INFO("Last card: OK UID=%02X %02X %02X %02X",
                    last_uid[0], last_uid[1], last_uid[2], last_uid[3]);
    } else if (last_card_event == CARD_EVENT_NO_CARD) {
        KT_LOG_WARN("Last card: NO_CARD");
    } else if (last_card_event == CARD_EVENT_BAD_TIME) {
        KT_LOG_WARN("Last card: INVALID_TIME");
    } else if (last_card_event == CARD_EVENT_DENIED) {
        KT_LOG_WARN("Last card: DENIED UID=%02X %02X %02X %02X",
                    last_uid[0], last_uid[1], last_uid[2], last_uid[3]);
    } else {
        KT_LOG_INFO("Last card: none");
    }
}

void main_controller_app_run_card_flow(void)
{
    uint8_t uid[5];

    if (!kt_rc522_read_uid(uid)) {
        last_card_event = CARD_EVENT_NO_CARD;
        oled_show_card_fail("NO CARD");
        mark_result_screen();
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD EVENT: NO_CARD");
        return;
    }

    process_card_uid(uid, 1U);
}

void main_controller_app_show_home(void)
{
    kt_ds1302_time_t now;
    char line[18];
    char hm[8] = "--:--";

    kt_ds1302_read_time(&now);
    if (kt_ds1302_time_is_valid(&now)) {
        time_to_hm_text(&now, hm, sizeof(hm));
    }

    kt_oled_clear();
    kt_oled_print_line(0, "KENTO LIBRARY");
    kt_oled_print_line(1, "SWIPE CARD");
    snprintf(line, sizeof(line), "CARDS %02u", (unsigned int)main_card_db_count());
    kt_oled_print_line(2, line);
    snprintf(line, sizeof(line), "LOGS %02u %s",
             (unsigned int)main_access_log_count(), hm);
    kt_oled_print_line(3, line);
    result_screen_active = 0U;

    KT_LOG_INFO("OLED home shown");
}

void main_controller_app_init(void)
{
    main_access_log_init();
    card_wait_remove = 0U;
    gate_state = GATE_STATE_IDLE;
    result_screen_active = 0U;
    main_controller_app_show_home();
}

void main_controller_app_task(void)
{
    uint8_t uid[5];
    uint32_t now = kt_tick_get_ms();

    process_gate_task();

    if (result_screen_active && gate_state == GATE_STATE_IDLE &&
        kt_tick_is_timeout(result_screen_ms, MAIN_OLED_IDLE_HOME_MS)) {
        main_controller_app_show_home();
    }

    if (gate_state != GATE_STATE_IDLE) {
        return;
    }

    if (!main_keys_is_home()) {
        card_wait_remove = 0U;
        return;
    }

    if (kt_rc522_read_uid_quiet(uid)) {
        if (card_wait_remove) {
            return;
        }

        if (uid4_equal(uid, last_auto_uid) &&
            !kt_tick_is_timeout(last_card_ms, MAIN_CARD_REPEAT_GUARD_MS)) {
            card_wait_remove = 1U;
            return;
        }

        memcpy(last_auto_uid, uid, sizeof(last_auto_uid));
        last_card_ms = now;
        card_wait_remove = 1U;
        KT_LOG_INFO("CARD POLL: UID=%02X %02X %02X %02X",
                    uid[0], uid[1], uid[2], uid[3]);
        process_card_uid(uid, 1U);
    } else {
        card_wait_remove = 0U;
    }
}

void main_controller_app_show_last_card(void)
{
    if (last_card_event == CARD_EVENT_OK) {
        oled_show_card_ok(last_uid, &last_time, last_access_type);
        KT_LOG_INFO("OLED last card result shown");
    } else if (last_card_event == CARD_EVENT_NO_CARD) {
        oled_show_card_fail("NO CARD");
        KT_LOG_INFO("OLED last card failure shown");
    } else if (last_card_event == CARD_EVENT_BAD_TIME) {
        oled_show_card_fail("BAD TIME");
        KT_LOG_INFO("OLED last card bad time shown");
    } else if (last_card_event == CARD_EVENT_DENIED) {
        char uid_text[12];
        uid_to_text(last_uid, uid_text, sizeof(uid_text));
        oled_show_three_lines("CARD DENIED", uid_text, 0);
        KT_LOG_INFO("OLED last card denied shown");
    } else {
        kt_oled_clear();
        kt_oled_print_line(0, "CARD EVENT");
        kt_oled_print_line(1, "NONE");
        KT_LOG_INFO("OLED last card empty shown");
    }
}

void main_controller_app_set_time(const kt_ds1302_time_t *time)
{
    char date_text[18];
    char time_text[12];

    if (time == 0 || !kt_ds1302_time_is_valid(time)) {
        oled_show_three_lines("TIME FAIL", "INVALID", 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("TIME SET: INVALID_TIME");
        return;
    }

    KT_LOG_INFO("TIME SET: 20%02u-%02u-%02u %02u:%02u:%02u",
                (unsigned int)time->year,
                (unsigned int)time->month,
                (unsigned int)time->day,
                (unsigned int)time->hour,
                (unsigned int)time->minute,
                (unsigned int)time->second);

    if (kt_ds1302_set_time(time)) {
        snprintf(date_text, sizeof(date_text), "20%02u-%02u-%02u",
                 (unsigned int)time->year,
                 (unsigned int)time->month,
                 (unsigned int)time->day);
        time_to_hms_text(time, time_text, sizeof(time_text));
        oled_show_three_lines("TIME SAVED", date_text, time_text);
        main_controller_app_beep_success();
        KT_LOG_INFO("TIME SET: OK");
    } else {
        oled_show_three_lines("TIME FAIL", "INVALID", 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("TIME SET: INVALID_TIME");
    }
}

void main_controller_app_card_add_current(void)
{
    uint8_t uid[5];
    char uid_text[12];
    main_card_add_result_t result;

    if (!kt_rc522_read_uid(uid)) {
        oled_show_three_lines("CARD ADD", "NO CARD", 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD ADD: NO_CARD");
        return;
    }

    uid_to_text(uid, uid_text, sizeof(uid_text));
    log_uid4("CARD ADD", uid);
    result = main_card_db_add(uid);

    if (result == MAIN_CARD_ADD_OK) {
        main_access_log_flush();
        oled_show_three_lines("CARD ADD", "OK", uid_text);
        main_controller_app_beep_success();
        KT_LOG_INFO("CARD ADD: OK");
    } else if (result == MAIN_CARD_ADD_EXISTS) {
        oled_show_three_lines("CARD ADD", "EXISTS", uid_text);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD ADD: EXISTS");
    } else {
        oled_show_three_lines("CARD ADD", "FULL", 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD ADD: FULL");
    }
}

void main_controller_app_card_del_current(void)
{
    uint8_t uid[5];
    char uid_text[12];
    main_card_del_result_t result;

    if (!kt_rc522_read_uid(uid)) {
        oled_show_three_lines("CARD DEL", "NO CARD", 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD DEL: NO_CARD");
        return;
    }

    uid_to_text(uid, uid_text, sizeof(uid_text));
    log_uid4("CARD DEL", uid);
    result = main_card_db_remove(uid);

    if (result == MAIN_CARD_DEL_OK) {
        main_access_log_flush();
        oled_show_three_lines("CARD DEL", "OK", uid_text);
        main_controller_app_beep_success();
        KT_LOG_INFO("CARD DEL: OK");
    } else {
        oled_show_three_lines("CARD DEL", "NOT FOUND", uid_text);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD DEL: NOT_FOUND");
    }
}

void main_controller_app_print_card_db(void)
{
    main_card_db_print();
}

void main_controller_app_clear_card_db(void)
{
    main_card_db_clear();
    main_access_log_clear();
    main_controller_app_beep_success();
}

void main_controller_app_print_access_log(void)
{
    main_access_log_print_all();
}

void main_controller_app_clear_access_log(void)
{
    main_access_log_clear();
    main_controller_app_beep_success();
}

void main_controller_app_print_access_stats(void)
{
    main_access_log_print_stats();
}

void main_controller_app_send_last_card_event_to_esp32(void)
{
    if (last_card_event == CARD_EVENT_OK) {
        kt_esp32_link_send_card_event(last_uid, (uint8_t)last_access_type, 1U, &last_time);
    } else if (last_card_event == CARD_EVENT_DENIED) {
        kt_esp32_link_send_card_event(last_uid, MAIN_ACCESS_DENIED, 0U, &last_time);
    } else {
        KT_LOG_WARN("No real CARD_EVENT available for ESP32");
    }
}

void main_controller_app_beep_success(void)
{
    kt_buzzer_beep(&app_buzzer, 80U);
    KT_LOG_INFO("Buzzer success beep");
}

void main_controller_app_beep_fail(void)
{
    kt_buzzer_beep(&app_buzzer, 200U);
    KT_LOG_INFO("Buzzer fail beep");
}

void main_controller_app_toggle_seat_state(void)
{
    seat_occupied[0] ^= 1U;
    seat_occupied[1] ^= 1U;
    seat_occupied[2] ^= 1U;
    KT_LOG_INFO("Sim seats: S1 %s, S2 %s, S3 %s",
                seat_text(seat_occupied[0]),
                seat_text(seat_occupied[1]),
                seat_text(seat_occupied[2]));
}
