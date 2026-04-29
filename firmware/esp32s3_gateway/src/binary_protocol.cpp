#include "binary_protocol.h"

uint16_t kt_crc16_modbus(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x0001) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001) : static_cast<uint16_t>(crc >> 1);
        }
    }
    return crc;
}

const char *kt_msg_type_name(uint8_t type)
{
    switch (type) {
    case KT_MSG_PING: return "PING";
    case KT_MSG_PONG: return "PONG";
    case KT_MSG_ACK: return "ACK";
    case KT_MSG_ERR: return "ERR";
    case KT_MSG_HEARTBEAT: return "HEARTBEAT";
    case KT_MSG_HEARTBEAT_ACK: return "HEARTBEAT_ACK";
    case KT_MSG_WIFI_STATUS: return "WIFI_STATUS";
    case KT_MSG_CARD_EVENT: return "CARD_EVENT";
    case KT_MSG_ACCESS_LOG: return "ACCESS_LOG";
    case KT_MSG_DEVICE_STATUS: return "DEVICE_STATUS";
    case KT_MSG_BOOT_SYNC: return "BOOT_SYNC";
    case KT_MSG_SYNC_ACK: return "SYNC_ACK";
    default: return "UNKNOWN";
    }
}

uint16_t kt_get_u16_le(const uint8_t *p)
{
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

void kt_put_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>(v >> 8);
}

void kt_put_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}
