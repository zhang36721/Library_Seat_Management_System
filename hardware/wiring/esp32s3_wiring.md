# ESP32S3 网关接线图

ESP32S3 负责接收 STM32 主控 USART3 二进制协议，并主动 HTTP 上报后端。ESP32S3 不占用 STM32 USART2 debug。

| 模块 | 功能 | 模块引脚 | MCU 引脚 | 方向 | 电平 | 默认状态 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| STM32 主控 | USART3_RX | ESP32 GPIO47 UART_TX | STM32 PB11 USART3_RX | ESP32 -> STM32 | 3.3V TTL, 115200 8N1 | 空闲高 | TX/RX 交叉 |
| STM32 主控 | USART3_TX | ESP32 GPIO48 UART_RX | STM32 PB10 USART3_TX | STM32 -> ESP32 | 3.3V TTL, 115200 8N1 | 空闲高 | GPIO48 可能与板载 RGB LED 复用，实机需确认 |
| STM32 主控 | 共地 | ESP32 GND | STM32 GND | 双向 | 0V | 共地 | 必须共地，否则 UART 不稳定 |
| ESP32S3 开发板 | 供电 | USB / 5V / 3.3V | 外部电源 | 输入 | 按开发板要求 | 稳定供电 | 推荐使用开发板 USB 供电；不要把 5V 信号接 STM32 IO |
| 板载 LED | RGB / PWR LED | GPIO48 或 PWR | 板载 | 输出/固定 | 依板卡 | 尝试关闭 | PWR LED 通常无法软件关闭；RGB 若占 GPIO48，可能干扰 UART_RX |
| WiFi | 3408 网络 | `B4-3408_2.4G` | ESP32 WiFi | 无线 | 2.4GHz | 自动连接 | 后端地址 `http://192.168.1.100:18080` |
| WiFi | 301 网络 | `301` | ESP32 WiFi | 无线 | 2.4GHz | 自动连接 | 后端地址 `http://192.168.141.236:18080` |
| WiFi | 310 网络 | `310` | ESP32 WiFi | 无线 | 2.4GHz | 自动连接 | 后端地址 `http://192.168.223.93:18080` |
| HTTP 上报 | 后端服务 | `SERVER_BASE_URL` | ESP32 HTTPClient | ESP32 -> 后端 | TCP/HTTP | 失败不阻塞 UART | 不能写 `localhost` 或 `127.0.0.1` |
| HTTP 上报 | 正式域名预留 | `www.kento.top` | ESP32 HTTPClient | ESP32 -> 后端 | TCP/HTTP | 暂不启用 | 正式版再切换 |
