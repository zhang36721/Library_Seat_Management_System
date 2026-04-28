# v0.6 主控硬件引脚资源表

本表用于 v0.6 阶段整理 STM32F103C8T6 主控硬件资源。当前目标只验证基础硬件层：GPIO 默认状态、USART2 debug 命令、基础输入输出读取。后端、前端、ESP32S3 上传、ZigBee 业务协议、OLED/RFID/DS1302/电机驱动均不在本阶段实现。

如源码、CubeMX、原理图、实物接线不一致，先在本表标记“冲突待确认”，不强行推进业务代码。

| 模块 | 功能 | 引脚 | 模式 | 有效电平 | 默认状态 | 是否已初始化 | 是否已验证 | 备注 |
|------|------|------|------|----------|----------|--------------|------------|------|
| MCU | 主控芯片 | STM32F103C8T6 | 72MHz HAL 工程 | 不适用 | 上电复位 | 是 | 待实机验证 | 当前 Keil/CubeMX 工程目标芯片 |
| 运行 LED | 运行/调试 LED | PC13 | GPIO 输出推挽 | 低电平点亮 | 高电平，LED 关闭 | 是 | 待 FF 21 验证 | v0.6 默认关闭 heartbeat，避免打断 LED 测试 |
| 用户按键 | 调试按键输入 | PA0 | GPIO 输入上拉 | 暂按低电平按下 | 上拉释放态 | 是 | 待 FF 23 验证 | 上拉/下拉仍需按实物确认；若按下为高电平需改宏 |
| 蜂鸣器 | 短响验证 | PA4 | GPIO 输出推挽 | 待确认，代码暂按高电平响 | 低电平，蜂鸣器关闭 | 是 | 待 FF 22 验证 | 若实物为低电平触发，需修改 `APP_BUZZER_DEFAULT_ACTIVE_LEVEL` 并复测复位不乱响 |
| 座位检测 1 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT1_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| 座位检测 2 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT2_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| 座位检测 3 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT3_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| RC522 | SPI SCK/MISO/MOSI/NSS/RST | 待确认 | SPI/GPIO | 待确认 | 未初始化 | 否 | 未验证 | 本阶段不做 RC522 业务；需确认是否占用 PA4 NSS，与蜂鸣器 PA4 冲突待确认 |
| OLED | I2C 或 SPI 显示接口 | 待确认 | I2C/SPI/GPIO | 待确认 | 未初始化 | 否 | 未验证 | 采购/接线说明为 OLED IIC，但源码未初始化 I2C |
| DS1302 | CLK / DAT / RST | 待确认 | GPIO | 待确认 | 未初始化 | 否 | 未验证 | 本阶段不做时间业务 |
| 步进电机 | ULN2003/ULN12003 IN1-IN4 | 待确认 | GPIO 输出 | 待确认 | 未初始化 | 否 | 未验证 | 本阶段不做闸机业务；ULN 型号需确认 |
| USART1 | ZigBee 预留 | PA9 TX / PA10 RX | USART 异步串口 | 不适用 | 未初始化 | 否 | 未验证 | 仅预留，不接入 v0.6 debug 协议 |
| USART2 | Debug 调试串口 | PA2 TX / PA3 RX | USART 异步串口 115200 8N1 | 不适用 | RX 中断接收 | 是 | 待实机验证 | 当前 `FF CMD DATA FF` 协议固定走 USART2；ESP32S3 暂不共用 |
| USART3 | ESP32S3 预留 | PB10 TX / PB11 RX | USART 异步串口 | 不适用 | 未初始化 | 否 | 未验证 | 后续 ESP32S3 协议独立于 USART2 debug 协议 |

## 已发现的接线/文档冲突

| 来源 | 当前描述 | v0.6 处理 |
|------|----------|-----------|
| `hardware/wiring/main_controller_wiring.md` | PA2/PA3 USART2 连接 USB-TTL/ESP32S3 | 冲突待确认。v0.6 明确 USART2 只作为 debug 串口，ESP32S3 改为 USART3 预留 |
| `hardware/wiring/esp32s3_wiring.md` | ESP32S3 GPIO44/GPIO43 连接 STM32 PA2/PA3 | 冲突待确认。v0.6 不让 ESP32S3 与 debug 共用 USART2 |
| CubeMX `.ioc` | 仅初始化 PC13、USART2 | 已在 `gpio.c` 手工补 PA0/PA4；未改 `.ioc`，后续如回到 CubeMX 需同步 |
| RC522 常见 SPI1 NSS | PA4 常被用作 SPI1_NSS | 冲突待确认。当前 PA4 已作蜂鸣器 |

## USART 职责

| 串口 | 职责 | 当前状态 |
|------|------|----------|
| USART1 | ZigBee 通信预留 | 未初始化，不接业务 |
| USART2 | Debug 调试串口，115200，`FF CMD DATA FF` 协议 | 已初始化并用于命令验证 |
| USART3 | ESP32S3 通信预留 | 未初始化，不上传数据 |

## v0.6 Debug 验证命令

| 命令 | 用途 |
|------|------|
| `FF 20 00 FF` | 打印硬件资源状态 |
| `FF 21 00 FF` | PC13 LED 测试 |
| `FF 22 00 FF` | PA4 蜂鸣器短响测试 |
| `FF 23 00 FF` | 读取 PA0 按键状态 |
| `FF 24 00 FF` | 打印 USART1/USART2/USART3 职责 |
| `FF 25 00 FF` | 打印当前 GPIO 初始化状态 |
| `FF 30 00 FF` | 读取 3 路座位传感器原始电平 |
| `FF 31 00 FF` | 打印 3 路座位 FREE/OCCUPIED 判断结果 |
