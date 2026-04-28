# v0.7.1/v0.8 硬件模块独立验证报告

本文档只记录 STM32 主控外设模块、seat_node_stm32 座位端采集模块，以及两端 ZigBee 串口透传链路的独立验证。USART2 debug 协议保持独立，不接后端、不接前端、不做完整业务流程。

实机结果必须由烧录后的真实硬件测试补充；当前代码侧已提供命令、日志和 Keil 编译产物。

## 1. 固件与编译

| 项目 | 结果 |
|------|------|
| 主控固件版本 | v0.7.1 |
| 座位端固件版本 | v0.8 |
| 主控 Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| 座位端 Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| 主控 ZigBee 串口 | USART1，115200 |
| 座位端 ZigBee 串口 | USART1，115200 |
| ESP32S3 测试串口 | 主控 USART3，115200 |
| Keil Rebuild All | 主控 0 Error / 0 Warning；座位端 0 Error / 0 Warning |

## 2. 主控上电自检

主控 `main.c` 在 HAL/CubeMX 初始化、USART2 debug 接收启动之后调用 `kt_modules_init()`。该入口只做硬件初始化和状态打印，不接业务逻辑。

| 模块 | 上电行为 | 预期日志 | 实机结果 |
|------|----------|----------|----------|
| RC522 | 初始化软件 SPI，并读取版本寄存器探测 | `RC522 init: OK, version=0x..` 或 `RC522 init: PROBE_FAIL, version=0x..` | 待实机填写 |
| OLED | 初始化 I2C OLED，显示 `STM32 MAIN` / `v0.7.1` | `OLED init: OK` 或 `OLED init: ACK_FAIL` | 待实机填写 |
| DS1302 | 读取一次时间并校验 BCD 转换后范围 | `DS1302 init: OK, time=...` 或 `DS1302 init: INVALID_TIME` | 待实机填写 |
| 步进电机 | 上电默认停止，四路线圈关闭 | `Stepper init: STOP, coils off` | 待实机填写 |
| ZigBee USART1 | 启动 USART1 RX 中断缓冲 | `USART1 ZigBee UART init: OK`，并提示链路待实测 | 待实机填写 |
| ESP32S3 USART3 | 确认 USART3 初始化职责 | `USART3 ESP32S3 UART init: OK`，并提示对端待实测 | 待实机填写 |

注意：USART1/USART3 的 `OK` 只表示 STM32 侧 UART 已初始化，不代表 ZigBee 入网、无线链路可用，或 ESP32S3 对端通信成功。

## 3. 主控 USART2 Debug 命令

| 模块 | 命令 | 用途 | 预期日志/现象 |
|------|------|------|---------------|
| RC522 | `FF 40 00 FF` | 初始化/探测 RC522 | 打印 `OK` 或 `PROBE_FAIL` |
| RC522 | `FF 41 00 FF` | 读取卡片 UID | 有卡时打印 UID |
| OLED | `FF 50 00 FF` | 显示测试文字 | OLED 显示启动文字，日志 `OLED test text sent` |
| DS1302 | `FF 60 00 FF` | 读取当前时间 | 合理时间或 `INVALID_TIME` |
| DS1302 | `FF 61 00 FF` | 写入测试时间 | 写入 `2026-04-28 12:34:00` |
| 步进电机 | `FF 70 00 FF` | 正转测试 | 电机正转一段后停止 |
| 步进电机 | `FF 71 00 FF` | 反转测试 | 电机反转一段后停止 |
| 步进电机 | `FF 72 00 FF` | 停止并关闭线圈 | 线圈关闭，不再动作 |
| ZigBee | `FF 80 00 FF` | USART1 发送测试字符串 | `USART1 ZigBee test TX: OK` |
| ZigBee | `FF 81 00 FF` | 打印 USART1 最近接收缓冲 | `ZigBee recent RX: ...` 或提示暂无数据 |
| ZigBee | `FF 82 00 FF` | USART1 发送 PING 测试帧 | `ZigBee TX: MAIN,PING,1` |
| ESP32S3 | `FF 90 00 FF` | USART3 发送测试字符串 | `USART3 ESP32S3 test TX: OK` |

## 4. ZigBee CC2530 透传测试

接线：

| STM32 主控 | 方向 | CC2530 ZigBee |
|------------|------|---------------|
| PA9 USART1_TX | -> | P1.7 RX |
| PA10 USART1_RX | <- | P1.6 TX |
| GND | -> | GND |
| 3.3V | -> | VCC |
| GND | -> | P2.0 |

测试说明：

- `FF 80 00 FF` 只能证明 STM32 USART1 发送成功，不能证明 ZigBee 模块入网或对端收到。
- `FF 82 00 FF` 发送 `MAIN,PING,1\r\n`。
- 主控收到对端数据时，在主循环打印 `ZigBee RX: ...`。
- `FF 81 00 FF` 打印最近一条 USART1 接收行。
- ZigBee 模块频道、PAN ID、协调器/终端角色、透传参数需要按模块资料或配置工具单独确认。

## 5. seat_node_stm32 v0.8

座位端第一阶段只做最小功能：STM32 初始化、USART2 debug、USART1 ZigBee、3 路传感器输入、FREE/OCCUPIED 判断和文本透传。

| 功能 | 当前实现 |
|------|----------|
| Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| ZigBee 串口 | USART1，115200 |
| 座位检测 1 | PA0，输入上拉，临时规划 |
| 座位检测 2 | PA1，输入上拉，临时规划 |
| 座位检测 3 | PA4，输入上拉，临时规划 |
| Active level | `GPIO_PIN_RESET`，低电平判断为 `OCCUPIED` |

座位端 USART2 debug 命令：

| 命令 | 用途 | 预期日志/数据 |
|------|------|---------------|
| `FF 20 00 FF` | 打印座位端硬件状态 | 版本、传感器引脚、ZigBee 状态 |
| `FF 24 00 FF` | 打印 USART 角色 | USART1 ZigBee，USART2 Debug |
| `FF 30 00 FF` | 读取 3 路传感器原始电平 | `SN,1,RAW,x,y,z` |
| `FF 31 00 FF` | 打印 3 路 FREE/OCCUPIED 判断 | `SN,1,SEAT,...` |
| `FF 80 00 FF` | 通过 ZigBee 发送当前座位状态 | `SN,1,RAW,...` 和 `SN,1,SEAT,...` |
| `FF 81 00 FF` | 发送 ZigBee PONG 测试帧 | `SN,1,PONG` |

座位端收到主控 `MAIN,PING,1` 时，会在主循环打印 `ZigBee RX: MAIN,PING,1` 并回发 `SN,1,PONG`。

## 6. 待实机填写记录

| 模块 | 命令/动作 | 实机日志 | 实机现象 | 结果 |
|------|-----------|----------|----------|------|
| 主控启动 | 上电 | 待填写 | 待确认全部自检日志 | 待验证 |
| RC522 | `FF 40/41 00 FF` | 待填写 | 未接时 PROBE_FAIL，接上后可读 UID | 待验证 |
| OLED | 上电 / `FF 50 00 FF` | 待填写 | 显示 `STM32 MAIN` / `v0.7.1` | 待验证 |
| DS1302 | 上电 / `FF 60/61 00 FF` | 待填写 | 异常时报 INVALID_TIME，写入后可读合理时间 | 待验证 |
| 步进电机 | `FF 70/71/72 00 FF` | 待填写 | 正转、反转、停止，复位不乱动 | 待验证 |
| 主控 ZigBee TX | `FF 82 00 FF` | 待填写 | 座位端收到 `MAIN,PING,1` | 待验证 |
| 主控 ZigBee RX | `FF 81 00 FF` | 待填写 | 可打印座位端最近数据 | 待验证 |
| 座位端传感器 | `FF 30/31 00 FF` | 待填写 | 原始电平和 FREE/OCCUPIED 与实物一致 | 待验证 |
| 座位端 ZigBee TX | `FF 80/81 00 FF` | 待填写 | 主控收到座位状态或 PONG | 待验证 |
| USART2 独立性 | 任意 ZigBee 收发时执行 debug 命令 | 待填写 | ZigBee 数据不污染 USART2 debug 协议 | 待验证 |

## 7. 当前结论

当前阶段可以作为 v0.7.1/v0.8 固件候选：主控已具备上电外设自检入口，座位端已具备基础采集与 ZigBee 文本透传命令。最终是否关闭版本，以 Keil Rebuild All 和实机测试结果为准。
