#include "kt_boot_count.h"
#include "kt_config.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

#define KT_BOOT_COUNT_FLASH_MAGIC_ADDR  (KT_BOOT_COUNT_FLASH_PAGE_ADDR)
#define KT_BOOT_COUNT_FLASH_VALUE_ADDR  (KT_BOOT_COUNT_FLASH_PAGE_ADDR + 4U)

static uint32_t kt_boot_count_value = 0;

static uint32_t boot_count_flash_read(uint32_t addr)
{
    return *(__IO uint32_t *)addr;
}

static void boot_count_flash_write(uint32_t count)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = KT_BOOT_COUNT_FLASH_PAGE_ADDR;
    erase.NbPages = 1;

    if (HAL_FLASHEx_Erase(&erase, &page_error) == HAL_OK) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                              KT_BOOT_COUNT_FLASH_MAGIC_ADDR,
                              KT_BOOT_COUNT_MAGIC) == HAL_OK) {
            (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                    KT_BOOT_COUNT_FLASH_VALUE_ADDR,
                                    count);
        }
    }

    HAL_FLASH_Lock();
}

void kt_boot_count_init(void)
{
    uint32_t magic = boot_count_flash_read(KT_BOOT_COUNT_FLASH_MAGIC_ADDR);
    uint32_t stored_count = boot_count_flash_read(KT_BOOT_COUNT_FLASH_VALUE_ADDR);

    if (magic != KT_BOOT_COUNT_MAGIC || stored_count == 0xFFFFFFFFU) {
        kt_boot_count_value = 1;
    } else {
        kt_boot_count_value = stored_count + 1U;
        if (kt_boot_count_value == 0U || kt_boot_count_value == 0xFFFFFFFFU) {
            kt_boot_count_value = 1;
        }
    }

    boot_count_flash_write(kt_boot_count_value);
}

uint32_t kt_boot_count_get(void)
{
    return kt_boot_count_value;
}
