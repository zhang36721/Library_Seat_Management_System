# 硬件接线总览

本文件作为最终演示阶段的接线入口。详细接线表已按设备拆分到 `hardware/wiring/`，旧版硬件表仍保留用于追溯。

## 接线文档

| 设备 | 文档 | 说明 |
| --- | --- | --- |
| STM32 主控 | [main_controller_wiring.md](../hardware/wiring/main_controller_wiring.md) | RC522、OLED、DS1302、蜂鸣器、按键、步进电机、ZigBee、ESP32、debug |
| 座位端 STM32 | [seat_node_wiring.md](../hardware/wiring/seat_node_wiring.md) | HX711、红外、座位输入、座位红绿灯、ZigBee、debug |
| ESP32S3 网关 | [esp32s3_wiring.md](../hardware/wiring/esp32s3_wiring.md) | USART3、供电、GPIO47/48、WiFi、HTTP 上报地址 |

## 关键约束

1. STM32 USART2 只作为 debug 串口，协议为 `FF CMD DATA FF`。
2. ZigBee 使用 USART1，波特率 `38400 8N1`，帧格式固定为 `FA ADDR_L ADDR_H LEN DATA F5`。
3. ESP32S3 使用 STM32 USART3，波特率 `115200 8N1`，二进制协议帧以 `A5 5A` 开头。
4. ESP32S3 上报后端端口为 `18080`，前端开发端口为 `15173`。
5. ESP32S3 不能使用 `localhost` 或 `127.0.0.1` 作为后端地址，必须使用后端电脑局域网 IP 或正式域名。
6. 所有串口设备必须共地，RC522/ZigBee/ESP32 信号侧必须使用 3.3V 电平。

## 已实测项

| 模块 | 状态 | 备注 |
| --- | --- | --- |
| RC522 | 已通过 | 可读取 UID，无卡能提示 |
| OLED | 已通过 | 首页、菜单、刷卡结果可显示 |
| DS1302 | 已通过 | 可读写合法时间 |
| 蜂鸣器 | 已通过 | 成功/失败提示非阻塞 |
| 8 路按键 | 已通过 | K1~K8 短按，K1 长按 |
| ESP32S3 HTTP 上报 | 已通过主链路 | STM32 -> ESP32 -> 后端 -> Web |
| 座位端 HX711 | 已通过基础读取 | 校准参数可写入 Flash，默认参数已保底 |
| ZigBee 点对点 | 调试中 | 保留二进制帧，继续观察丢包统计 |
