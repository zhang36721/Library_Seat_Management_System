# Nginx 配置

本目录存放 Nginx 服务器配置文件。

## 配置文件（待实现）

### library.conf
主配置文件，包含：
- 前端静态文件服务
- 后端 API 反向代理
- SSL 证书配置
- 安全头配置
- 缓存配置

### 使用方式

```bash
# 复制配置到 Nginx 目录
sudo cp library.conf /etc/nginx/sites-available/

# 创建软链接
sudo ln -s /etc/nginx/sites-available/library.conf /etc/nginx/sites-enabled/

# 测试配置
sudo nginx -t

# 重载 Nginx
sudo systemctl reload nginx
```

## 配置说明

- 前端文件服务：监听 80/443 端口，提供静态文件
- API 代理：将 `/api/` 请求代理到后端服务（localhost:8000）
- SSL：配置 HTTPS（可选）
- 安全：添加安全响应头