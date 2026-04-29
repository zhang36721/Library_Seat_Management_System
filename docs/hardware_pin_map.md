# v0.8.3 主控硬件引脚资源表

本表用于整理 STM32F103C8T6 主控硬件资源。当前 v0.8.3 重点是 8 路独立按键输入、OLED 本地菜单、USART2 debug 命令和本地外设验证。后端、前端、ESP32S3 上传和 ZigBee 业务协议均不在本阶段实现。

如源码、CubeMX、原理图、实物接线不一致，先在本表标记“冲突待确认”，不强行推进业务代码。

| 模块 | 功能 | 引脚 | 模式 | 有效电平 | 默认状态 | 是否已初始化 | 是否已验证 | 备注 |
|------|------|------|------|----------|----------|--------------|------------|------|
| MCU | 主控芯片 | STM32F103C8T6 | 72MHz HAL 工程 | 不适用 | 上电复位 | 是 | 待实机验证 | 当前 Keil/CubeMX 工程目标芯片 |
| 运行 LED | 运行/调试 LED | PC13 | GPIO 输出推挽 | 低电平点亮 | 高电平，LED 关闭 | 是 | 待 FF 21 验证 | v0.6 默认关闭 heartbeat，避免打断 LED 测试 |
| 用户按键 | 调试按键输入 | PA0 | GPIO 输入上拉 | 暂按低电平按下 | 上拉释放态 | 是 | 待 FF 23 验证 | 上拉/下拉仍需按实物确认；若按下为高电平需改宏 |
| 蜂鸣器 | 短响验证 | PA4 | GPIO 输出推挽 | 待确认，代码暂按高电平响 | 低电平，蜂鸣器关闭 | 是 | 待 FF 22 验证 | 若实物为低电平触发，需修改 `APP_BUZZER_DEFAULT_ACTIVE_LEVEL` 并复测复位不乱响 |
| 8 位独立按键 K1 | MENU / 进入菜单 / 长按返回 | PA1 | GPIO 输入上拉 | 默认低电平按下，`MAIN_KEY_ACTIVE_LEVEL` 可配置 | 上拉释放态 | 是 | 待 FF B1/B2 验证 | 当前实物为 8 路独立按键模块，不是矩阵键盘 |
| 8 位独立按键 K2 | UP / 加 | PA8 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 |  |
| 8 位独立按键 K3 | DOWN / 减 | PA11 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 |  |
| 8 位独立按键 K4 | LEFT / 上一个字段 | PA12 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 |  |
| 8 位独立按键 K5 | RIGHT / 下一个字段 | PA15 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 | 已关闭 JTAG，保留 SWD |
| 8 位独立按键 K6 | OK / 确认 | PB3 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 | 已关闭 JTAG，保留 SWD |
| 8 位独立按键 K7 | CARD ADD / 注册 IC 卡 | PB4 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 | 已关闭 JTAG，保留 SWD |
| 8 位独立按键 K8 | CARD DEL / 删除 IC 卡 | PC14 | GPIO 输入上拉 | 默认低电平按下 | 上拉释放态 | 是 | 待 FF B1/B2 验证 | 若 PC14 被 32.768k 晶振占用，则改用 PC15 |
| 座位检测 1 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT1_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| 座位检测 2 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT2_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| 座位检测 3 | 红外/压力输入 | 待确认 | 待配置为 GPIO 输入 | `APP_SEAT_SENSOR_ACTIVE_LEVEL` | 待确认 | 否 | 待确认 | 已保留 `APP_SEAT3_SENSOR_PORT/PIN`；未确认前不占用具体引脚 |
| RC522 | 软件 SPI SCK/MISO/MOSI/NSS/RST | PA5 / PA6 / PA7 / PB12 / PB13 | GPIO 软件 SPI | 待确认 | NSS 高、RST 高 | 是 | 待 FF 40/41 实机验证 | 避开 PA4；若实物 NSS 接 PA4，则与蜂鸣器冲突待确认 |
| OLED | I2C 显示接口 | PB6 SCL / PB7 SDA | GPIO 软件 I2C 开漏 | 待确认 | SCL/SDA 高 | 是 | 待 FF 50 实机验证 | 当前明确规划为 I2C OLED，地址 `0x78` |
| DS1302 | CLK / DAT / RST | PB0 / PB1 / PB5 | GPIO 三线 | 待确认 | CLK/RST 低，DAT 输入上拉 | 是 | 待 FF 60/61 实机验证 | 只做时间读写验证，不接业务 |
| 步进电机 | ULN2003/ULN12003 IN1-IN4 | PB8 / PB9 / PB14 / PB15 | GPIO 输出 | 高电平驱动 | 全低关闭线圈 | 是 | 待 FF 70/71/72 实机验证 | 相序需按实机确认 |
| USART1 | ZigBee 测试 | PA9 TX / PA10 RX | USART 异步串口 115200 8N1 | 不适用 | 已初始化 | 是 | 待 FF 80/81/82 实机验证 | CC2530 串口透传测试通道，支持 TX/RX 缓冲验证，不跑 ZigBee 业务协议 |
| USART2 | Debug 调试串口 | PA2 TX / PA3 RX | USART 异步串口 115200 8N1 | 不适用 | RX 中断接收 | 是 | 待实机验证 | 当前 `FF CMD DATA FF` 协议固定走 USART2；ESP32S3 暂不共用 |
| USART3 | ESP32S3 测试 | PB10 TX / PB11 RX | USART 异步串口 115200 8N1 | 不适用 | 已初始化 | 是 | 待 FF 90 实机验证 | 对接 ESP32S3 GPIO48 RX / GPIO47 TX，不污染 USART2 debug |

## 8 位独立按键模块接线

当前按键实物为 8 位独立按键模块，不是 3x3 矩阵键盘。模块 VCC 接 STM32 3.3V，GND 接 STM32 GND，不建议接 5V。

| 按键 | STM32 引脚 | 模式 | 功能 |
|------|------------|------|------|
| K1 | PA1 | GPIO 输入上拉 | MENU / 进入菜单 / 长按返回 |
| K2 | PA8 | GPIO 输入上拉 | UP / 数值加 |
| K3 | PA11 | GPIO 输入上拉 | DOWN / 数值减 |
| K4 | PA12 | GPIO 输入上拉 | LEFT / 上一个字段 |
| K5 | PA15 | GPIO 输入上拉 | RIGHT / 下一个字段 |
| K6 | PB3 | GPIO 输入上拉 | OK / 确认 |
| K7 | PB4 | GPIO 输入上拉 | CARD ADD / 注册 IC 卡 |
| K8 | PC14 | GPIO 输入上拉 | CARD DEL / 删除 IC 卡 |

PA15 / PB3 / PB4 默认与 JTAG 复用冲突，固件中已调用 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 关闭 JTAG、保留 SWD。按键默认按低电平有效处理，宏为 `MAIN_KEY_ACTIVE_LEVEL`。

## ESP32S3 USART3 接线补充

| STM32 主控 | 方向 | ESP32S3 |
|------------|------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | -> | GND |

串口参数先统一为 115200 8N1。TX/RX 必须交叉接，STM32 与 ESP32S3 必须共地。USART2 仍然只作为 STM32 debug，不允许 ESP32S3 占用 USART2。需要确认所用 ESP32S3 开发板的 GPIO47 / GPIO48 是否引出且未被板载功能占用。

## ZigBee CC2530 接线补充

| STM32 主控 | 方向 | CC2530 ZigBee 模块 | 说明 |
|------------|------|--------------------|------|
| PA9 USART1_TX | -> | P1.7 RX | TX/RX 必须交叉接 |
| PA10 USART1_RX | <- | P1.6 TX | TX/RX 必须交叉接 |
| GND | -> | GND | 共地 |
| 3.3V | -> | VCC | 只能接 3.3V，不能接 5V |
| GND | -> | P2.0 | P2.0 为休眠控制，串口通信时必须拉低保持唤醒 |

RST 可以先保持模块默认连接，后续如需要再接 STM32 GPIO 控制。P1.3 / P1.1 是射频收发指示，可先不接。

当前 `FF 80 00 FF` 只能证明 STM32 USART1 发送成功，不能证明 ZigBee 模块入网或对端收到。ZigBee 模块的频道、PAN ID、主从/协调器角色、透传参数，需要按模块资料或配置工具单独确认。

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
| USART1 | ZigBee 串口透传测试，115200 | 已初始化，支持 TX/RX 测试，不接业务 |
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
| `FF 40 00 FF` | 初始化 RC522 |
| `FF 41 00 FF` | 读取 RC522 UID |
| `FF 50 00 FF` | OLED 显示测试文字 |
| `FF 60 00 FF` | 读取 DS1302 时间 |
| `FF 61 00 FF` | 写入 DS1302 测试时间 |
| `FF 70 00 FF` | 步进电机正转测试 |
| `FF 71 00 FF` | 步进电机反转测试 |
| `FF 72 00 FF` | 步进电机停止 |
| `FF 80 00 FF` | USART1 ZigBee 测试发送 |
| `FF 81 00 FF` | 打印 USART1 ZigBee 最近接收缓冲 |
| `FF 82 00 FF` | 通过 USART1 ZigBee 发送 `MAIN,PING,1` |
| `FF 90 00 FF` | USART3 ESP32S3 测试发送 |
| `FF B0 00 FF` | 打印 8 路独立按键映射 |
| `FF B1 00 FF` | 打印 K1~K8 原始电平 |
| `FF B2 00 FF` | 打印最近一次按键事件 |
| `FF B3 00 FF` | 进入时间设置界面 |
| `FF B4 00 FF` | 进入 IC 卡注册界面 |
| `FF B5 00 FF` | 进入 IC 卡删除界面 |
