#include "main_card_db.h"
#include "kt_log.h"
#include <string.h>

static main_card_record_t card_table[MAIN_CARD_MAX_COUNT];

static uint8_t uid_equal(const uint8_t a[4], const uint8_t b[4])
{
    return (memcmp(a, b, 4U) == 0) ? 1U : 0U;
}

uint8_t main_card_db_contains(const uint8_t uid[4])
{
    uint8_t i;

    if (uid == 0) {
        return 0;
    }

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (card_table[i].used && uid_equal(card_table[i].uid, uid)) {
            return 1;
        }
    }
    return 0;
}

uint8_t main_card_db_count(void)
{
    uint8_t i;
    uint8_t count = 0U;

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (card_table[i].used) {
            count++;
        }
    }

    return count;
}

main_card_add_result_t main_card_db_add(const uint8_t uid[4])
{
    uint8_t i;

    if (uid == 0) {
        return MAIN_CARD_ADD_FULL;
    }

    if (main_card_db_contains(uid)) {
        return MAIN_CARD_ADD_EXISTS;
    }

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (!card_table[i].used) {
            card_table[i].used = 1U;
            memcpy(card_table[i].uid, uid, 4U);
            return MAIN_CARD_ADD_OK;
        }
    }

    return MAIN_CARD_ADD_FULL;
}

main_card_del_result_t main_card_db_remove(const uint8_t uid[4])
{
    uint8_t i;

    if (uid == 0) {
        return MAIN_CARD_DEL_NOT_FOUND;
    }

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (card_table[i].used && uid_equal(card_table[i].uid, uid)) {
            card_table[i].used = 0U;
            memset(card_table[i].uid, 0, sizeof(card_table[i].uid));
            return MAIN_CARD_DEL_OK;
        }
    }

    return MAIN_CARD_DEL_NOT_FOUND;
}

void main_card_db_export(main_card_record_t records[MAIN_CARD_MAX_COUNT])
{
    if (records == 0) {
        return;
    }
    memcpy(records, card_table, sizeof(card_table));
}

void main_card_db_import(const main_card_record_t records[MAIN_CARD_MAX_COUNT])
{
    uint8_t i;

    memset(card_table, 0, sizeof(card_table));
    if (records == 0) {
        return;
    }

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (records[i].used) {
            card_table[i].used = 1U;
            memcpy(card_table[i].uid, records[i].uid, 4U);
        }
    }
}

void main_card_db_clear(void)
{
    memset(card_table, 0, sizeof(card_table));
    KT_LOG_INFO("Card DB cleared");
}

void main_card_db_print(void)
{
    uint8_t i;
    uint8_t count = 0;

    KT_LOG_INFO("Card DB max: %u", (unsigned int)MAIN_CARD_MAX_COUNT);
    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (card_table[i].used) {
            count++;
            KT_LOG_INFO("Card %u: %02X %02X %02X %02X",
                        (unsigned int)count,
                        card_table[i].uid[0],
                        card_table[i].uid[1],
                        card_table[i].uid[2],
                        card_table[i].uid[3]);
        }
    }

    if (count == 0U) {
        KT_LOG_INFO("Card DB empty");
    } else {
        KT_LOG_INFO("Card DB count: %u", (unsigned int)count);
    }
}
