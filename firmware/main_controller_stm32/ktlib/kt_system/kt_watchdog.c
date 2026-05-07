#include "kt_watchdog.h"
#include "kt_config.h"
#include "kt_log.h"
#include "kt_system_health.h"
#include "stm32f1xx.h"

#if (KT_WATCHDOG_ENABLE != 0U)
static uint8_t watchdog_started;

static void iwdg_wait_ready(void)
{
    uint32_t guard = 100000U;

    while ((IWDG->SR != 0U) && (guard > 0U)) {
        guard--;
    }
}
#endif

void kt_watchdog_init(void)
{
#if (KT_WATCHDOG_ENABLE != 0U)
    IWDG->KR = 0x5555U;
    iwdg_wait_ready();
    IWDG->PR = (uint32_t)(KT_WATCHDOG_PR_VALUE & 0x07U);
    IWDG->RLR = (uint32_t)(KT_WATCHDOG_RELOAD_VALUE & 0x0FFFU);
    iwdg_wait_ready();
    IWDG->KR = 0xAAAAU;
    IWDG->KR = 0xCCCCU;
    watchdog_started = 1U;
    KT_LOG_WARN("IWDG enabled: timeout about 6-8s");
#endif
}

void kt_watchdog_task(void)
{
#if (KT_WATCHDOG_ENABLE != 0U)
    if (watchdog_started == 0U) {
        return;
    }

    if (kt_system_health_is_ok_for_watchdog() != 0U) {
        IWDG->KR = 0xAAAAU;
    }
#endif
}
