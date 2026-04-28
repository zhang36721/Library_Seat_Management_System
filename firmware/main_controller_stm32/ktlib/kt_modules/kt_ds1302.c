#include "kt_ds1302.h"
#include "kt_config.h"
#include "kt_log.h"
#include "stm32f1xx_hal.h"

#define DS1302_SEC_W   0x80U
#define DS1302_MIN_W   0x82U
#define DS1302_HOUR_W  0x84U
#define DS1302_DATE_W  0x86U
#define DS1302_MON_W   0x88U
#define DS1302_YEAR_W  0x8CU
#define DS1302_WP_W    0x8EU

static void ds_delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 20U; i++) {
    }
}

static uint8_t bcd_to_dec(uint8_t bcd)
{
    return (uint8_t)(((bcd >> 4U) * 10U) + (bcd & 0x0FU));
}

static uint8_t dec_to_bcd(uint8_t dec)
{
    return (uint8_t)(((dec / 10U) << 4U) | (dec % 10U));
}

static void dat_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = KT_DS1302_DAT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(KT_DS1302_DAT_PORT, &GPIO_InitStruct);
}

static void dat_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = KT_DS1302_DAT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KT_DS1302_DAT_PORT, &GPIO_InitStruct);
}

static void write_byte(uint8_t data)
{
    uint8_t i;
    dat_output();
    for (i = 0; i < 8U; i++) {
        HAL_GPIO_WritePin(KT_DS1302_DAT_PORT, KT_DS1302_DAT_PIN,
                          (data & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        ds_delay();
        HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_SET);
        ds_delay();
        HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_RESET);
        data >>= 1;
    }
}

static uint8_t read_byte(void)
{
    uint8_t data = 0;
    uint8_t i;
    dat_input();
    for (i = 0; i < 8U; i++) {
        if (HAL_GPIO_ReadPin(KT_DS1302_DAT_PORT, KT_DS1302_DAT_PIN) == GPIO_PIN_SET) {
            data |= (uint8_t)(1U << i);
        }
        HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_SET);
        ds_delay();
        HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_RESET);
        ds_delay();
    }
    return data;
}

static void write_reg(uint8_t addr, uint8_t data)
{
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_SET);
    write_byte(addr);
    write_byte(data);
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_RESET);
}

static uint8_t read_reg(uint8_t addr)
{
    uint8_t data;
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_DS1302_CLK_PORT, KT_DS1302_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_SET);
    write_byte((uint8_t)(addr | 0x01U));
    data = read_byte();
    HAL_GPIO_WritePin(KT_DS1302_RST_PORT, KT_DS1302_RST_PIN, GPIO_PIN_RESET);
    return data;
}

void kt_ds1302_write_test_time(void)
{
    write_reg(DS1302_WP_W, 0x00U);
    write_reg(DS1302_SEC_W, dec_to_bcd(0));
    write_reg(DS1302_MIN_W, dec_to_bcd(34));
    write_reg(DS1302_HOUR_W, dec_to_bcd(12));
    write_reg(DS1302_DATE_W, dec_to_bcd(28));
    write_reg(DS1302_MON_W, dec_to_bcd(4));
    write_reg(DS1302_YEAR_W, dec_to_bcd(26));
    write_reg(DS1302_WP_W, 0x80U);
    KT_LOG_INFO("DS1302 test time written: 2026-04-28 12:34:00");
}

void kt_ds1302_read_time(kt_ds1302_time_t *time)
{
    if (time == 0) {
        return;
    }
    time->second = bcd_to_dec((uint8_t)(read_reg(DS1302_SEC_W) & 0x7FU));
    time->minute = bcd_to_dec(read_reg(DS1302_MIN_W));
    time->hour = bcd_to_dec((uint8_t)(read_reg(DS1302_HOUR_W) & 0x3FU));
    time->day = bcd_to_dec(read_reg(DS1302_DATE_W));
    time->month = bcd_to_dec(read_reg(DS1302_MON_W));
    time->year = bcd_to_dec(read_reg(DS1302_YEAR_W));
}

uint8_t kt_ds1302_time_is_valid(const kt_ds1302_time_t *time)
{
    if (time == 0) return 0;
    if (time->year > 99U) return 0;
    if (time->month < 1U || time->month > 12U) return 0;
    if (time->day < 1U || time->day > 31U) return 0;
    if (time->hour > 23U) return 0;
    if (time->minute > 59U) return 0;
    if (time->second > 59U) return 0;
    return 1;
}

uint8_t kt_ds1302_init_check(void)
{
    kt_ds1302_time_t t;
    kt_ds1302_read_time(&t);
    if (!kt_ds1302_time_is_valid(&t)) {
        KT_LOG_WARN("DS1302 init: INVALID_TIME");
        return 0;
    }

    KT_LOG_INFO("DS1302 init: OK, time=20%02u-%02u-%02u %02u:%02u:%02u",
                t.year, t.month, t.day, t.hour, t.minute, t.second);
    return 1;
}

void kt_ds1302_print_time(void)
{
    kt_ds1302_time_t t;
    kt_ds1302_read_time(&t);
    if (!kt_ds1302_time_is_valid(&t)) {
        KT_LOG_WARN("DS1302 time: INVALID_TIME");
        return;
    }
    KT_LOG_INFO("DS1302 time: 20%02u-%02u-%02u %02u:%02u:%02u",
                t.year, t.month, t.day, t.hour, t.minute, t.second);
}
