# 云服务器部署指南

当前 `https://www.kento.top/` 已经有项目在运行，所以本系统不要占用根路径、`/login` 或通用 `/api`。

本系统云端入口固定为：

```text
Web 页面 : https://www.kento.top/library-seat/
HTTP API : https://www.kento.top/library-seat/api/iot/*
健康检查 : https://www.kento.top/library-seat/health
```

整体链路：

```text
STM32 主控 -> ESP32S3 -> https://www.kento.top/library-seat/api/iot/*
Web 浏览器 -> https://www.kento.top/library-seat/
```

Nginx 只需要把 `/library-seat/` 这一段挂到现有 `www.kento.top` 的 server block 中：

```text
Nginx 80/443 existing server
  ├─ /                         -> 你原来的项目
  ├─ /login                    -> 你原来的项目
  ├─ /api/*                    -> 你原来的项目
  └─ /library-seat/*
       ├─ static frontend dist
       └─ /library-seat/api/*  -> FastAPI 127.0.0.1:18080/api/*
```

## 1. DNS

如果 `www.kento.top` 已经可访问，不需要新增解析。否则在域名服务商处添加：

| 类型 | 主机记录 | 值 |
| --- | --- | --- |
| A | `www` | 云服务器公网 IP |
| A | `@` | 云服务器公网 IP |

## 2. 安装环境

Ubuntu 示例：

```bash
sudo apt update
sudo apt install -y python3 python3-venv python3-pip nginx git nodejs npm
```

如果系统源里的 Node 版本太旧，建议安装 Node.js 20 LTS。

## 3. 上传代码

示例路径：

```bash
sudo mkdir -p /opt/library-seat
sudo chown -R $USER:$USER /opt/library-seat
cd /opt/library-seat
git clone <your-repo-url> .
```

也可以直接用 `scp/rsync` 上传当前工程目录。

## 4. 后端配置

```bash
cd /opt/library-seat
python3 -m venv .venv
source .venv/bin/activate
pip install -r backend/requirements.txt
cp backend/.env.example backend/.env
```

编辑 `backend/.env`：

```env
HOST=127.0.0.1
PORT=18080
DEBUG=false
CORS_ORIGINS=https://www.kento.top
CORS_ALLOW_CREDENTIALS=false
SECRET_KEY=请改成随机字符串
```

手动测试：

```bash
python -m uvicorn backend.main:app --host 127.0.0.1 --port 18080
curl http://127.0.0.1:18080/health
```

## 5. 前端构建

生产环境必须使用子路径：

```env
VITE_PUBLIC_BASE_PATH=/library-seat/
VITE_API_BASE_URL=/library-seat
```

构建：

```bash
cd /opt/library-seat/frontend
cp .env.production.example .env.production
npm install
npm run build
```

构建产物：

```text
frontend/dist
```

## 6. Nginx 接入现有站点

不要覆盖你现有项目的 Nginx 配置。把下面示例文件里的 `location` 片段合并到当前 `www.kento.top` 的 `server { ... }` 中：

```bash
cat /opt/library-seat/deploy/nginx/kento.top.conf.example
```

合并后检查：

```bash
sudo nginx -t
sudo systemctl reload nginx
```

访问：

```text
https://www.kento.top/library-seat/
https://www.kento.top/library-seat/health
https://www.kento.top/library-seat/api/iot/status
```

## 7. HTTPS

如果你现有项目已经有 HTTPS 证书，本系统直接复用同一个 `www.kento.top` 证书即可。

如果还没有 HTTPS：

```bash
sudo apt install -y certbot python3-certbot-nginx
sudo certbot --nginx -d www.kento.top -d kento.top
```

## 8. systemd 后台运行

```bash
sudo cp /opt/library-seat/deploy/systemd/kento-backend.service.example /etc/systemd/system/kento-backend.service
sudo systemctl daemon-reload
sudo systemctl enable --now kento-backend
sudo systemctl status kento-backend
```

查看日志：

```bash
journalctl -u kento-backend -f
```

## 9. ESP32S3 固件配置

当前已改成子路径正式地址：

```cpp
constexpr const char *SERVER_BASE_URL_PROD = "https://www.kento.top/library-seat";
constexpr bool CLOUD_USE_PROD_SERVER = true;
```

ESP32 最终 POST 地址会变成：

```text
https://www.kento.top/library-seat/api/iot/heartbeat
https://www.kento.top/library-seat/api/iot/device-status
https://www.kento.top/library-seat/api/iot/card-event
```

重新编译并烧录 ESP32S3。串口应看到：

```text
[CLOUD] server=https://www.kento.top/library-seat device=kento-main-001
```

## 10. 验收

1. 打开 `https://www.kento.top/library-seat/`，Web 页面正常加载。
2. `https://www.kento.top/library-seat/api/iot/status` 返回 JSON。
3. ESP32S3 上电后，Web 显示 ESP32 在线。
4. STM32 主控上电后，Web 显示 STM32 在线。
5. 刷卡后，Web 最近刷卡记录出现新记录。
6. 断开 ESP32S3 后，Web 约 10 秒内显示连接丢失。

## 常见问题

| 问题 | 处理 |
| --- | --- |
| 打开根路径进了旧项目 | 正常，本系统入口是 `/library-seat/` |
| 前端能打开但没数据 | 确认 `VITE_API_BASE_URL=/library-seat`，并访问 `/library-seat/api/iot/status` |
| ESP32 POST 失败 | 确认串口打印的 server 是 `https://www.kento.top/library-seat` |
| API 404 | 检查 Nginx 是否有 `/library-seat/api/` 反代到 `http://127.0.0.1:18080/api/` |
| HTTPS 证书问题 | 先确认 `https://www.kento.top/library-seat/health` 可访问 |
| ESP32 HTTPS 握手失败 | 当前固件使用 `setInsecure()` 跑通优先；后续可换 CA 证书校验 |
