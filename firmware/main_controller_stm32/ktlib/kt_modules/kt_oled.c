#include "kt_oled.h"
#include "kt_config.h"
#include "kt_log.h"
#include "stm32f1xx_hal.h"

static const uint8_t font_5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
};

static const uint8_t font_dot[5] = {0x00,0x60,0x60,0x00,0x00};
static const uint8_t font_colon[5] = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t font_gt[5] = {0x41,0x22,0x14,0x08,0x00};

static void oled_delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 30U; i++) {
    }
}

static void scl(uint8_t high)
{
    HAL_GPIO_WritePin(KT_OLED_SCL_PORT, KT_OLED_SCL_PIN, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void sda(uint8_t high)
{
    HAL_GPIO_WritePin(KT_OLED_SDA_PORT, KT_OLED_SDA_PIN, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void i2c_start(void)
{
    sda(1); scl(1); oled_delay();
    sda(0); oled_delay();
    scl(0);
}

static void i2c_stop(void)
{
    sda(0); scl(1); oled_delay();
    sda(1); oled_delay();
}

static uint8_t i2c_write(uint8_t data)
{
    uint8_t i;
    GPIO_PinState ack;

    for (i = 0; i < 8U; i++) {
        sda((data & 0x80U) != 0U);
        data <<= 1;
        scl(1); oled_delay();
        scl(0); oled_delay();
    }
    sda(1);
    scl(1); oled_delay();
    ack = HAL_GPIO_ReadPin(KT_OLED_SDA_PORT, KT_OLED_SDA_PIN);
    scl(0);
    return (ack == GPIO_PIN_RESET) ? 1U : 0U;
}

static uint8_t oled_write(uint8_t control, uint8_t data)
{
    uint8_t ok;

    i2c_start();
    ok = i2c_write(KT_OLED_I2C_ADDR);
    ok &= i2c_write(control);
    ok &= i2c_write(data);
    i2c_stop();
    return ok;
}

static uint8_t oled_cmd(uint8_t cmd)
{
    return oled_write(0x00U, cmd);
}

static void oled_data(uint8_t data)
{
    (void)oled_write(0x40U, data);
}

static void oled_pos(uint8_t page, uint8_t col)
{
    oled_cmd((uint8_t)(0xB0U + page));
    oled_cmd((uint8_t)(0x00U + (col & 0x0FU)));
    oled_cmd((uint8_t)(0x10U + (col >> 4U)));
}

static void oled_clear(void)
{
    uint8_t page, col;
    for (page = 0; page < 8U; page++) {
        oled_pos(page, 0);
        for (col = 0; col < 128U; col++) {
            oled_data(0x00U);
        }
    }
}

static const uint8_t *font_for(char c)
{
    if (c >= 'A' && c <= 'Z') return font_5x7[1 + c - 'A'];
    if (c >= 'a' && c <= 'z') return font_5x7[1 + c - 'a'];
    if (c >= '0' && c <= '9') return font_5x7[27 + c - '0'];
    if (c == '.') return font_dot;
    if (c == ':') return font_colon;
    if (c == '>') return font_gt;
    return font_5x7[0];
}

static void oled_print(uint8_t page, uint8_t col, const char *text)
{
    uint8_t i;
    oled_pos(page, col);
    while (*text != '\0') {
        const uint8_t *glyph = font_for(*text++);
        for (i = 0; i < 5U; i++) {
            oled_data(glyph[i]);
        }
        oled_data(0x00U);
    }
}

uint8_t kt_oled_init_startup(void)
{
    static const uint8_t init_cmds[] = {
        0xAE,0x20,0x10,0xB0,0xC8,0x00,0x10,0x40,0x81,0x7F,0xA1,0xA6,
        0xA8,0x3F,0xA4,0xD3,0x00,0xD5,0x80,0xD9,0xF1,0xDA,0x12,0xDB,
        0x40,0x8D,0x14,0xAF
    };
    uint8_t i;

    uint8_t ok = 1U;

    for (i = 0; i < sizeof(init_cmds); i++) {
        ok &= oled_cmd(init_cmds[i]);
    }
    if (!ok) {
        KT_LOG_WARN("OLED init: ACK_FAIL");
        return 0;
    }

    oled_clear();
    oled_print(1, 0, "STM32 MAIN");
    oled_print(3, 0, "v0.7.1");
    KT_LOG_INFO("OLED init: OK");
    return 1;
}

void kt_oled_test(void)
{
    (void)kt_oled_init_startup();
    KT_LOG_INFO("OLED test text sent");
}

void kt_oled_clear(void)
{
    oled_clear();
}

void kt_oled_print_line(uint8_t line, const char *text)
{
    if (line > 3U || text == 0) {
        return;
    }

    oled_print((uint8_t)(line * 2U), 0, text);
}
