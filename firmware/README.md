# 嵌入式固件

本目录包含所有嵌入式设备的固件代码。

## 目录结构

| 目录 | 说明 |
|------|------|
| `seat_node_stm32/` | 座位检测节点固件（STM32 + ZigBee） |
| `main_controller_stm32/` | 主控节点固件（STM32 + ZigBee 协调器） |
| `esp32s3_wifi/` | WiFi 网关固件（ESP32S3） |

## 开发环境

### STM32 开发
- **IDE**: Keil MDK / STM32CubeIDE
- **SDK**: STM32CubeF1
- **调试器**: DAPLINK

### ESP32S3 开发
- **框架**: Arduino / ESP-IDF
- **IDE**: VS Code + PlatformIO / Arduino IDE
- **调试器**: 内置 USB 调试

## 编译和烧录

### STM32 固件
1. 使用 Keil 或 STM32CubeIDE 打开项目
2. 编译生成 .hex 或 .bin 文件
3. 使用 DAP-Link 烧录到目标设备

### ESP32S3 固件
1. 使用 PlatformIO 或 Arduino IDE 打开项目
2. 配置串口和开发板
3. 编译并上传到 ESP32S3

## 版本记录

| 版本 | 日期 | 修改内容 | 开发者 |
|------|------|----------|--------|
| V1.0 | TBD | 初始版本 | TBD |