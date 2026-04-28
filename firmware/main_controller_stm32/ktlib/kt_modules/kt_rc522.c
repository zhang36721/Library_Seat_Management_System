#include "kt_rc522.h"
#include "kt_config.h"
#include "kt_log.h"
#include "stm32f1xx_hal.h"

#define RC522_COMMAND_REG       0x01U
#define RC522_COMM_IE_N_REG     0x02U
#define RC522_COMM_IRQ_REG      0x04U
#define RC522_ERROR_REG         0x06U
#define RC522_FIFO_DATA_REG     0x09U
#define RC522_FIFO_LEVEL_REG    0x0AU
#define RC522_CONTROL_REG       0x0CU
#define RC522_BIT_FRAMING_REG   0x0DU
#define RC522_MODE_REG          0x11U
#define RC522_TX_CONTROL_REG    0x14U
#define RC522_TX_ASK_REG        0x15U
#define RC522_T_MODE_REG        0x2AU
#define RC522_T_PRESCALER_REG   0x2BU
#define RC522_T_RELOAD_REG_H    0x2CU
#define RC522_T_RELOAD_REG_L    0x2DU

#define RC522_CMD_IDLE          0x00U
#define RC522_CMD_TRANSCEIVE    0x0CU
#define RC522_CMD_SOFT_RESET    0x0FU

#define PICC_REQIDL             0x26U
#define PICC_ANTICOLL           0x93U

static void rc522_delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 20U; i++) {
    }
}

static void rc522_cs(uint8_t selected)
{
    HAL_GPIO_WritePin(KT_RC522_NSS_PORT, KT_RC522_NSS_PIN,
                      selected ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static uint8_t rc522_spi_transfer(uint8_t data)
{
    uint8_t rx = 0;
    uint8_t i;

    for (i = 0; i < 8U; i++) {
        HAL_GPIO_WritePin(KT_RC522_MOSI_PORT, KT_RC522_MOSI_PIN,
                          (data & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        data <<= 1;
        HAL_GPIO_WritePin(KT_RC522_SCK_PORT, KT_RC522_SCK_PIN, GPIO_PIN_SET);
        rc522_delay();
        rx <<= 1;
        if (HAL_GPIO_ReadPin(KT_RC522_MISO_PORT, KT_RC522_MISO_PIN) == GPIO_PIN_SET) {
            rx |= 0x01U;
        }
        HAL_GPIO_WritePin(KT_RC522_SCK_PORT, KT_RC522_SCK_PIN, GPIO_PIN_RESET);
        rc522_delay();
    }

    return rx;
}

static void rc522_write_reg(uint8_t reg, uint8_t value)
{
    rc522_cs(1);
    (void)rc522_spi_transfer((uint8_t)((reg << 1U) & 0x7EU));
    (void)rc522_spi_transfer(value);
    rc522_cs(0);
}

static uint8_t rc522_read_reg(uint8_t reg)
{
    uint8_t value;

    rc522_cs(1);
    (void)rc522_spi_transfer((uint8_t)(((reg << 1U) & 0x7EU) | 0x80U));
    value = rc522_spi_transfer(0x00U);
    rc522_cs(0);

    return value;
}

static void rc522_set_bits(uint8_t reg, uint8_t mask)
{
    rc522_write_reg(reg, (uint8_t)(rc522_read_reg(reg) | mask));
}

static void rc522_clear_bits(uint8_t reg, uint8_t mask)
{
    rc522_write_reg(reg, (uint8_t)(rc522_read_reg(reg) & (uint8_t)(~mask)));
}

static uint8_t rc522_transceive(const uint8_t *tx, uint8_t tx_len,
                                uint8_t *rx, uint8_t *rx_len, uint8_t *valid_bits)
{
    uint16_t timeout = 2000U;
    uint8_t irq;
    uint8_t i;
    uint8_t fifo_level;

    rc522_write_reg(RC522_COMMAND_REG, RC522_CMD_IDLE);
    rc522_write_reg(RC522_COMM_IRQ_REG, 0x7FU);
    rc522_set_bits(RC522_FIFO_LEVEL_REG, 0x80U);

    for (i = 0; i < tx_len; i++) {
        rc522_write_reg(RC522_FIFO_DATA_REG, tx[i]);
    }

    rc522_write_reg(RC522_COMMAND_REG, RC522_CMD_TRANSCEIVE);
    rc522_set_bits(RC522_BIT_FRAMING_REG, 0x80U);

    do {
        irq = rc522_read_reg(RC522_COMM_IRQ_REG);
        timeout--;
    } while (timeout && !(irq & 0x30U));

    rc522_clear_bits(RC522_BIT_FRAMING_REG, 0x80U);

    if (!timeout || (rc522_read_reg(RC522_ERROR_REG) & 0x1BU)) {
        return 0;
    }

    fifo_level = rc522_read_reg(RC522_FIFO_LEVEL_REG);
    if (fifo_level > *rx_len) {
        fifo_level = *rx_len;
    }

    for (i = 0; i < fifo_level; i++) {
        rx[i] = rc522_read_reg(RC522_FIFO_DATA_REG);
    }

    *rx_len = fifo_level;
    *valid_bits = rc522_read_reg(RC522_CONTROL_REG) & 0x07U;
    return 1;
}

void kt_rc522_init(void)
{
    HAL_GPIO_WritePin(KT_RC522_RST_PORT, KT_RC522_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(5);
    rc522_write_reg(RC522_COMMAND_REG, RC522_CMD_SOFT_RESET);
    HAL_Delay(50);

    rc522_write_reg(RC522_T_MODE_REG, 0x8DU);
    rc522_write_reg(RC522_T_PRESCALER_REG, 0x3EU);
    rc522_write_reg(RC522_T_RELOAD_REG_L, 30U);
    rc522_write_reg(RC522_T_RELOAD_REG_H, 0U);
    rc522_write_reg(RC522_TX_ASK_REG, 0x40U);
    rc522_write_reg(RC522_MODE_REG, 0x3DU);

    if (!(rc522_read_reg(RC522_TX_CONTROL_REG) & 0x03U)) {
        rc522_set_bits(RC522_TX_CONTROL_REG, 0x03U);
    }

    KT_LOG_INFO("RC522 initialized");
}

uint8_t kt_rc522_read_uid(uint8_t uid[5])
{
    uint8_t req_cmd = PICC_REQIDL;
    uint8_t anticoll_cmd[2] = { PICC_ANTICOLL, 0x20U };
    uint8_t rx[8] = {0};
    uint8_t rx_len = sizeof(rx);
    uint8_t valid_bits = 0;

    rc522_write_reg(RC522_BIT_FRAMING_REG, 0x07U);
    if (!rc522_transceive(&req_cmd, 1, rx, &rx_len, &valid_bits)) {
        KT_LOG_WARN("RC522 no card detected");
        return 0;
    }

    rx_len = sizeof(rx);
    rc522_write_reg(RC522_BIT_FRAMING_REG, 0x00U);
    if (!rc522_transceive(anticoll_cmd, 2, rx, &rx_len, &valid_bits) || rx_len < 5U) {
        KT_LOG_WARN("RC522 UID read failed");
        return 0;
    }

    uid[0] = rx[0];
    uid[1] = rx[1];
    uid[2] = rx[2];
    uid[3] = rx[3];
    uid[4] = rx[4];
    KT_LOG_INFO("RC522 UID: %02X %02X %02X %02X BCC=%02X",
                uid[0], uid[1], uid[2], uid[3], uid[4]);
    return 1;
}
