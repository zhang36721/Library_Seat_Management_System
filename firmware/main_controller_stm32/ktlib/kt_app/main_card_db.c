#include "main_card_db.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_system/kt_tick.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define CARD_DB_FLASH_VERSION 1U
#define CARD_DB_FLASH_WORDS   256U

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    uint32_t checksum;
    main_card_record_t cards[MAIN_CARD_MAX_COUNT];
} main_card_db_flash_image_t;

static main_card_record_t card_table[MAIN_CARD_MAX_COUNT];
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
static uint32_t card_db_flash_words[CARD_DB_FLASH_WORDS];
static uint8_t card_db_flash_loaded;
static uint32_t card_db_flash_save_count;
static uint32_t card_db_flash_fail_count;
static uint32_t card_db_flash_last_cost_ms;

typedef char card_db_flash_size_check[(sizeof(main_card_db_flash_image_t) <= 1024U) ? 1 : -1];
#endif

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

#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
static uint32_t card_db_checksum(const main_card_record_t records[MAIN_CARD_MAX_COUNT])
{
    uint8_t i;
    uint8_t j;
    uint32_t sum = 0x4B544344U;

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        sum = (sum << 5) ^ (sum >> 2) ^ records[i].used;
        for (j = 0; j < 4U; j++) {
            sum = (sum << 5) ^ (sum >> 2) ^ records[i].uid[j];
        }
    }

    return sum;
}

static void main_card_db_flash_save(void)
{
    FLASH_EraseInitTypeDef erase = {0};
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t page_error = 0U;
    uint32_t i;
    uint32_t start_ms = kt_tick_get_ms();
    main_card_db_flash_image_t *image = (main_card_db_flash_image_t *)card_db_flash_words;

    memset(card_db_flash_words, 0xFF, sizeof(card_db_flash_words));
    image->magic = MAIN_CARD_DB_FLASH_MAGIC;
    image->version = CARD_DB_FLASH_VERSION;
    image->count = main_card_db_count();
    memcpy(image->cards, card_table, sizeof(card_table));
    image->checksum = card_db_checksum(image->cards);

    HAL_FLASH_Unlock();
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = MAIN_CARD_DB_FLASH_PAGE_ADDR;
    erase.NbPages = 1U;

    status = HAL_FLASHEx_Erase(&erase, &page_error);
    if (status == HAL_OK) {
        for (i = 0; i < CARD_DB_FLASH_WORDS; i++) {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                       MAIN_CARD_DB_FLASH_PAGE_ADDR + (i * 4U),
                                       card_db_flash_words[i]);
            if (status != HAL_OK) {
                break;
            }
        }
    }

    HAL_FLASH_Lock();
    card_db_flash_last_cost_ms = kt_tick_get_ms() - start_ms;

    if (status == HAL_OK) {
        card_db_flash_save_count++;
        KT_LOG_INFO("CARD DB FLASH SAVED: count=%u cost=%lu ms",
                    (unsigned int)image->count,
                    (unsigned long)card_db_flash_last_cost_ms);
    } else {
        card_db_flash_fail_count++;
        KT_LOG_WARN("CARD DB FLASH SAVE FAIL: err=%lu cost=%lu ms",
                    (unsigned long)status,
                    (unsigned long)card_db_flash_last_cost_ms);
    }
}

static uint8_t main_card_db_flash_load(void)
{
    const main_card_db_flash_image_t *image =
        (const main_card_db_flash_image_t *)MAIN_CARD_DB_FLASH_PAGE_ADDR;

    if (image->magic != MAIN_CARD_DB_FLASH_MAGIC ||
        image->version != CARD_DB_FLASH_VERSION ||
        image->count > MAIN_CARD_MAX_COUNT ||
        image->checksum != card_db_checksum(image->cards)) {
        return 0U;
    }

    memcpy(card_table, image->cards, sizeof(card_table));
    card_db_flash_loaded = 1U;
    KT_LOG_INFO("CARD DB FLASH LOADED: count=%u/%u",
                (unsigned int)main_card_db_count(),
                (unsigned int)MAIN_CARD_MAX_COUNT);
    return 1U;
}
#endif

void main_card_db_init(void)
{
    memset(card_table, 0, sizeof(card_table));
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
    card_db_flash_loaded = 0U;
    card_db_flash_save_count = 0U;
    card_db_flash_fail_count = 0U;
    card_db_flash_last_cost_ms = 0U;
    if (main_card_db_flash_load() == 0U) {
        KT_LOG_WARN("CARD DB FLASH EMPTY: RAM starts empty");
    }
#else
    KT_LOG_WARN("CARD DB FLASH DISABLED: RAM only");
#endif
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
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
            main_card_db_flash_save();
#endif
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
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
            main_card_db_flash_save();
#endif
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
    KT_LOG_INFO("RAM Card DB cleared");
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
    main_card_db_flash_save();
#endif
}

void main_card_db_print(void)
{
    uint8_t i;
    uint8_t count = 0;

    KT_LOG_INFO("Card DB flash: %s",
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
                "ENABLED");
    KT_LOG_INFO("Card DB flash loaded=%u saves=%lu fails=%lu last_cost=%lu ms",
                (unsigned int)card_db_flash_loaded,
                (unsigned long)card_db_flash_save_count,
                (unsigned long)card_db_flash_fail_count,
                (unsigned long)card_db_flash_last_cost_ms);
#else
                "DISABLED");
#endif
    KT_LOG_INFO("RAM Card DB max: %u", (unsigned int)MAIN_CARD_MAX_COUNT);
    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (card_table[i].used) {
            count++;
            KT_LOG_INFO("RAM Card %u: %02X %02X %02X %02X",
                        (unsigned int)count,
                        card_table[i].uid[0],
                        card_table[i].uid[1],
                        card_table[i].uid[2],
                        card_table[i].uid[3]);
        }
    }

    if (count == 0U) {
        KT_LOG_INFO("RAM Card DB empty");
    } else {
        KT_LOG_INFO("RAM Card DB count: %u", (unsigned int)count);
    }
}
