# 图书馆座位管理系统

基于 STM32 主控、座位端 STM32、CC2530 ZigBee、ESP32S3 网关、FastAPI 后端和 Vue Web 前端的图书馆座位管理演示系统。

当前版本重点是稳定演示：本地刷卡、OLED/蜂鸣器/闸门反馈、座位状态采集、ESP32 主动上报、Web 展示。暂不做登录、用户权限、预约业务和数据库持久化。

主控 STM32 当前采用拆分持久化策略：注册卡表写入 STM32 Flash，断电不丢；最近通行记录只保留在本次上电 RAM 中，最终刷卡记录以后端/云端数据为准。

## 系统架构

```text
座位端 STM32 -> CC2530 ZigBee -> STM32 主控 -> ESP32S3 -> FastAPI 后端 -> Vue Web
                         |                 |
                         |                 +-> OLED / 蜂鸣器 / 闸门 / RC522 / DS1302
                         +-> USART2 debug   +-> USART2 debug
```

| 层级 | 组成 | 当前职责 |
| --- | --- | --- |
| STM32 主控 | STM32F103C8T6 | 刷卡、本地卡表、通行记录、OLED、蜂鸣器、闸门、座位状态汇总 |
| 座位端 | STM32F103C8T6 | HX711/红外/电平检测、座位 LED、ZigBee 上报 |
| ZigBee | CC2530 两端 | USART1 38400 二进制点对点通信 |
| ESP32S3 | 网关 | USART3 二进制协议、WiFi、HTTP 主动上报 |
| 后端 | FastAPI | 内存保存最新设备状态和最近刷卡记录 |
| 前端 | Vue 3 + Vite | 中文演示控制台 |

## 端口说明

| 服务 | 端口 | 说明 |
| --- | --- | --- |
| 后端 FastAPI | 18080 | ESP32 主动上报和 Web 读取数据 |
| 前端 Vite dev | 15173 | 演示页面 |
| 前端 preview | 15174 | 构建后预览 |
| STM32 Debug | USART2 115200 | `FF CMD DATA FF` |
| ZigBee | USART1 38400 | `FA ADDR_L ADDR_H LEN DATA F5` |
| ESP32 链路 | USART3 115200 | `A5 5A ... CRC 0D` |

ESP32 的后端地址不能写 `localhost` 或 `127.0.0.1`，必须写后端电脑局域网 IP，例如：

```cpp
constexpr const char *SERVER_BASE_URL_301 = "http://192.168.141.236:18080";
```

## 启动方式

1. 启动后端：

   ```powershell
   python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080
   ```

2. 启动前端：

   ```powershell
   cd frontend
   npm run dev -- --host 0.0.0.0 --port 15173
   ```

3. STM32 主控上电。
4. 座位端 STM32 上电。
5. ESP32S3 上电。
6. 浏览器打开：

   ```text
   http://localhost:15173/
   ```

## 演示流程

1. Web 显示后端服务正常。
2. ESP32S3 每 2 秒上报 heartbeat，Web 显示 ESP32 网关在线。
3. STM32 主控上电后，Web 显示 STM32 主控在线。
4. Web 显示 3 个座位状态和闸门状态。
5. 刷已注册卡：OLED 显示通过，蜂鸣器快速响一声，闸门开关，Web 出现通过记录。
6. 刷未注册卡：OLED 显示拒绝，蜂鸣器三连响，闸门不动作，Web 出现拒绝记录。
7. 断开 ESP32S3：Web 约 10 秒后显示连接丢失。
8. 恢复 ESP32S3：Web 自动恢复在线。

## 关键调试命令

| 命令 | 功能 |
| --- | --- |
| `FF A0 00 FF` | 打印主控业务状态 |
| `FF A1 00 FF` | 手动执行一次刷卡流程测试 |
| `FF B0 00 FF` | 打印 8 路按键映射 |
| `FF B1 00 FF` | 打印 8 路按键原始电平 |
| `FF C0 00 FF` | 打印本次上电 RAM 通行记录 |
| `FF D2 00 FF` | 打印 ESP32 链路状态和队列统计 |
| `FF 84 00 FF` | 打印 ZigBee 二进制链路统计 |
| `FF 86 00 FF` | 发送 ZigBee 二进制 PING |
| `FF E0 00 FF` | 打印系统健康统计 |

## 文档入口

| 文档 | 说明 |
| --- | --- |
| [docs/01_project_overview.md](docs/01_project_overview.md) | 项目总览 |
| [docs/02_software_architecture.md](docs/02_software_architecture.md) | 软件架构 |
| [docs/03_protocol_and_data_flow.md](docs/03_protocol_and_data_flow.md) | 协议与数据流 |
| [docs/04_hardware_wiring.md](docs/04_hardware_wiring.md) | 硬件接线入口 |
| [docs/05_test_record.md](docs/05_test_record.md) | 测试记录 |
| [docs/06_demo_guide.md](docs/06_demo_guide.md) | 演示指南 |
| [docs/07_cloud_deploy.md](docs/07_cloud_deploy.md) | 云服务器部署指南 |
| [hardware/wiring/main_controller_wiring.md](hardware/wiring/main_controller_wiring.md) | STM32 主控接线图 |
| [hardware/wiring/seat_node_wiring.md](hardware/wiring/seat_node_wiring.md) | 座位端接线图 |
| [hardware/wiring/esp32s3_wiring.md](hardware/wiring/esp32s3_wiring.md) | ESP32S3 接线图 |

## 当前限制

云服务器部署时，本系统入口为 `https://www.kento.top/library-seat/`，避免占用现有项目的根路径和登录页。

1. STM32 主控注册卡表会写入 Flash；通行记录为 RAM only，断电后清空。
2. 后端状态和刷卡记录以内存保存为主，重启后会清空。
3. ZigBee 点对点链路仍需继续观察丢包统计和现场供电稳定性。
4. ESP32S3 PlatformIO 编译需要在安装 PlatformIO 的电脑上执行。
5. 目前不做登录、用户系统、预约系统和云端数据库。
