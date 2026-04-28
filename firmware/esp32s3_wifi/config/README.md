# 配置目录

## 用途

本目录用于存放系统配置文件。

## 文件说明

### config.example.h
配置文件模板，包含所有可配置参数

### 配置项

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| WIFI_SSID | WiFi 名称 | "SSID" |
| WIFI_PASSWORD | WiFi 密码 | "PASSWORD" |
| API_URL | 后端 API 地址 | "http://localhost:8000" |
| UART_BAUD_RATE | UART 波特率 | 115200 |
| MAX_RETRY_COUNT | 最大重试次数 | 3 |
| RETRY_DELAY_MS | 重试间隔（毫秒） | 5000 |

## 使用方法

1. 复制 config.example.h 为 config.h
2. 修改配置参数
3. 在程序中包含 config.h

## 注意事项

- config.h 不应提交到版本控制
- 敏感信息（如密码）应通过环境变量设置
- 保持配置文件的安全性