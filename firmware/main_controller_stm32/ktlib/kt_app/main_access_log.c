#include "main_access_log.h"
#include "main_card_db.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_system/kt_tick.h"
#include "kt_system/kt_system_health.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define ACCESS_LOG_FLASH_VERSION  2U
#define ACCESS_LOG_FLASH_WORDS    256U
#define ACCESS_LOG_FLASH_SAVE_DELAY_MS 5000U

typedef struct {
    uint8_t used;
    uint8_t uid[4];
    uint8_t in_library;
} main_access_presence_t;

#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
typedef struct {
    uint32_t index;
    uint8_t uid[4];
    uint8_t type;
    uint8_t allowed;
    kt_ds1302_time_t time;
} main_access_flash_record_t;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t next_index;
    uint8_t record_start;
    uint8_t record_count;
    uint8_t reserved0;
    uint8_t reserved1;
    main_access_flash_record_t records[MAIN_ACCESS_LOG_MAX_COUNT];
} main_access_flash_image_t;
#endif

static main_access_record_t records[MAIN_ACCESS_LOG_MAX_COUNT];
static main_access_presence_t presence[MAIN_CARD_MAX_COUNT];
static uint8_t record_start;
static uint8_t record_count;
static uint32_t next_index = 1U;
static uint8_t flash_dirty;
static uint32_t flash_dirty_ms;
#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
static uint32_t flash_save_words[ACCESS_LOG_FLASH_WORDS];

typedef char access_log_flash_size_check[(sizeof(main_access_flash_image_t) <= 1024U) ? 1 : -1];
#endif

static uint8_t uid_equal(const uint8_t a[4], const uint8_t b[4])
{
    return (memcmp(a, b, 4U) == 0) ? 1U : 0U;
}

static main_access_presence_t *find_presence(const uint8_t uid[4], uint8_t create)
{
    uint8_t i;
    uint8_t free_slot = 0xFFU;

    if (uid == 0) {
        return 0;
    }

    for (i = 0; i < MAIN_CARD_MAX_COUNT; i++) {
        if (presence[i].used && uid_equal(presence[i].uid, uid)) {
            return &presence[i];
        }
        if (!presence[i].used && free_slot == 0xFFU) {
            free_slot = i;
        }
    }

    if (create && free_slot != 0xFFU) {
        presence[free_slot].used = 1U;
        presence[free_slot].in_library = 0U;
        memcpy(presence[free_slot].uid, uid, 4U);
        return &presence[free_slot];
    }

    return 0;
}

#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
static void rebuild_presence_from_records(void)
{
    uint8_t i;

    memset(presence, 0, sizeof(presence));
    for (i = 0; i < record_count; i++) {
        uint8_t slot = (uint8_t)((record_start + i) % MAIN_ACCESS_LOG_MAX_COUNT);
        const main_access_record_t *r = &records[slot];

        if (r->allowed && r->type != MAIN_ACCESS_DENIED) {
            main_access_presence_t *item = find_presence(r->uid, 1U);
            if (item != 0) {
                item->in_library = (r->type == MAIN_ACCESS_CHECK_IN) ? 1U : 0U;
            }
        }
    }
}
#endif

#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
static void access_log_flash_save(void)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;
    uint32_t i;
    uint32_t start_ms = kt_tick_get_ms();
    main_access_flash_image_t *image = (main_access_flash_image_t *)flash_save_words;

    memset(flash_save_words, 0xFF, sizeof(flash_save_words));
    image->magic = MAIN_ACCESS_LOG_FLASH_MAGIC;
    image->version = ACCESS_LOG_FLASH_VERSION;
    image->next_index = next_index;
    image->record_start = record_start;
    image->record_count = record_count;

    for (i = 0; i < MAIN_ACCESS_LOG_MAX_COUNT; i++) {
        image->records[i].index = records[i].index;
        memcpy(image->records[i].uid, records[i].uid, 4U);
        image->records[i].type = (uint8_t)records[i].type;
        image->records[i].allowed = records[i].allowed;
        image->records[i].time = records[i].time;
    }

    HAL_FLASH_Unlock();
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = MAIN_ACCESS_LOG_FLASH_PAGE_ADDR;
    erase.NbPages = 1U;

    if (HAL_FLASHEx_Erase(&erase, &page_error) == HAL_OK) {
        for (i = 0; i < ACCESS_LOG_FLASH_WORDS; i++) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                  MAIN_ACCESS_LOG_FLASH_PAGE_ADDR + (i * 4U),
                                  flash_save_words[i]) != HAL_OK) {
                KT_LOG_WARN("ACCESS LOG FLASH WRITE FAIL");
                break;
            }
        }
    } else {
        KT_LOG_WARN("ACCESS LOG FLASH ERASE FAIL");
    }
    HAL_FLASH_Lock();
    kt_system_health_note_flash_save(kt_tick_get_ms() - start_ms);
}
#endif

void main_access_log_init(void)
{
#if (MAIN_ACCESS_LOG_FLASH_ENABLE == 0U)
    memset(records, 0, sizeof(records));
    memset(presence, 0, sizeof(presence));
    record_start = 0U;
    record_count = 0U;
    next_index = 1U;
    flash_dirty = 0U;
    flash_dirty_ms = 0U;
    KT_LOG_WARN("ACCESS LOG FLASH DISABLED: RAM only");
    return;
#else
    uint8_t i;
    const main_access_flash_image_t *image =
        (const main_access_flash_image_t *)MAIN_ACCESS_LOG_FLASH_PAGE_ADDR;

    memset(records, 0, sizeof(records));
    memset(presence, 0, sizeof(presence));
    record_start = 0U;
    record_count = 0U;
    next_index = 1U;
    flash_dirty = 0U;
    flash_dirty_ms = 0U;

    if (image->magic != MAIN_ACCESS_LOG_FLASH_MAGIC ||
        image->version != ACCESS_LOG_FLASH_VERSION ||
        image->record_count > MAIN_ACCESS_LOG_MAX_COUNT ||
        image->record_start >= MAIN_ACCESS_LOG_MAX_COUNT) {
        KT_LOG_INFO("ACCESS LOG FLASH EMPTY");
        return;
    }

    record_start = image->record_start;
    record_count = image->record_count;
    next_index = image->next_index;
    if (next_index == 0U || next_index == 0xFFFFFFFFU) {
        next_index = 1U;
    }

    for (i = 0; i < MAIN_ACCESS_LOG_MAX_COUNT; i++) {
        records[i].index = image->records[i].index;
        memcpy(records[i].uid, image->records[i].uid, 4U);
        records[i].type = (main_access_type_t)image->records[i].type;
        records[i].allowed = image->records[i].allowed;
        records[i].time = image->records[i].time;
    }

    rebuild_presence_from_records();
    KT_LOG_INFO("ACCESS LOG FLASH LOADED: logs=%u/%u",
                (unsigned int)record_count,
                (unsigned int)MAIN_ACCESS_LOG_MAX_COUNT);
#endif
}

void main_access_log_flush(void)
{
#if (MAIN_ACCESS_LOG_FLASH_ENABLE == 0U)
    flash_dirty = 0U;
    flash_dirty_ms = 0U;
    KT_LOG_WARN("ACCESS LOG FLASH DISABLED: flush skipped");
    return;
#else
    flash_dirty = 0U;
    access_log_flash_save();
    KT_LOG_INFO("LOCAL STORE FLASH SAVED");
#endif
}

void main_access_log_task(uint8_t allow_flash_write)
{
#if (MAIN_ACCESS_LOG_FLASH_ENABLE == 0U)
    (void)allow_flash_write;
    return;
#else
    if (flash_dirty == 0U || allow_flash_write == 0U) {
        return;
    }

    if (!kt_tick_is_timeout(flash_dirty_ms, ACCESS_LOG_FLASH_SAVE_DELAY_MS)) {
        return;
    }

    flash_dirty = 0U;
    access_log_flash_save();
#endif
}

const char *main_access_log_type_text(main_access_type_t type)
{
    if (type == MAIN_ACCESS_CHECK_IN) {
        return "CHECK_IN";
    }
    if (type == MAIN_ACCESS_CHECK_OUT) {
        return "CHECK_OUT";
    }
    return "DENIED";
}

main_access_type_t main_access_log_next_type_for_uid(const uint8_t uid[4])
{
    main_access_presence_t *item = find_presence(uid, 1U);

    if (item == 0 || !item->in_library) {
        if (item != 0) {
            item->in_library = 1U;
        }
        return MAIN_ACCESS_CHECK_IN;
    }

    item->in_library = 0U;
    return MAIN_ACCESS_CHECK_OUT;
}

void main_access_log_add(const uint8_t uid[4],
                         main_access_type_t type,
                         uint8_t allowed,
                         const kt_ds1302_time_t *time)
{
    uint8_t slot;

    if (uid == 0 || time == 0) {
        return;
    }

    if (record_count < MAIN_ACCESS_LOG_MAX_COUNT) {
        slot = (uint8_t)((record_start + record_count) % MAIN_ACCESS_LOG_MAX_COUNT);
        record_count++;
    } else {
        slot = record_start;
        record_start = (uint8_t)((record_start + 1U) % MAIN_ACCESS_LOG_MAX_COUNT);
    }

    records[slot].index = next_index++;
    memcpy(records[slot].uid, uid, 4U);
    records[slot].type = type;
    records[slot].allowed = allowed ? 1U : 0U;
    records[slot].time = *time;
#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
    flash_dirty = 1U;
    flash_dirty_ms = kt_tick_get_ms();
#endif
}

void main_access_log_clear(void)
{
    memset(records, 0, sizeof(records));
    memset(presence, 0, sizeof(presence));
    record_start = 0U;
    record_count = 0U;
    next_index = 1U;
    flash_dirty = 0U;
#if (MAIN_ACCESS_LOG_FLASH_ENABLE != 0U)
    access_log_flash_save();
#else
    flash_dirty_ms = 0U;
    KT_LOG_WARN("FLASH PERSIST DISABLED");
#endif
    KT_LOG_INFO("RAM ACCESS LOG CLEARED");
}

uint8_t main_access_log_count(void)
{
    return record_count;
}

void main_access_log_print_all(void)
{
    uint8_t i;

    if (record_count == 0U) {
        KT_LOG_INFO("RAM ACCESS LOG EMPTY");
        KT_LOG_WARN("FLASH PERSIST DISABLED");
        return;
    }

    KT_LOG_INFO("RAM ACCESS LOG COUNT: %u/%u",
                (unsigned int)record_count,
                (unsigned int)MAIN_ACCESS_LOG_MAX_COUNT);

    for (i = 0; i < record_count; i++) {
        uint8_t slot = (uint8_t)((record_start + i) % MAIN_ACCESS_LOG_MAX_COUNT);
        const main_access_record_t *r = &records[slot];

        KT_LOG_INFO("#%03lu 20%02u-%02u-%02u %02u:%02u:%02u UID=%02X %02X %02X %02X %s %s",
                    (unsigned long)r->index,
                    (unsigned int)r->time.year,
                    (unsigned int)r->time.month,
                    (unsigned int)r->time.day,
                    (unsigned int)r->time.hour,
                    (unsigned int)r->time.minute,
                    (unsigned int)r->time.second,
                    r->uid[0],
                    r->uid[1],
                    r->uid[2],
                    r->uid[3],
                    main_access_log_type_text(r->type),
                    r->allowed ? "OK" : "");
    }
}

void main_access_log_print_stats(void)
{
    uint8_t i;
    uint8_t allowed = 0U;
    uint8_t denied = 0U;

    for (i = 0; i < record_count; i++) {
        uint8_t slot = (uint8_t)((record_start + i) % MAIN_ACCESS_LOG_MAX_COUNT);
        if (records[slot].allowed) {
            allowed++;
        } else {
            denied++;
        }
    }

    KT_LOG_INFO("RAM ACCESS LOG COUNT: %u/%u",
                (unsigned int)record_count,
                (unsigned int)MAIN_ACCESS_LOG_MAX_COUNT);
    KT_LOG_INFO("RAM ACCESS OK: %u", (unsigned int)allowed);
    KT_LOG_INFO("RAM ACCESS DENIED: %u", (unsigned int)denied);
    KT_LOG_WARN("FLASH PERSIST DISABLED");
}
