#ifndef CONFIG_H
#define CONFIG_H

// WiFi 配置
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// 后端 API 配置
#define API_URL "http://your-server-ip:8000"

// UART 配置
#define UART_BAUD_RATE 115200
#define UART_PORT 2

// 网络配置
#define MAX_RETRY_COUNT 3
#define RETRY_DELAY_MS 5000
#define CONNECTION_TIMEOUT_MS 10000

// 设备配置
#define DEVICE_ID "esp32s3_001"

// 日志配置
#define LOG_LEVEL LOG_DEBUG

#endif /* CONFIG_H */