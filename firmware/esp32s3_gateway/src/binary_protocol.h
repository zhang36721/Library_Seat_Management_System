#ifndef BINARY_PROTOCOL_H
#define BINARY_PROTOCOL_H

#include <Arduino.h>

constexpr uint8_t KT_BIN_SOF1 = 0xA5;
constexpr uint8_t KT_BIN_SOF2 = 0x5A;
constexpr uint8_t KT_BIN_EOF = 0x0D;
constexpr uint8_t KT_BIN_VERSION = 0x01;
constexpr size_t KT_BIN_MAX_PAYLOAD = 128;

constexpr uint8_t KT_MSG_PING = 0x01;
constexpr uint8_t KT_MSG_PONG = 0x02;
constexpr uint8_t KT_MSG_ACK = 0x03;
constexpr uint8_t KT_MSG_ERR = 0x04;
constexpr uint8_t KT_MSG_HEARTBEAT = 0x05;
constexpr uint8_t KT_MSG_HEARTBEAT_ACK = 0x06;
constexpr uint8_t KT_MSG_WIFI_STATUS = 0x10;
constexpr uint8_t KT_MSG_CARD_EVENT = 0x20;
constexpr uint8_t KT_MSG_ACCESS_LOG = 0x21;
constexpr uint8_t KT_MSG_DEVICE_STATUS = 0x30;
constexpr uint8_t KT_MSG_BOOT_SYNC = 0x31;
constexpr uint8_t KT_MSG_SYNC_ACK = 0x32;

constexpr uint8_t KT_ERR_CRC_FAIL = 1;
constexpr uint8_t KT_ERR_LEN_ERR = 2;
constexpr uint8_t KT_ERR_TYPE_ERR = 3;
constexpr uint8_t KT_ERR_FORMAT_ERR = 4;

struct KtFrame {
    uint8_t type = 0;
    uint16_t seq = 0;
    uint16_t len = 0;
    uint8_t payload[KT_BIN_MAX_PAYLOAD]{};
};

uint16_t kt_crc16_modbus(const uint8_t *data, size_t len);
const char *kt_msg_type_name(uint8_t type);
uint16_t kt_get_u16_le(const uint8_t *p);
void kt_put_u16_le(uint8_t *p, uint16_t v);
void kt_put_u32_le(uint8_t *p, uint32_t v);

#endif
