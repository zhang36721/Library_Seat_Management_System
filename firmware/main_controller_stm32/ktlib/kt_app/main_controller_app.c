#include "main_controller_app.h"
#include "app_io.h"
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
    CARD_EVENT_BAD_TIME
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
    KT_LOG_INFO("Main app: v0.8.2 local card terminal");
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

    kt_ds1302_read_time(&now);
    if (!kt_ds1302_time_is_valid(&now)) {
        memcpy(last_uid, uid, sizeof(last_uid));
        last_card_event = CARD_EVENT_BAD_TIME;
        oled_show_card_fail("BAD TIME");
        main_controller_app_beep_fail();
        KT_LOG_INFO("CARD UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
        KT_LOG_WARN("CARD EVENT: INVALID_TIME");
        return;
    }

    memcpy(last_uid, uid, sizeof(last_uid));
    last_time = now;
    last_card_event = CARD_EVENT_OK;

    oled_show_card_ok(uid, &now);
    main_controller_app_beep_success();

    KT_LOG_INFO("CARD UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
    KT_LOG_INFO("TIME: 20%02u-%02u-%02u %02u:%02u:%02u",
                (unsigned int)now.year,
                (unsigned int)now.month,
                (unsigned int)now.day,
                (unsigned int)now.hour,
                (unsigned int)now.minute,
                (unsigned int)now.second);
    KT_LOG_INFO("CARD EVENT: CHECK_IN_TEST OK");
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
    } else {
        kt_oled_clear();
        kt_oled_print_line(0, "CARD EVENT");
        kt_oled_print_line(1, "NONE");
        KT_LOG_INFO("OLED last card empty shown");
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
