#include "seat_node_app.h"
#include "kt_config.h"
#include "kt_log.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define ZB_RX_BUF_SIZE 96

static volatile uint8_t zb_rx_byte;
static volatile uint8_t zb_rx_len;
static volatile uint8_t zb_line_ready;
static char zb_line[ZB_RX_BUF_SIZE];
static char zb_recent[ZB_RX_BUF_SIZE];

static GPIO_PinState seat_raw(uint8_t index)
{
    switch (index) {
    case 0: return HAL_GPIO_ReadPin(SEAT_SENSOR1_PORT, SEAT_SENSOR1_PIN);
    case 1: return HAL_GPIO_ReadPin(SEAT_SENSOR2_PORT, SEAT_SENSOR2_PIN);
    default: return HAL_GPIO_ReadPin(SEAT_SENSOR3_PORT, SEAT_SENSOR3_PIN);
    }
}

static const char *level_str(GPIO_PinState state)
{
    return (state == GPIO_PIN_SET) ? "1" : "0";
}

static const char *seat_status(GPIO_PinState state)
{
    return (state == SEAT_SENSOR_ACTIVE_LEVEL) ? "OCCUPIED" : "FREE";
}

static void zigbee_send(const char *msg)
{
    char log_msg[ZB_RX_BUF_SIZE];
    size_t len = strlen(msg);
    HAL_StatusTypeDef st = HAL_UART_Transmit(&huart1, (uint8_t *)msg,
                                             (uint16_t)len, 100U);
    if (len >= sizeof(log_msg)) {
        len = sizeof(log_msg) - 1U;
    }
    memcpy(log_msg, msg, len);
    log_msg[len] = '\0';
    while (len > 0U && (log_msg[len - 1U] == '\r' || log_msg[len - 1U] == '\n')) {
        log_msg[--len] = '\0';
    }
    KT_LOG_INFO("ZigBee TX: %s", log_msg);
    KT_LOG_INFO("USART1 ZigBee TX: %s", (st == HAL_OK) ? "OK" : "FAIL");
}

void seat_node_app_init(void)
{
    zb_rx_len = 0;
    zb_line_ready = 0;
    zb_recent[0] = '\0';
    (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zb_rx_byte, 1);
    KT_LOG_INFO("Seat node app init: OK");
    KT_LOG_INFO("USART1 ZigBee UART init: OK, link pending");
}

void seat_node_app_task(void)
{
    if (zb_line_ready) {
        zb_line_ready = 0;
        KT_LOG_INFO("ZigBee RX: %s", zb_recent);
        if (strcmp(zb_recent, "MAIN,PING,1") == 0) {
            seat_node_send_zigbee_pong();
        }
    }
}

void seat_node_zigbee_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uint8_t b = zb_rx_byte;
        if (b == '\r' || b == '\n') {
            if (zb_rx_len > 0U) {
                zb_line[zb_rx_len] = '\0';
                memcpy(zb_recent, zb_line, zb_rx_len + 1U);
                zb_line_ready = 1U;
                zb_rx_len = 0U;
            }
        } else if (zb_rx_len < (ZB_RX_BUF_SIZE - 1U)) {
            zb_line[zb_rx_len++] = (char)b;
        } else {
            zb_rx_len = 0U;
        }
        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&zb_rx_byte, 1);
    }
}

void seat_node_print_hardware_status(void)
{
    KT_LOG_INFO("Seat node v0.8 hardware");
    KT_LOG_INFO("Sensor1: PA0 input pull-up");
    KT_LOG_INFO("Sensor2: PA1 input pull-up");
    KT_LOG_INFO("Sensor3: PA4 input pull-up");
    KT_LOG_INFO("ZigBee : USART1 PA9/PA10 115200");
    KT_LOG_INFO("Debug  : USART2 PA2/PA3 115200 FF CMD DATA FF");
}

void seat_node_print_uart_roles(void)
{
    KT_LOG_INFO("USART1: ZigBee transparent test UART");
    KT_LOG_INFO("USART2: Debug UART, FF CMD DATA FF");
}

void seat_node_print_raw_levels(void)
{
    GPIO_PinState s1 = seat_raw(0);
    GPIO_PinState s2 = seat_raw(1);
    GPIO_PinState s3 = seat_raw(2);
    KT_LOG_INFO("RAW: %s,%s,%s", level_str(s1), level_str(s2), level_str(s3));
}

void seat_node_print_status(void)
{
    GPIO_PinState s1 = seat_raw(0);
    GPIO_PinState s2 = seat_raw(1);
    GPIO_PinState s3 = seat_raw(2);
    KT_LOG_INFO("SEAT: %s,%s,%s", seat_status(s1), seat_status(s2), seat_status(s3));
}

void seat_node_send_zigbee_status(void)
{
    char msg[96];
    GPIO_PinState s1 = seat_raw(0);
    GPIO_PinState s2 = seat_raw(1);
    GPIO_PinState s3 = seat_raw(2);

    snprintf(msg, sizeof(msg), "SN,%u,RAW,%s,%s,%s\r\n",
             SEAT_NODE_ID, level_str(s1), level_str(s2), level_str(s3));
    zigbee_send(msg);

    snprintf(msg, sizeof(msg), "SN,%u,SEAT,%s,%s,%s\r\n",
             SEAT_NODE_ID, seat_status(s1), seat_status(s2), seat_status(s3));
    zigbee_send(msg);
}

void seat_node_send_zigbee_pong(void)
{
    char msg[32];
    snprintf(msg, sizeof(msg), "SN,%u,PONG\r\n", SEAT_NODE_ID);
    zigbee_send(msg);
}
