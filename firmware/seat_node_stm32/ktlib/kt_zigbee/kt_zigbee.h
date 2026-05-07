#ifndef KT_ZIGBEE_H
#define KT_ZIGBEE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KT_ZIGBEE_FRAME_HEAD      0xFAU
#define KT_ZIGBEE_FRAME_TAIL      0xF5U
#define KT_ZIGBEE_MAX_PAYLOAD_LEN 80U

typedef void (*kt_zigbee_rx_callback_t)(uint16_t addr,
                                        const uint8_t *data,
                                        uint8_t len);

void kt_zigbee_init(UART_HandleTypeDef *uart, kt_zigbee_rx_callback_t callback);
HAL_StatusTypeDef kt_zigbee_send_to(uint16_t dst_addr,
                                    const uint8_t *data,
                                    uint8_t len);
void kt_zigbee_rx_byte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* KT_ZIGBEE_H */
