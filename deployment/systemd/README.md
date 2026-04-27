# Systemd 服务配置

本目录存放 Systemd 服务单元文件，用于管理后端服务的自启动和守护。

## 配置文件（待实现）

### library-backend.service
后端服务配置，包含：
- 服务描述
- 启动依赖（网络、数据库）
- 启动命令
- 重启策略
- 资源限制

### 使用方式

```bash
# 复制服务文件到 systemd 目录
sudo cp library-backend.service /etc/systemd/system/

# 重载 systemd 配置
sudo systemctl daemon-reload

# 启用服务（开机自启）
sudo systemctl enable library-backend

# 启动服务
sudo systemctl start library-backend

# 查看服务状态
sudo systemctl status library-backend

# 查看服务日志
sudo journalctl -u library-backend -f
```

## 配置说明

- 服务名：library-backend
- 运行用户：www-data 或专用用户
- 工作目录：/var/www/library-seat-system/backend
- 启动命令：gunicorn main:app -w 4 -k uvicorn.workers.UvicornWorker --bind 127.0.0.1:8000
- 重启策略：失败后自动重启，延迟 5 秒