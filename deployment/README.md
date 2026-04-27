# 服务器部署

本目录包含服务器部署相关的配置文件和文档。

## 目录结构

| 目录/文件 | 说明 |
|-----------|------|
| `nginx/` | Nginx 配置文件 |
| `systemd/` | Systemd 服务配置文件 |
| `server_notes.md` | 服务器部署笔记 |

## 部署说明

### 1. 后端服务部署

```bash
# 安装依赖
pip install -r requirements.txt

# 配置环境变量
cp .env.example .env
# 编辑 .env 文件

# 使用 Gunicorn 启动
gunicorn main:app -w 4 -k uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000
```

### 2. 前端部署

```bash
# 构建
npm run build

# 将 dist 目录部署到 Nginx 或 CDN
```

### 3. Nginx 配置

参考 `nginx/` 目录下的配置文件。

### 4. Systemd 服务

参考 `systemd/` 目录下的服务配置文件。

## 注意事项

- 生产环境务必修改默认密钥
- 配置 HTTPS 证书
- 设置防火墙规则
- 定期备份数据库