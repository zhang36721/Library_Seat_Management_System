# 测试记录

## v0.8.4 按键菜单功能闭环记录

### 功能范围

v0.8.4 将 v0.8.3 菜单中的 placeholder 补成主控本地功能闭环：Time Set 写入 DS1302，Card Add/Del 操作 RAM 卡表，FF A1 刷卡流程按本地卡表判断是否放行。本阶段仍不依赖 ZigBee、不依赖 ESP32S3、不接后端和前端。

### 新增/完善功能

| 功能 | 操作 | 预期结果 |
|------|------|----------|
| 时间设置写入 | TIME SET 界面 K6 短按 | 调用 `kt_ds1302_set_time()`，OLED 显示 `TIME SAVED` 或 `TIME FAIL` |
| IC 卡注册 | CARD ADD 界面 K6 短按 | 读取 RC522 UID，加入 RAM 卡表；重复卡显示 `EXISTS`，满表显示 `FULL` |
| IC 卡删除 | CARD DEL 界面 K6 短按 | 读取 RC522 UID，从 RAM 卡表删除；未找到显示 `NOT FOUND` |
| 本地刷卡判定 | `FF A1 00 FF` | 未注册卡显示 `CARD DENIED`，已注册卡显示 `CARD OK` |
| 打印卡表 | `FF B6 00 FF` | 打印本地 RAM 卡表 |
| 清空卡表 | `FF B7 00 FF` | 清空本地 RAM 卡表 |

### 待验收命令

| 命令/操作 | 验收目标 | 预期结果 |
|-----------|----------|----------|
| K6 in TIME SET | 写入 DS1302 | OLED 显示 `TIME SAVED`，`FF 60 00 FF` 可读回新时间 |
| K6 in CARD ADD | 注册当前 IC 卡 | OLED 显示 `CARD ADD / OK / UID` |
| K6 in CARD ADD，同一张卡 | 重复注册保护 | OLED 显示 `CARD ADD / EXISTS / UID` |
| K6 in CARD DEL | 删除当前 IC 卡 | OLED 显示 `CARD DEL / OK / UID` |
| K6 in CARD DEL，未注册卡 | 删除不存在保护 | OLED 显示 `CARD DEL / NOT FOUND / UID` |
| `FF B6 00 FF` | 打印卡表 | USART2 打印已注册 UID |
| `FF B7 00 FF` | 清空卡表 | USART2 打印 `Card DB cleared` |
| `FF A1 00 FF` 未注册卡 | 刷卡拒绝 | OLED 显示 `CARD DENIED`，日志 `CARD EVENT: DENIED` |
| `FF A1 00 FF` 已注册卡 | 刷卡通过 | OLED 显示 `CARD OK`，日志 `CARD EVENT: CHECK_IN OK` |

RAM 卡表当前容量为 10 张卡，不做 Flash 持久化；断电后卡表清空。

## v0.8.3 8 路独立按键与 OLED 菜单记录

### 接线确认

当前按键模块实物为 8 位独立按键模块，不是矩阵键盘。模块 VCC 接 STM32 3.3V，GND 接 STM32 GND，K1~K8 分别接独立 GPIO 输入。

| 按键 | STM32 引脚 | 功能 | 验收命令 |
|------|------------|------|----------|
| K1 | PA1 | MENU / 进入菜单 / 长按返回 | `FF B1/B2 00 FF` |
| K2 | PA8 | UP / 数值加 | `FF B1/B2 00 FF` |
| K3 | PA11 | DOWN / 数值减 | `FF B1/B2 00 FF` |
| K4 | PA12 | LEFT / 上一个字段 | `FF B1/B2 00 FF` |
| K5 | PA15 | RIGHT / 下一个字段 | `FF B1/B2 00 FF` |
| K6 | PB3 | OK / 确认 | `FF B1/B2 00 FF` |
| K7 | PB4 | CARD ADD / 注册 IC 卡 | `FF B1/B2 00 FF` |
| K8 | PC14 | CARD DEL / 删除 IC 卡 | `FF B1/B2 00 FF` |

按键默认低电平有效：未按下 raw=1，按下 raw=0。PA15 / PB3 / PB4 已关闭 JTAG、保留 SWD。

### v0.8.3 待验收命令

| 命令 | 验收目标 | 预期结果 |
|------|----------|----------|
| `FF B0 00 FF` | 打印按键映射 | K1~K8 映射和功能说明正确 |
| `FF B1 00 FF` | 打印原始电平 | K1~K8 raw/pressed 与实物按下状态一致 |
| `FF B2 00 FF` | 打印最近按键事件 | 短按显示 `Kx SHORT`，K1 长按显示 `K1 LONG` |
| `FF B3 00 FF` | 进入时间设置界面 | OLED 显示 `TIME SET` |
| `FF B4 00 FF` | 进入 IC 卡注册界面 | OLED 显示 `CARD ADD` |
| `FF B5 00 FF` | 进入 IC 卡删除界面 | OLED 显示 `CARD DEL` |

### ESP32S3 USART3 接线更新

STM32 主控侧保持 USART3：PB10 为 TX，PB11 为 RX。ESP32S3 侧规划为 GPIO47 UART_TX / GPIO48 UART_RX。

| STM32 主控 | 方向 | ESP32S3 |
|------------|------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | -> | GND |

USART2 debug 保持独立，不允许 ESP32S3 占用 USART2。

## v0.8.2 主控本地刷卡与显示闭环记录

### 已完成实测

| 模块 | 实测结果 | 结论 |
|------|----------|------|
| RC522 有卡读取 | `[INFO] RC522 UID: BF A4 A5 1F BCC=A1` | 通过 |
| RC522 无卡检测 | `[WARN] RC522 no card detected` | 通过 |
| DS1302 启动读时 | `[INFO] DS1302 init: OK, time=2026-04-28 12:59:36` | 通过 |
| OLED | 初始化 OK，屏幕可显示 | 通过 |
| 蜂鸣器 | Debug 命令实测可响 | 通过 |
| 按键 | Debug 命令实测可读 | 通过 |
| Stepper | `FF 70 / FF 71 / FF 72` 命令链路正常，ULN2003 指示灯有灯效，电机本体不转 | 不阻塞 v0.8.2，待复查供电/相序/线序/驱动板/电机接口 |

### v0.8.2 待验收命令

| 命令 | 验收目标 | 预期结果 |
|------|----------|----------|
| `FF A0 00 FF` | 打印当前主控业务状态 | 打印模拟座位状态和最近刷卡结果 |
| `FF A1 00 FF` | 本地刷卡流程测试 | 有卡时读 UID、读 DS1302 时间、OLED 显示成功、蜂鸣器成功提示、USART2 打印事件 |
| `FF A1 00 FF` | 本地刷卡失败测试 | 无卡时 OLED 显示失败、蜂鸣器失败提示、USART2 打印 `CARD EVENT: NO_CARD` |
| `FF A2 00 FF` | OLED 首页 | 显示 `KENTO LIB`、时间、S1/S2/S3 模拟状态 |
| `FF A3 00 FF` | OLED 最近刷卡结果 | 显示最近一次成功或失败结果 |
| `FF A4 00 FF` | 蜂鸣器成功提示 | 80ms 短响 |
| `FF A5 00 FF` | 蜂鸣器失败提示 | 200ms 短响 |
| `FF A6 00 FF` | 模拟座位状态变化 | S1/S2/S3 FREE/OCCUPIED 模拟状态切换 |

### DS1302 时间校准命令

`DATA` 是一个字节，串口工具按 HEX 发送。例如十进制 29 需要发送 `1D`。

| 命令 | 用途 | 范围 |
|------|------|------|
| `FF 62 YY FF` | 设置待写入年份，表示 20YY | 0~99 |
| `FF 63 MM FF` | 设置待写入月份 | 1~12 |
| `FF 64 DD FF` | 设置待写入日期 | 1~31 |
| `FF 65 hh FF` | 设置待写入小时 | 0~23 |
| `FF 66 mm FF` | 设置待写入分钟 | 0~59 |
| `FF 67 ss FF` | 设置待写入秒 | 0~59 |
| `FF 68 00 FF` | 校验并提交写入 DS1302 | 合法时间才写入 |
| `FF 69 00 FF` | 打印当前待写入时间缓存 | 只打印不写入 |

设置 `2026-04-29 11:28:00` 的发送顺序：

```text
FF 62 1A FF
FF 63 04 FF
FF 64 1D FF
FF 65 0B FF
FF 66 1C FF
FF 67 00 FF
FF 69 00 FF
FF 68 00 FF
FF 60 00 FF
```

`FF 61 00 FF` 保留为固定测试写入命令：`2026-04-28 12:34:00`。

## v0.8.5 主控本地通行记录与自动刷卡

本阶段不依赖 ZigBee、不依赖 ESP32S3、不接后端和前端，目标是主控本地真实刷卡体验。

| 验收项 | 操作 | 预期结果 | 实机结果 |
|--------|------|----------|----------|
| OLED 首页 | 上电 | 显示 `KENTO LIBRARY`、`SWIPE CARD`、卡数、日志数和时间 | 待测 |
| 自动刷卡 | 已注册卡靠近 RC522 | 无需 `FF A1`，自动显示 `ACCESS OK` 并记录日志 | 待测 |
| 防重复刷 | 同一张卡持续放在读卡器上 | 不连续重复触发 | 待测 |
| 拿开再刷 | 拿开卡后再次靠近 | 允许下一次刷卡 | 待测 |
| 未注册卡 | 未注册卡靠近 RC522 | OLED 显示 `ACCESS DENIED`，电机不动作 | 待测 |
| 闸门联动 | 已注册卡刷卡成功 | 步进电机开闸、保持、关闸、停止线圈 | 待测 |
| 打印记录 | `FF C0 00 FF` | 打印最近通行记录或 `ACCESS LOG EMPTY` | 待测 |
| 清空记录 | `FF C1 00 FF` | 打印 `ACCESS LOG CLEARED` | 待测 |
| 记录统计 | `FF C2 00 FF` | 打印记录总数、OK 数、DENIED 数 | 待测 |
| 手动刷卡 | `FF A1 00 FF` | 仍可手动执行本地刷卡测试 | 待测 |
| 菜单暂停轮询 | 进入 TIME SET / CARD ADD / CARD DEL / 菜单 | 自动刷卡暂停，放卡不会触发 ACCESS DENIED 或正常通行 | 待测 |
| 退出恢复轮询 | K1 长按返回首页 | 自动刷卡恢复 | 待测 |
| 掉电保护 | 产生记录后断电重启，再发 `FF C0 00 FF` | 最近 50 条记录从 Flash 恢复 | 待测 |
| 卡表掉电保护 | 注册 IC 卡后断电重启，再发 `FF B6 00 FF` | 已注册卡列表从 Flash 恢复 | 待测 |

代码侧 Keil Rebuild All 已通过：`0 Error(s), 0 Warning(s)`。最终关闭版本仍以实机验收结果为准。

本地数据持久化说明：已注册卡表和最近 50 条通行记录共同保存到 STM32 内部 Flash `0x0800F800`，boot count 仍使用 `0x0800FC00`。Keil IROM 已预留最后两页，避免程序区覆盖持久化数据。

### v0.8.2 阶段边界

- 不依赖 ZigBee。
- 不依赖 ESP32S3。
- 不接后端和前端。
- Stepper 保留接口，不作为本阶段阻塞项。

## 1. 测试环境

### 1.1 硬件环境
- 座位检测节点 × N
- 主控节点 × 1
- ESP32S3 网关 × 1
- 测试用 PC

### 1.2 软件环境
- 后端服务版本：v0.1.0
- 前端版本：v0.1.0
- 数据库：MySQL 8.0
- 测试工具：Postman / JMeter

## 2. 测试类型

### 2.1 功能测试
| 测试项 | 测试方法 | 预期结果 | 实际结果 | 测试日期 | 测试人 |
|--------|----------|----------|----------|----------|--------|
| 座位状态检测 | 模拟人员坐下/离开 | 状态正确变化 | 待测试 | TBD | TBD |
| 数据上传 | 检查数据是否正确上传 | 服务器收到数据 | 待测试 | TBD | TBD |
| 预约功能 | 用户预约座位 | 预约成功 | 待测试 | TBD | TBD |
| 签到功能 | 用户扫码签到 | 签到成功 | 待测试 | TBD | TBD |

### 2.2 性能测试
| 测试项 | 测试方法 | 预期指标 | 实际结果 | 测试日期 | 测试人 |
|--------|----------|----------|----------|----------|--------|
| 并发用户数 | 模拟多用户同时操作 | ≥100 并发 | 待测试 | TBD | TBD |
| 响应时间 | 测量 API 响应时间 | <2 秒 | 待测试 | TBD | TBD |
| 数据准确性 | 长时间运行检测 | 准确率 >95% | 待测试 | TBD | TBD |

### 2.3 稳定性测试
| 测试项 | 测试方法 | 预期结果 | 实际结果 | 测试日期 | 测试人 |
|--------|----------|----------|----------|----------|--------|
| 长时间运行 | 连续运行 7 天 | 无崩溃、无内存泄漏 | 待测试 | TBD | TBD |
| 网络异常 | 模拟网络中断恢复 | 数据不丢失 | 待测试 | TBD | TBD |
| 断电恢复 | 模拟设备断电重启 | 自动恢复工作 | 待测试 | TBD | TBD |

## 3. Bug 记录

| Bug ID | 描述 | 严重程度 | 状态 | 修复日期 | 修复人 |
|--------|------|----------|------|----------|--------|
| BUG-001 | 待记录 | 待评估 | 待修复 | TBD | TBD |

## 4. 测试总结

### 4.1 测试覆盖率
- 功能测试覆盖率：待统计
- 代码覆盖率：待统计

### 4.2 遗留问题
- 待补充

### 4.3 改进建议
- 待补充
# v0.9 ESP32S3 Binary Link Test Record

| Test | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| ESP32S3 WiFi | Boot ESP32S3 gateway | Try configured WiFi profiles; print connected SSID/IP/RSSI or offline | Pending |
| STM32 PING | `FF D0 00 FF` | STM32 sends `PING`, ESP32 replies `PONG` | Pending |
| WiFi status | `FF D1 00 FF` after ESP32 sends status | STM32 prints WiFi state, SSID, IP, RSSI | Pending |
| Link status | `FF D2 00 FF` | STM32 prints ESP32 link and WiFi status | Pending |
| Mock card event | `FF D3 00 FF` | ESP32 parses `CARD_EVENT` and replies `ACK` | Pending |
| Real card event | Swipe a registered or denied card | Local access still works; ESP32 receives `CARD_EVENT` | Pending |
| Bad CRC | `FF D5 00 FF` | ESP32 replies `ERR` with CRC reason | Pending |
| USART2 isolation | Send debug commands while UART3 traffic exists | USART2 `FF CMD DATA FF` remains independent | Pending |

## v0.9.1 ESP32 Active Heartbeat Test Record

| Test | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| Bright LED | ESP32 boot | RGB LED is requested off, or PWR LED is confirmed not software controllable | Pending |
| GPIO48 conflict | ESP32 UART traffic on GPIO48 | Confirm whether board RGB LED flickers or interferes with UART RX | Pending |
| Active heartbeat | ESP32 powered | `[HB TX] seq=...` every 3 seconds | Pending |
| STM32 offline | ESP32 powered while STM32 disconnected | After 3 missed ACKs, ESP32 prints `[STM32] offline` | Pending |
| STM32 heartbeat ACK | STM32 connected and powered | STM32 replies `HEARTBEAT_ACK`; ESP32 prints `[HB ACK]` | Pending |
| PC13 pulse | ESP32 heartbeat received | STM32 PC13 pulses for about 50 ms | Pending |
| Recovery | Connect/reset STM32 after ESP32 offline | ESP32 prints `[STM32] recovered` | Pending |
| Initial sync | First heartbeat after link capture | ESP32 prints `[SYNC RX] cards=... logs=... time=...` | Pending |
| FF D2 status | `FF D2 00 FF` | STM32 prints ONLINE/OFFLINE, heartbeat count, WiFi and last RX type | Pending |
| EOF check | Send malformed frame tail | Parser rejects frame and records EOF/format error | Pending |
