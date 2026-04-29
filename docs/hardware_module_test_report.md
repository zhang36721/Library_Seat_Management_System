# v0.8.4 主控本地菜单功能闭环测试报告

本文档记录 STM32 主控外设模块、seat_node_stm32 座位端采集模块、两端 ZigBee 串口透传链路，以及 v0.8.2 主控本地刷卡与显示闭环。USART2 debug 协议保持独立，不接后端、不接前端。

实机结果必须由烧录后的真实硬件测试补充；当前代码侧已提供命令、日志和 Keil 编译产物。

## 1. 固件与编译

| 项目 | 结果 |
|------|------|
| 主控固件版本 | v0.8.4 |
| 座位端固件版本 | v0.8 |
| 主控 Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| 座位端 Debug 串口 | USART2，115200，`FF CMD DATA FF` |
| 主控 ZigBee 串口 | USART1，115200 |
| 座位端 ZigBee 串口 | USART1，115200 |
| ESP32S3 测试串口 | 主控 USART3，115200；ESP32S3 侧 GPIO47 TX / GPIO48 RX |
| Keil Rebuild All | 主控 0 Error / 0 Warning；座位端 0 Error / 0 Warning |

## 2. 主控上电自检

主控 `main.c` 在 HAL/CubeMX 初始化、USART2 debug 接收启动之后调用 `kt_modules_init()`。该入口只做硬件初始化和状态打印，不接业务逻辑。

| 模块 | 上电行为 | 预期日志 | 实机结果 |
|------|----------|----------|----------|
| RC522 | 初始化软件 SPI，并读取版本寄存器探测 | `RC522 init: OK, version=0x..` 或 `RC522 init: PROBE_FAIL, version=0x..` | 已实测可读 UID：`BF A4 A5 1F BCC=A1`；无卡提示 `RC522 no card detected` |
| OLED | 初始化 I2C OLED，显示 `STM32 MAIN` / `v0.8.2` | `OLED init: OK` 或 `OLED init: ACK_FAIL` | 已实测初始化 OK，屏幕可显示 |
| DS1302 | 读取一次时间并校验 BCD 转换后范围 | `DS1302 init: OK, time=...` 或 `DS1302 init: INVALID_TIME` | 已实测合法：`2026-04-28 12:59:36` |
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
| DS1302 | `FF 62 YY FF` | 设置待写入年份 | `YY` 为 HEX 数据，范围 0~99，表示 20YY |
| DS1302 | `FF 63 MM FF` | 设置待写入月份 | 范围 1~12，非法值拒绝 |
| DS1302 | `FF 64 DD FF` | 设置待写入日期 | 范围 1~31，非法值拒绝 |
| DS1302 | `FF 65 hh FF` | 设置待写入小时 | 范围 0~23，非法值拒绝 |
| DS1302 | `FF 66 mm FF` | 设置待写入分钟 | 范围 0~59，非法值拒绝 |
| DS1302 | `FF 67 ss FF` | 设置待写入秒 | 范围 0~59，非法值拒绝 |
| DS1302 | `FF 68 00 FF` | 提交写入 DS1302 | 校验缓存合法后写入 |
| DS1302 | `FF 69 00 FF` | 打印待写入时间缓存 | 只打印，不写入 |
| 步进电机 | `FF 70 00 FF` | 正转测试 | 电机正转一段后停止 |
| 步进电机 | `FF 71 00 FF` | 反转测试 | 电机反转一段后停止 |
| 步进电机 | `FF 72 00 FF` | 停止并关闭线圈 | 线圈关闭，不再动作 |
| ZigBee | `FF 80 00 FF` | USART1 发送测试字符串 | `USART1 ZigBee test TX: OK` |
| ZigBee | `FF 81 00 FF` | 打印 USART1 最近接收缓冲 | `ZigBee recent RX: ...` 或提示暂无数据 |
| ZigBee | `FF 82 00 FF` | USART1 发送 PING 测试帧 | `ZigBee TX: MAIN,PING,1` |
| ESP32S3 | `FF 90 00 FF` | USART3 发送测试字符串 | `USART3 ESP32S3 test TX: OK` |
| 主控本地业务 | `FF A0 00 FF` | 打印当前主控业务状态 | 打印模拟座位状态和最近刷卡结果 |
| 主控本地业务 | `FF A1 00 FF` | 执行一次本地刷卡流程 | 有卡打印 UID/时间/事件，OLED 显示成功；无卡显示失败 |
| 主控本地业务 | `FF A2 00 FF` | OLED 显示首页 | `KENTO LIB`、时间、S1/S2/S3 模拟状态 |
| 主控本地业务 | `FF A3 00 FF` | OLED 显示最近一次刷卡结果 | 显示最近成功/失败结果 |
| 主控本地业务 | `FF A4 00 FF` | 蜂鸣器成功提示 | 80ms 短响 |
| 主控本地业务 | `FF A5 00 FF` | 蜂鸣器失败提示 | 200ms 短响 |
| 主控本地业务 | `FF A6 00 FF` | 模拟座位状态变化 | S1/S2/S3 模拟 FREE/OCCUPIED 切换 |
| 按键 | `FF B0 00 FF` | 打印 8 路独立按键映射 | 打印 K1~K8 与 PA/PB/PC 引脚关系 |
| 按键 | `FF B1 00 FF` | 打印 K1~K8 原始电平 | 例如 `K2 raw=0 pressed=1` |
| 按键 | `FF B2 00 FF` | 打印最近一次按键事件 | 例如 `Last key: K2 SHORT` |
| 菜单 | `FF B3 00 FF` | 进入时间设置界面 | OLED 显示 `TIME SET` |
| 菜单 | `FF B4 00 FF` | 进入 IC 卡注册界面 | OLED 显示 `CARD ADD` |
| 菜单 | `FF B5 00 FF` | 进入 IC 卡删除界面 | OLED 显示 `CARD DEL` |
| 本地卡表 | `FF B6 00 FF` | 打印本地已注册卡列表 | RAM 卡表最多 10 张卡 |
| 本地卡表 | `FF B7 00 FF` | 清空 RAM 卡表 | 清空后未注册卡刷卡会被拒绝 |

## 4. v0.8.2 主控本地刷卡闭环

本阶段不依赖 ZigBee，不依赖 ESP32S3，不接后端和前端。主控本地闭环流程为：

`RC522 读卡 UID -> DS1302 读取当前时间 -> OLED 显示结果 -> 蜂鸣器提示 -> USART2 debug 打印事件`

成功流程预期日志：

```text
[INFO] CMD 0xA1: Local card flow test
[INFO] CARD UID: BF A4 A5 1F
[INFO] TIME: 2026-04-28 12:59:36
[INFO] CARD EVENT: CHECK_IN_TEST OK
```

失败流程预期日志：

```text
[INFO] CMD 0xA1: Local card flow test
[WARN] RC522 no card detected
[WARN] CARD EVENT: NO_CARD
```

OLED 首页：

```text
KENTO LIB
TIME 12:59
S1 FREE
S2 OCC S3 FREE
```

OLED 刷卡成功：

```text
CARD OK
BFA4A51F
12:59:36
```

OLED 刷卡失败：

```text
CARD FAIL
NO CARD
```

Stepper 当前不作为 v0.8.2 阻塞项。当前记录：`FF 70 / FF 71 / FF 72` 命令链路正常，ULN2003 四个指示灯有灯效；电机本体不转，后续复查供电、相序、线序、驱动板和电机接口。

## 5. v0.8.3 8 路独立按键与 OLED 菜单

当前按键实物为 8 位独立按键模块，不是矩阵键盘。模块 VCC 接 STM32 3.3V，GND 接 STM32 GND。K1~K8 均配置为 GPIO 输入上拉，默认低电平按下。

| 按键 | STM32 引脚 | 功能 |
|------|------------|------|
| K1 | PA1 | MENU / 进入菜单 / 长按返回 |
| K2 | PA8 | UP / 数值加 |
| K3 | PA11 | DOWN / 数值减 |
| K4 | PA12 | LEFT / 上一个字段 |
| K5 | PA15 | RIGHT / 下一个字段 |
| K6 | PB3 | OK / 确认 |
| K7 | PB4 | CARD ADD / 注册 IC 卡 |
| K8 | PC14 | CARD DEL / 删除 IC 卡 |

PA15 / PB3 / PB4 已通过关闭 JTAG、保留 SWD 释放。若 PC14 被 32.768k 晶振占用，K8 改用 PC15。

OLED 菜单入口：

```text
> CARD TEST
  TIME SET
  CARD ADD
  CARD DEL
```

按键任务在主循环任务调度中每 10ms 扫描一次，消抖 20ms，长按阈值 800ms。不在中断中做按键业务。

ESP32S3 接线规划同步调整：STM32 PB10 USART3_TX -> ESP32S3 GPIO48 UART_RX；STM32 PB11 USART3_RX <- ESP32S3 GPIO47 UART_TX；USART2 debug 保持独立。

## 6. v0.8.4 按键菜单功能闭环

v0.8.4 将菜单 placeholder 补成真实本地功能，不依赖 ZigBee、不依赖 ESP32S3、不接后端和前端。

| 功能 | 操作 | 预期 OLED | 预期日志 |
|------|------|-----------|----------|
| 时间设置提交 | TIME SET 界面 K6 | `TIME SAVED` / 日期 / 时间 | `TIME SET: OK` |
| 时间设置非法 | TIME SET 界面 K6，非法时间 | `TIME FAIL` / `INVALID` | `TIME SET: INVALID_TIME` |
| 注册卡成功 | CARD ADD 界面 K6 | `CARD ADD` / `OK` / UID | `CARD ADD: OK` |
| 重复注册 | CARD ADD 界面 K6，同一张卡 | `CARD ADD` / `EXISTS` / UID | `CARD ADD: EXISTS` |
| 注册无卡 | CARD ADD 界面 K6，无卡 | `CARD ADD` / `NO CARD` | `CARD ADD: NO_CARD` |
| 删除卡成功 | CARD DEL 界面 K6 | `CARD DEL` / `OK` / UID | `CARD DEL: OK` |
| 删除未找到 | CARD DEL 界面 K6，未注册卡 | `CARD DEL` / `NOT FOUND` / UID | `CARD DEL: NOT_FOUND` |
| 刷卡已注册 | `FF A1 00 FF` | `CARD OK` / UID / 时间 | `CARD EVENT: CHECK_IN OK UID=...` |
| 刷卡未注册 | `FF A1 00 FF` | `CARD DENIED` / UID | `CARD EVENT: DENIED UID=...` |

本地卡表当前为 RAM 表，容量 10 张卡，断电不保留。后续如需要持久化，再迁移到 Flash 或外部存储。

## 7. ZigBee CC2530 透传测试

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

## 8. seat_node_stm32 v0.8

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

## 9. 待实机填写记录

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
| 主控本地刷卡成功 | `FF A1 00 FF`，有卡 | 待填写 | OLED 显示 `CARD OK`、UID、时间；蜂鸣器 80ms | 待验证 |
| 主控本地刷卡失败 | `FF A1 00 FF`，无卡 | 待填写 | OLED 显示 `CARD FAIL` / `NO CARD`；蜂鸣器 200ms | 待验证 |
| 主控 OLED 首页 | `FF A2 00 FF` | 待填写 | OLED 显示首页和模拟座位状态 | 待验证 |
| 主控最近刷卡结果 | `FF A3 00 FF` | 待填写 | OLED 显示最近一次刷卡结果 | 待验证 |
| 按键原始电平 | `FF B1 00 FF` | 待填写 | K1~K8 raw/pressed 与实物一致 | 待验证 |
| 按键短按事件 | K1~K8 短按后 `FF B2 00 FF` | 待填写 | 可识别 `Kx SHORT` | 待验证 |
| K1 长按事件 | K1 长按后 `FF B2 00 FF` | 待填写 | 可识别 `K1 LONG` | 待验证 |
| OLED 菜单 | K1 / K2 / K3 / K4 / K5 / K6 | 待填写 | 可进入菜单、上下选择、切换字段、确认 | 待验证 |
| IC 卡菜单入口 | K7 / K8 或 `FF B4/B5 00 FF` | 待填写 | 可进入 `CARD ADD` / `CARD DEL` 页面 | 待验证 |
| 时间设置写入 | TIME SET 界面 K6 | 待填写 | `FF 60 00 FF` 可读回新时间 | 待验证 |
| IC 卡注册 | CARD ADD 界面 K6 | 待填写 | 卡表出现 UID，重复注册提示 EXISTS | 待验证 |
| IC 卡删除 | CARD DEL 界面 K6 | 待填写 | 卡表删除 UID，未找到提示 NOT FOUND | 待验证 |
| 卡表打印 | `FF B6 00 FF` | 待填写 | 可打印本地已注册卡列表 | 待验证 |
| 卡表清空 | `FF B7 00 FF` | 待填写 | 卡表清空 | 待验证 |
| 卡表刷卡判断 | `FF A1 00 FF` | 待填写 | 未注册 DENIED，已注册 CARD OK | 待验证 |

## 10. 当前结论

当前阶段可以作为 v0.8.4 固件候选：主控已具备时间设置写入、RAM 卡表注册/删除、卡表打印/清空，以及刷卡流程按本地卡表放行或拒绝。最终是否关闭版本，以实机菜单 K6 操作、`FF A1`、`FF B6`、`FF B7` 验收结果为准。
