# v0.7 主控外设模块独立验证报告

本报告只记录 STM32 主控外设模块独立验证。USART2 debug 协议保持独立，不接后端、不接前端、不做完整业务流程。实机结果必须由烧录后的真实硬件测试填写；当前代码侧已提供命令、日志和 Keil 编译结果。

## 1. 固件与编译

| 项目 | 结果 |
|------|------|
| 固件版本 | v0.7 |
| Keil Rebuild All | 0 Error / 0 Warning |
| Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| ZigBee 测试串口 | USART1，115200 |
| ESP32S3 测试串口 | USART3，115200 |

## 2. 模块测试顺序

必须按以下顺序验收：

1. RC522
2. OLED
3. DS1302
4. 步进电机
5. ZigBee
6. ESP32S3

## 3. RC522

| 项目 | 内容 |
|------|------|
| 接线 | SCK PA5、MISO PA6、MOSI PA7、NSS PB12、RST PB13 |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_rc522.c` |
| 初始化命令 | `FF 40 00 FF` |
| 读取 UID 命令 | `FF 41 00 FF` |
| 预期日志 | `[INFO] RC522 initialized`；有卡时打印 `RC522 UID: xx xx xx xx BCC=xx` |
| 实机结果 | 待实机填写 |
| 备注 | 若实物 RC522 NSS 接 PA4，则与蜂鸣器冲突，需改线或改宏 |

## 4. OLED

| 项目 | 内容 |
|------|------|
| 接线 | SCL PB6、SDA PB7，软件 I2C，地址 `0x78` |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_oled.c` |
| 测试命令 | `FF 50 00 FF` |
| 预期日志 | `[INFO] OLED test text sent` |
| 预期现象 | OLED 显示 `STM32 MAIN` 和 `V07 OLED` |
| 实机结果 | 待实机填写 |

## 5. DS1302

| 项目 | 内容 |
|------|------|
| 接线 | CLK PB0、DAT PB1、RST PB5 |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_ds1302.c` |
| 读时间命令 | `FF 60 00 FF` |
| 写测试时间命令 | `FF 61 00 FF` |
| 预期日志 | 写入后打印 `DS1302 test time written: 2026-04-28 12:34:00`；读取时打印当前时间 |
| 实机结果 | 待实机填写 |

## 6. 步进电机

| 项目 | 内容 |
|------|------|
| 接线 | IN1 PB8、IN2 PB9、IN3 PB14、IN4 PB15 |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_stepper.c` |
| 正转命令 | `FF 70 00 FF` |
| 反转命令 | `FF 71 00 FF` |
| 停止命令 | `FF 72 00 FF` |
| 预期日志 | `Stepper forward test done`、`Stepper reverse test done`、`Stepper stopped` |
| 实机结果 | 待实机填写 |
| 备注 | 如方向不一致，先调换相序宏或线序，不接业务逻辑 |

## 7. ZigBee

| 项目 | 内容 |
|------|------|
| 接线 | USART1：PA9 TX -> ZigBee RXD，PA10 RX <- ZigBee TXD |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_uart_links.c` |
| 测试命令 | `FF 80 00 FF` |
| 发送内容 | `STM32_MAIN_ZIGBEE_TEST\r\n` |
| 预期日志 | `[INFO] USART1 ZigBee test TX: OK` |
| 实机结果 | 待实机填写 |

## 8. ESP32S3

| 项目 | 内容 |
|------|------|
| 接线 | USART3：PB10 TX -> ESP32S3 RX，PB11 RX <- ESP32S3 TX |
| 驱动文件 | `firmware/main_controller_stm32/ktlib/kt_modules/kt_uart_links.c` |
| 测试命令 | `FF 90 00 FF` |
| 发送内容 | `STM32_MAIN_ESP32S3_TEST\r\n` |
| 预期日志 | `[INFO] USART3 ESP32S3 test TX: OK` |
| 实机结果 | 待实机填写 |
| 备注 | ESP32S3 不共用 USART2 debug |

## 9. USART2 独立性检查

| 检查项 | 结果 |
|--------|------|
| USART2 仍为 debug 命令通道 | 是 |
| ZigBee 是否使用 USART2 | 否，使用 USART1 |
| ESP32S3 是否使用 USART2 | 否，使用 USART3 |
| `FF CMD DATA FF` 是否只走 USART2 | 是 |

## 10. 待实机填写记录

| 模块 | 命令 | 实机日志 | 实机现象 | 结果 |
|------|------|----------|----------|------|
| RC522 | `FF 40 00 FF` | 待填写 | 待填写 | 待验证 |
| RC522 | `FF 41 00 FF` | 待填写 | 待读 UID | 待验证 |
| OLED | `FF 50 00 FF` | 待填写 | 待显示文字 | 待验证 |
| DS1302 | `FF 61 00 FF` / `FF 60 00 FF` | 待填写 | 待读写时间 | 待验证 |
| 步进电机 | `FF 70/71/72 00 FF` | 待填写 | 待转动/停止 | 待验证 |
| ZigBee | `FF 80 00 FF` | 待填写 | 待对端收到测试串 | 待验证 |
| ESP32S3 | `FF 90 00 FF` | 待填写 | 待对端收到测试串 | 待验证 |
