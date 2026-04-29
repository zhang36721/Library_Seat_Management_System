# 主控与座位端接线布局规划

本文档记录当前 STM32F103C8T6 主控和 seat_node_stm32 座位端的接线规划。v0.8.3 重点是主控 8 路独立按键、OLED 本地菜单、USART2 debug、本地外设验证和串口接线规划，不接后端、不接前端、不做完整业务闭环。

## 1. 主控 STM32 引脚分配

| 功能模块 | STM32 引脚 | 接线规划 | 当前状态 | 备注 |
|----------|------------|----------|----------|------|
| MCU | STM32F103C8T6 | 主控最小系统板 | 已确认 | Flash 64KB，最后 1KB 页用于 boot count |
| 运行 LED | PC13 | 板载/外接 LED | 已初始化 | 低电平点亮，默认高电平关闭 |
| 用户按键 | PA0 | 独立按键输入 | 已初始化，待实测 | 当前按上拉、低电平按下处理 |
| 蜂鸣器 | PA4 | 有源蜂鸣器信号脚 | 已初始化，待实测 | 当前按高电平响处理；与 RC522 NSS 冲突待确认 |
| RC522 SCK | PA5 | 软件 SPI 时钟 | v0.7 已初始化 | 需实机确认接线 |
| RC522 MISO | PA6 | 软件 SPI MISO | v0.7 已初始化 | 需实机确认接线 |
| RC522 MOSI | PA7 | 软件 SPI MOSI | v0.7 已初始化 | 需实机确认接线 |
| RC522 NSS/SDA | PB12 | 软件 SPI 片选 | v0.7 已初始化 | 避开 PA4 蜂鸣器；若实物接 PA4，则冲突待确认 |
| RC522 RST | PB13 | 复位 GPIO | v0.7 已初始化 | 需实机确认接线 |
| OLED SCL | PB6 | 软件 I2C SCL | v0.7 已初始化 | 当前明确为 I2C OLED |
| OLED SDA | PB7 | 软件 I2C SDA | v0.7 已初始化 | 地址默认 `0x78` |
| DS1302 CLK | PB0 | GPIO 输出 | v0.7 已初始化 | 需实机确认接线 |
| DS1302 DAT | PB1 | GPIO 双向 | v0.7 已初始化 | 空闲输入上拉 |
| DS1302 RST | PB5 | GPIO 输出 | v0.7 已初始化 | 需实机确认接线 |
| ULN2003 IN1 | PB8 | 步进电机控制 | v0.7 已初始化 | ULN2003/ULN12003 型号需确认 |
| ULN2003 IN2 | PB9 | 步进电机控制 | v0.7 已初始化 | 需实机确认相序 |
| ULN2003 IN3 | PB14 | 步进电机控制 | v0.7 已初始化 | 需实机确认相序 |
| ULN2003 IN4 | PB15 | 步进电机控制 | v0.7 已初始化 | 需实机确认相序 |

## 2. USART 职责

| 串口 | STM32 引脚 | 职责 | 当前状态 |
|------|------------|------|----------|
| USART1 | PA9 TX / PA10 RX | CC2530 ZigBee 串口透传测试，115200 | v0.7.1 已支持 TX/RX 测试，不代表无线链路已通 |
| USART2 | PA2 TX / PA3 RX | Debug 调试串口，115200，`FF CMD DATA FF` | 当前唯一 debug 命令通道 |
| USART3 | PB10 TX / PB11 RX | ESP32S3 串口测试，115200 | STM32 侧保持 USART3，对接 ESP32S3 GPIO47/GPIO48 |

ESP32S3 暂时不要接 USART2。旧文档里 ESP32S3 占用 PA2/PA3 的描述属于冲突待确认；若实物已经固定，需要后续重新评估 USART 规划。

## 3. 8 位独立按键模块接线规划

当前按键实物为 8 位独立按键模块，不是 3x3 矩阵键盘。模块 VCC 接 STM32 3.3V，GND 接 STM32 GND；不建议接 5V，避免输出电平超过 STM32 IO。

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

按键默认按低电平有效处理：未按下为高电平，按下为低电平。固件保留 `MAIN_KEY_ACTIVE_LEVEL` 宏，若实测为高电平按下，可改为 `GPIO_PIN_SET`。

PA15 / PB3 / PB4 默认与 JTAG 复用冲突，固件初始化时关闭 JTAG、保留 SWD：

```c
__HAL_RCC_AFIO_CLK_ENABLE();
__HAL_AFIO_REMAP_SWJ_NOJTAG();
```

## 4. ZigBee CC2530 接线规划

当前使用 CC2530 串口透传类 ZigBee 模块。主控 STM32 侧当前只证明 USART1 收发能力，尚未证明 ZigBee 模块入网或无线链路已通。

| STM32 主控 | 方向 | CC2530 ZigBee 模块 | 说明 |
|------------|------|--------------------|------|
| PA9 USART1_TX | -> | P1.7 RX | TX/RX 必须交叉接 |
| PA10 USART1_RX | <- | P1.6 TX | TX/RX 必须交叉接 |
| GND | -> | GND | 共地 |
| 3.3V | -> | VCC | 只能接 3.3V，不能接 5V |
| GND | -> | P2.0 | P2.0 为休眠控制，串口通信时必须拉低保持唤醒 |

RST 可以先保持模块默认连接，后续如需要再接 STM32 GPIO 控制。P1.3 / P1.1 是射频收发指示，可先不接。

当前 `FF 80 00 FF` 只能证明 STM32 USART1 发送成功，不能证明 ZigBee 模块入网或对端收到。ZigBee 模块的频道、PAN ID、主从/协调器角色、透传参数，需要按模块资料或配置工具单独确认。

## 5. ESP32S3 USART3 接线规划

STM32 主控侧保持 USART3：PB10 为 USART3_TX，PB11 为 USART3_RX。ESP32S3 侧改用 GPIO47 / GPIO48。

| STM32 主控 | 方向 | ESP32S3 |
|------------|------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | -> | GND |

串口参数先统一为 115200 8N1。TX/RX 必须交叉接，STM32 和 ESP32S3 必须共地。USART2 仍然只作为 STM32 debug，不允许 ESP32S3 占用 USART2。需要确认所用 ESP32S3 开发板的 GPIO47 / GPIO48 是否已经被板载功能占用。

## 6. 基础 I/O 接线

| 模块 | 信号 | STM32 引脚 | 电平规划 | 默认状态 |
|------|------|------------|----------|----------|
| LED | LED-CTRL | PC13 | 低电平点亮 | 高电平关闭 |
| 按键 | KEY | PA0 | 暂按上拉低有效 | 上拉释放态 |
| 蜂鸣器 | BUZZER | PA4 | 暂按高电平响 | 低电平关闭 |

PA0 和 PA4 的有效电平以实机验收为准。PA4 与 RC522 NSS 可能冲突，必须在接线最终确认前保持“冲突待确认”。

## 7. 主控外设接线规划

| 外设 | 接口规划 | 推荐/待确认引脚 | 当前状态 |
|------|----------|-----------------|----------|
| RC522 | 软件 SPI | SCK PA5、MISO PA6、MOSI PA7、NSS PB12、RST PB13 | v0.7.1 上电探测版本寄存器，命令可读 UID |
| OLED | 软件 I2C | SCL PB6、SDA PB7 | v0.7.1 上电显示 `STM32 MAIN` / `v0.7.1` |
| DS1302 | GPIO 三线 | CLK PB0、DAT PB1、RST PB5 | v0.7.1 上电读取并校验时间 |
| 步进电机 | ULN2003/ULN12003 GPIO 四线 | IN1 PB8、IN2 PB9、IN3 PB14、IN4 PB15 | v0.7.1 上电默认 STOP，线圈关闭 |
| ZigBee | USART1 | PA9/PA10 | v0.7.1 支持发送 PING 和最近 RX 缓冲打印 |
| ESP32S3 | USART3 | STM32 PB10/PB11 -> ESP32S3 GPIO48/GPIO47 | v0.8.3 文档规划，UART 测试保留 |

## 8. seat_node_stm32 v0.8 接线规划

| 功能模块 | STM32 引脚 | 接线规划 | 当前状态 | 备注 |
|----------|------------|----------|----------|------|
| MCU | STM32F103C8T6 | 座位端最小系统板 | 已建工程 | 独立于主控编译 |
| USART1_TX | PA9 | ZigBee P1.7 RX | v0.8 已初始化 | 发送座位状态到主控 |
| USART1_RX | PA10 | ZigBee P1.6 TX | v0.8 已初始化 | 接收主控 PING 并可回 PONG |
| USART2_TX | PA2 | USB-TTL RXD | v0.8 已初始化 | 座位端 debug 输出，115200 |
| USART2_RX | PA3 | USB-TTL TXD | v0.8 已初始化 | 座位端 debug 输入，`FF CMD DATA FF` |
| 座位检测 1 | PA0 | 红外/压力检测输出 | v0.8 已初始化 | 当前临时规划，需实机确认 |
| 座位检测 2 | PA1 | 红外/压力检测输出 | v0.8 已初始化 | 当前临时规划，需实机确认 |
| 座位检测 3 | PA4 | 红外/压力检测输出 | v0.8 已初始化 | 当前临时规划，需实机确认 |

seat_node_stm32 当前 active level 为 `GPIO_PIN_RESET`，即低电平判断为 `OCCUPIED`。若实物模块为高电平有效，需要修改 `SEAT_SENSOR_ACTIVE_LEVEL` 后复测。

## 9. USART2 Debug 命令

主控命令：

| 模块 | 命令 | 用途 |
|------|------|------|
| RC522 | `FF 40 00 FF` | 初始化/探测 RC522 |
| RC522 | `FF 41 00 FF` | 读取卡片 UID |
| OLED | `FF 50 00 FF` | 显示测试文字 |
| DS1302 | `FF 60 00 FF` | 读取当前时间 |
| DS1302 | `FF 61 00 FF` | 写入测试时间 `2026-04-28 12:34:00` |
| 步进电机 | `FF 70 00 FF` | 正转测试 |
| 步进电机 | `FF 71 00 FF` | 反转测试 |
| 步进电机 | `FF 72 00 FF` | 停止并关闭线圈 |
| ZigBee | `FF 80 00 FF` | USART1 发送测试字符串 |
| ZigBee | `FF 81 00 FF` | 打印 USART1 最近接收缓冲 |
| ZigBee | `FF 82 00 FF` | USART1 发送 `MAIN,PING,1` |
| ESP32S3 | `FF 90 00 FF` | USART3 发送测试字符串 |
| 按键 | `FF B0 00 FF` | 打印 8 路独立按键映射 |
| 按键 | `FF B1 00 FF` | 打印 K1~K8 原始电平 |
| 按键 | `FF B2 00 FF` | 打印最近一次按键事件 |
| 菜单 | `FF B3 00 FF` | 进入时间设置界面 |
| 菜单 | `FF B4 00 FF` | 进入 IC 卡注册界面 |
| 菜单 | `FF B5 00 FF` | 进入 IC 卡删除界面 |
| 本地卡表 | `FF B6 00 FF` | 打印本地已注册卡列表 |
| 本地卡表 | `FF B7 00 FF` | 清空 RAM 卡表 |

座位端命令：

| 模块 | 命令 | 用途 |
|------|------|------|
| 硬件状态 | `FF 20 00 FF` | 打印座位端硬件状态 |
| USART 角色 | `FF 24 00 FF` | 打印座位端 USART1/USART2 职责 |
| 座位传感器 | `FF 30 00 FF` | 读取 3 路传感器原始电平 |
| 座位传感器 | `FF 31 00 FF` | 打印 3 路 FREE/OCCUPIED 判断 |
| ZigBee | `FF 80 00 FF` | 通过 USART1 发送当前座位状态 |
| ZigBee | `FF 81 00 FF` | 发送 `SN,1,PONG` 测试帧 |

## 10. 确认清单

已确认：

- 主控芯片为 STM32F103C8T6。
- 主控 USART2 用于 debug，115200，协议为 `FF CMD DATA FF`。
- 主控 USART1 规划为 ZigBee CC2530 串口透传测试。
- 主控 USART3 规划为 ESP32S3 测试串口。
- 主控按键模块为 8 路独立 GPIO 输入，不是矩阵键盘。
- ESP32S3 侧串口规划为 GPIO47 UART_TX / GPIO48 UART_RX。
- CC2530 VCC 只能接 3.3V，P2.0 需要接 GND 保持唤醒。

待确认：

- PA0 按键实际上拉/下拉和有效电平。
- PA4 蜂鸣器实际有效电平。
- 主控 3 路座位检测是否仍需保留，还是完全迁移到 seat_node_stm32。
- seat_node_stm32 3 路座位检测真实 GPIO 和 active level。
- RC522、OLED、DS1302、步进电机真实接线。
- ESP32S3 是否已经实物占用 USART2。
- 两个 ZigBee 模块频道、PAN ID、角色和透传参数。

冲突待确认：

- PA4 蜂鸣器与 RC522 NSS/SDA 可能冲突。
- 旧 ESP32S3 接线文档使用 PA2/PA3，与当前 USART2 debug 规划冲突。
# v0.9 ESP32S3 UART Binary Link Wiring

| STM32 Main Controller | Direction | ESP32S3 |
|-----------------------|-----------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | <-> | GND |

USART3 and ESP32S3 UART use `115200 8N1`. USART2 remains the independent STM32
debug port and must not be shared with ESP32S3. v0.9 uses binary frames ending
with tail byte `0x0D`.

v0.9.1 note: GPIO48 may also be connected to a board RGB/WS2812 LED on some
ESP32S3 development boards. Firmware requests GPIO48 low before UART init, but
if UART traffic still causes LED flicker this is a hardware multiplexing
conflict and must be confirmed on the real board.
