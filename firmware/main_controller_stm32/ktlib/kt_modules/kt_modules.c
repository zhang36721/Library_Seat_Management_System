#include "kt_modules.h"
#include "kt_log.h"
#include "kt_rc522.h"
#include "kt_oled.h"
#include "kt_ds1302.h"
#include "kt_stepper.h"
#include "kt_uart_links.h"
#include "kt_esp32_link.h"

void kt_modules_init(void)
{
    (void)kt_rc522_init();
    (void)kt_oled_init_startup();
    (void)kt_ds1302_init_check();
    kt_stepper_init();

    kt_zigbee_uart_start_receive_it();
    KT_LOG_INFO("USART1 ZigBee UART init: OK");
    KT_LOG_WARN("ZigBee module/network status: pending real link test");

    kt_esp32_link_init();
    KT_LOG_INFO("USART3 ESP32S3 UART init: OK");
    KT_LOG_WARN("ESP32S3 peer status: pending binary link test");
}

void kt_modules_task(void)
{
    kt_stepper_task();
    kt_zigbee_task();
    kt_esp32_link_task();
}
