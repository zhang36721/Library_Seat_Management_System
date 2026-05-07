# 演示指南

## 启动顺序

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

4. ESP32S3 网关上电。

5. 浏览器打开：

   ```text
   http://localhost:15173/
   ```

## 演示流程

1. Web 显示后端服务正常。
2. ESP32S3 每 2 秒主动上报心跳，Web 显示 ESP32 网关在线。
3. STM32 主控上电后，ESP32 收到 DEVICE_STATUS，Web 显示 STM32 主控在线。
4. Web 显示 3 个座位状态和闸门状态。
5. 刷已注册卡：OLED 显示通过，蜂鸣器快速响一声，闸门开关，Web 出现通过记录。
6. 刷未注册卡：OLED 显示拒绝，蜂鸣器三连响，闸门不动作，Web 出现拒绝记录。
7. 断开 ESP32S3：Web 约 10 秒后显示 ESP32 连接丢失。
8. 恢复 ESP32S3：Web 自动恢复在线。

## 常见问题

| 问题 | 处理 |
| --- | --- |
| ESP32 访问不到后端 | `SERVER_BASE_URL` 不能写 `localhost` 或 `127.0.0.1`，必须写后端电脑局域网 IP，例如 `http://192.168.141.236:18080` |
| 后端本机能访问，ESP32 不能访问 | 后端必须使用 `--host 0.0.0.0` 启动，并检查 Windows 防火墙是否允许 TCP 18080 |
| 前端打不开 | 确认 Vite 端口为 15173，访问 `http://localhost:15173/` |
| ESP32 编译失败 | 需要安装 PlatformIO 或 Arduino ESP32 环境 |
| ZigBee 丢包 | 检查两端 USART1 和 CC2530 都为 38400、TX/RX 交叉、P2.0 拉低、PAN ID/Channel/角色一致、供电稳定 |
| 座位状态与 Web 不一致 | 先看座位端 debug 和主控 `FF 84` ZigBee 统计，再看 ESP32/后端状态 |

## 当前限制

1. 不做登录、用户权限、预约业务。
2. 后端状态和刷卡记录仍以内存保存为主。
3. 座位端 ZigBee 链路仍在稳定性调试阶段。
4. 云端演示时 ESP32 使用 `https://www.kento.top/library-seat`；局域网演示时可把 `CLOUD_USE_PROD_SERVER` 改为 `false`。
