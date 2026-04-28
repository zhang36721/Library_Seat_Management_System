# UART 通信协议

## 物理层

- 波特率：115200
- 数据位：8
- 停止位：1
- 校验位：无

## 数据帧格式

### 帧结构
```
FF LL DD... DD FF
FF: 帧头/帧尾
LL: 数据长度（1字节）
DD: 数据内容
```

### 命令类型

| 命令码 | 命令名称 | 说明 |
|--------|----------|------|
| 0x01 | SEAT_STATUS | 座位状态 |
| 0x02 | CARD_EVENT | 刷卡事件 |
| 0x03 | SYSTEM_INFO | 系统信息 |
| 0x04 | HEARTBEAT | 心跳包 |

### 座位状态帧
```
FF 05 01 SS TT TT TT TT FF
01: 命令码 SEAT_STATUS
SS: 座位状态 (0=空闲, 1=占用)
TT TT TT TT: 时间戳（Unix时间戳）
```

### 刷卡事件帧
```
FF 12 02 UU UU UU UU SS TT TT TT TT FF
02: 命令码 CARD_EVENT
UU UU UU UU: 卡UID
SS: 事件类型 (0=进入, 1=离开)
TT TT TT TT: 时间戳
```

### 心跳包帧
```
FF 02 04 FF
04: 命令码 HEARTBEAT
```

## 通信流程

1. ESP32S3 发送心跳包
2. STM32 响应心跳包
3. STM32 发送座位状态/刷卡事件
4. ESP32S3 解析并上传数据