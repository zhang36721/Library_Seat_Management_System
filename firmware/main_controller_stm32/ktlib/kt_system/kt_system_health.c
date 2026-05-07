#include "kt_system_health.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_system/kt_tick.h"
#include "stm32f1xx_hal.h"

static uint32_t main_loop_alive_count;
static uint32_t last_main_loop_ms;
static uint32_t last_main_app_ms;
static uint32_t last_esp32_task_ms;
static uint32_t last_zigbee_task_ms;
static uint32_t last_flash_save_ms;
static uint32_t last_flash_save_cost_ms;
static uint32_t last_card_flow_cost_ms;
static uint32_t hardfault_count;
static uint32_t fault_count;
static const char *last_fault_name;
static uint32_t reset_csr;

static void print_reset_reason(void)
{
    KT_LOG_INFO("reset CSR=0x%08lX", (unsigned long)reset_csr);
    if ((reset_csr & RCC_CSR_IWDGRSTF) != 0U) {
        KT_LOG_WARN("reset reason: IWDG");
    } else if ((reset_csr & RCC_CSR_WWDGRSTF) != 0U) {
        KT_LOG_WARN("reset reason: WWDG");
    } else if ((reset_csr & RCC_CSR_SFTRSTF) != 0U) {
        KT_LOG_WARN("reset reason: software reset");
    } else if ((reset_csr & RCC_CSR_PORRSTF) != 0U) {
        KT_LOG_INFO("reset reason: power on/reset");
    } else if ((reset_csr & RCC_CSR_PINRSTF) != 0U) {
        KT_LOG_INFO("reset reason: reset pin");
    } else {
        KT_LOG_INFO("reset reason: unknown");
    }
}

void kt_system_health_init(void)
{
    reset_csr = RCC->CSR;
    __HAL_RCC_CLEAR_RESET_FLAGS();
    main_loop_alive_count = 0U;
    last_main_loop_ms = 0U;
    last_main_app_ms = 0U;
    last_esp32_task_ms = 0U;
    last_zigbee_task_ms = 0U;
    last_flash_save_ms = 0U;
    last_flash_save_cost_ms = 0U;
    last_card_flow_cost_ms = 0U;
    hardfault_count = 0U;
    fault_count = 0U;
    last_fault_name = "none";
}

void kt_system_health_note_main_loop(void)
{
    main_loop_alive_count++;
    last_main_loop_ms = kt_tick_get_ms();
}

void kt_system_health_note_main_app_task(void)
{
    last_main_app_ms = kt_tick_get_ms();
}

void kt_system_health_note_esp32_task(void)
{
    last_esp32_task_ms = kt_tick_get_ms();
}

void kt_system_health_note_zigbee_task(void)
{
    last_zigbee_task_ms = kt_tick_get_ms();
}

void kt_system_health_note_flash_save(uint32_t cost_ms)
{
    last_flash_save_ms = kt_tick_get_ms();
    last_flash_save_cost_ms = cost_ms;
}

void kt_system_health_note_card_flow(uint32_t cost_ms)
{
    last_card_flow_cost_ms = cost_ms;
}

void kt_system_health_note_fault(const char *name)
{
    fault_count++;
    if (name != 0) {
        last_fault_name = name;
        if (name[0] == 'H') {
            hardfault_count++;
        }
    }
}

void kt_system_health_print(void)
{
    uint32_t now = kt_tick_get_ms();

    KT_LOG_INFO("SYSTEM HEALTH");
    print_reset_reason();
    KT_LOG_INFO("main_loop_alive=%lu last=%lu age=%lu",
                (unsigned long)main_loop_alive_count,
                (unsigned long)last_main_loop_ms,
                (unsigned long)(now - last_main_loop_ms));
    KT_LOG_INFO("last main_app=%lu age=%lu",
                (unsigned long)last_main_app_ms,
                (unsigned long)(now - last_main_app_ms));
    KT_LOG_INFO("last esp32_task=%lu age=%lu",
                (unsigned long)last_esp32_task_ms,
                (unsigned long)(now - last_esp32_task_ms));
    KT_LOG_INFO("last zigbee_task=%lu age=%lu",
                (unsigned long)last_zigbee_task_ms,
                (unsigned long)(now - last_zigbee_task_ms));
#if (MAIN_CARD_DB_FLASH_ENABLE != 0U)
    KT_LOG_INFO("card_db_flash=ENABLED");
#else
    KT_LOG_INFO("card_db_flash=DISABLED");
#endif
#if (MAIN_ACCESS_LOG_FLASH_ENABLE == 0U)
    KT_LOG_INFO("access_log_flash=DISABLED");
    KT_LOG_INFO("last flash_save=0 cost=0 ms");
#else
    KT_LOG_INFO("access_log_flash=ENABLED");
    KT_LOG_INFO("last flash_save=%lu cost=%lu ms",
                (unsigned long)last_flash_save_ms,
                (unsigned long)last_flash_save_cost_ms);
#endif
    KT_LOG_INFO("last card_flow cost=%lu ms",
                (unsigned long)last_card_flow_cost_ms);
    KT_LOG_INFO("fault_count=%lu hardfault_count=%lu last=%s",
                (unsigned long)fault_count,
                (unsigned long)hardfault_count,
                last_fault_name);
}
