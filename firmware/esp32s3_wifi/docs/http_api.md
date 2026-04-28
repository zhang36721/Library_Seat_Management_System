# HTTP API 接口

## 基础配置

- 协议：HTTP/1.1
- 方法：POST
- Content-Type: application/json
- 编码：UTF-8

## API 端点

### 1. 上传座位状态

**URL**: `/api/seat/status`

**请求体**:
```json
{
    "seat_id": 1,
    "status": 1,
    "timestamp": 1714275600,
    "sensor_type": "ir+pressure"
}
```

**参数说明**:
- seat_id: 座位编号（1-3）
- status: 座位状态（0=空闲, 1=占用）
- timestamp: Unix 时间戳
- sensor_type: 传感器类型

**成功响应** (200 OK):
```json
{
    "success": true,
    "message": "Seat status updated"
}
```

### 2. 上传刷卡事件

**URL**: `/api/card/event`

**请求体**:
```json
{
    "uid": "AABBCCDD",
    "event_type": "enter",
    "seat_id": 1,
    "timestamp": 1714275600
}
```

**参数说明**:
- uid: 卡UID（十六进制字符串）
- event_type: 事件类型（enter/leave）
- seat_id: 座位编号
- timestamp: Unix 时间戳

**成功响应** (200 OK):
```json
{
    "success": true,
    "message": "Card event recorded"
}
```

### 3. 上传系统状态

**URL**: `/api/system/status`

**请求体**:
```json
{
    "device_id": "esp32s3_001",
    "wifi_status": "connected",
    "signal_strength": -55,
    "uptime": 3600,
    "timestamp": 1714275600
}
```

**参数说明**:
- device_id: 设备ID
- wifi_status: WiFi状态（connected/disconnected）
- signal_strength: 信号强度（dBm）
- uptime: 运行时间（秒）
- timestamp: Unix 时间戳

**成功响应** (200 OK):
```json
{
    "success": true,
    "message": "System status received"
}
```

### 4. 心跳检测

**URL**: `/api/heartbeat`

**请求体**:
```json
{
    "device_id": "esp32s3_001",
    "timestamp": 1714275600
}
```

**成功响应** (200 OK):
```json
{
    "success": true,
    "server_time": 1714275600
}
```

## 错误响应

```json
{
    "success": false,
    "error": "Error message",
    "code": 400
}
```

## 重试机制

- 网络超时：10秒
- 重试次数：3次
- 重试间隔：5秒