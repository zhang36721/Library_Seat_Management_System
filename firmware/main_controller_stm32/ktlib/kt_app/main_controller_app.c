#include "main_controller_app.h"
#include "app_io.h"
#include "main_card_db.h"
#include "kt_log.h"
#include "kt_modules/kt_rc522.h"
#include "kt_modules/kt_oled.h"
#include "kt_modules/kt_ds1302.h"
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

static uint8_t seat_occupied[3] = {0U, 1U, 0U};
static card_event_t last_card_event = CARD_EVENT_NONE;
static uint8_t last_uid[5] = {0};
static kt_ds1302_time_t last_time = {0};

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

static void oled_show_card_ok(const uint8_t uid[5], const kt_ds1302_time_t *time)
{
    char uid_text[12];
    char time_text[12];

    uid_to_text(uid, uid_text, sizeof(uid_text));
    time_to_hms_text(time, time_text, sizeof(time_text));

    kt_oled_clear();
    kt_oled_print_line(0, "CARD OK");
    kt_oled_print_line(1, uid_text);
    kt_oled_print_line(2, time_text);
}

void main_controller_app_print_status(void)
{
    KT_LOG_INFO("Main app: v0.8.4 local card terminal");
    KT_LOG_INFO("Seats: S1 %s, S2 %s, S3 %s",
                seat_text(seat_occupied[0]),
                seat_text(seat_occupied[1]),
                seat_text(seat_occupied[2]));

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
    kt_ds1302_time_t now;

    if (!kt_rc522_read_uid(uid)) {
        last_card_event = CARD_EVENT_NO_CARD;
        oled_show_card_fail("NO CARD");
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD EVENT: NO_CARD");
        return;
    }

    memcpy(last_uid, uid, sizeof(last_uid));
    if (!main_card_db_contains(uid)) {
        char uid_text[12];

        uid_to_text(uid, uid_text, sizeof(uid_text));
        last_card_event = CARD_EVENT_DENIED;
        oled_show_three_lines("CARD DENIED", uid_text, 0);
        main_controller_app_beep_fail();
        KT_LOG_WARN("CARD EVENT: DENIED UID=%02X %02X %02X %02X",
                    uid[0], uid[1], uid[2], uid[3]);
        return;
    }

    kt_ds1302_read_time(&now);
    if (!kt_ds1302_time_is_valid(&now)) {
        last_card_event = CARD_EVENT_BAD_TIME;
        oled_show_card_fail("BAD TIME");
        main_controller_app_beep_fail();
        KT_LOG_INFO("CARD UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
        KT_LOG_WARN("CARD EVENT: INVALID_TIME");
        return;
    }

    last_time = now;
    last_card_event = CARD_EVENT_OK;

    oled_show_card_ok(uid, &now);
    main_controller_app_beep_success();

    KT_LOG_INFO("TIME: 20%02u-%02u-%02u %02u:%02u:%02u",
                (unsigned int)now.year,
                (unsigned int)now.month,
                (unsigned int)now.day,
                (unsigned int)now.hour,
                (unsigned int)now.minute,
                (unsigned int)now.second);
    KT_LOG_INFO("CARD EVENT: CHECK_IN OK UID=%02X %02X %02X %02X",
                uid[0], uid[1], uid[2], uid[3]);
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
    kt_oled_print_line(0, "KENTO LIB");
    snprintf(line, sizeof(line), "TIME %s", hm);
    kt_oled_print_line(1, line);
    snprintf(line, sizeof(line), "S1 %s", seat_text(seat_occupied[0]));
    kt_oled_print_line(2, line);
    snprintf(line, sizeof(line), "S2 %s S3 %s",
             seat_text(seat_occupied[1]), seat_text(seat_occupied[2]));
    kt_oled_print_line(3, line);

    KT_LOG_INFO("OLED home shown");
}

void main_controller_app_show_last_card(void)
{
    if (last_card_event == CARD_EVENT_OK) {
        oled_show_card_ok(last_uid, &last_time);
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
    main_controller_app_beep_success();
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
