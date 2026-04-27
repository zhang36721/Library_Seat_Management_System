# 服务器部署笔记

## 服务器要求

### 最低配置
- CPU: 2 核心
- 内存: 2GB
- 磁盘: 20GB
- 操作系统: Ubuntu 20.04 LTS / CentOS 7+

### 推荐配置
- CPU: 4 核心
- 内存: 4GB
- 磁盘: 40GB SSD
- 操作系统: Ubuntu 22.04 LTS

## 软件依赖

### 必需软件
- Python 3.8+
- Node.js 16+
- Nginx
- MySQL 8.0 / PostgreSQL 13+
- Redis (可选，用于缓存)

### 安装命令（Ubuntu）

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装基础软件
sudo apt install -y python3-pip python3-venv nodejs npm nginx mysql-server

# 安装 Python 依赖管理工具
sudo pip3 install virtualenv
```

## 部署步骤

### 1. 项目克隆

```bash
cd /var/www
git clone <repository-url> library-seat-system
cd library-seat-system
```

### 2. 后端部署

```bash
cd backend
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# 配置环境变量
cp .env.example .env
# 编辑 .env 文件，设置数据库连接等

# 数据库迁移
alembic upgrade head

# 启动服务（使用 systemd）
sudo systemctl enable library-backend
sudo systemctl start library-backend
```

### 3. 前端部署

```bash
cd frontend
npm install
npm run build

# 将 dist 目录复制到 Nginx 目录
sudo cp -r dist/* /var/www/html/
```

### 4. Nginx 配置

```bash
sudo cp deployment/nginx/library.conf /etc/nginx/sites-available/
sudo ln -s /etc/nginx/sites-available/library.conf /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### 5. SSL 证书配置（可选）

```bash
# 使用 Let's Encrypt
sudo apt install certbot python3-certbot-nginx
sudo certbot --nginx -d your-domain.com
```

## 监控和维护

### 日志查看

```bash
# 后端日志
sudo journalctl -u library-backend -f

# Nginx 日志
sudo tail -f /var/log/nginx/access.log
sudo tail -f /var/log/nginx/error.log
```

### 备份策略

```bash
# 数据库备份（每天凌晨2点）
0 2 * * * mysqldump -u root -p library_seats > /backup/library_seats_$(date +\%Y\%m\%d).sql
```

## 常见问题

### 问题1：端口被占用
```bash
# 查看端口占用
sudo lsof -i :8000
# 杀死占用端口的进程
sudo kill -9 <PID>
```

### 问题2：数据库连接失败
- 检查 MySQL 服务状态
- 检查数据库用户权限
- 检查 .env 文件配置

### 问题3：Nginx 502 错误
- 检查后端服务是否正常运行
- 检查 Nginx 配置中的 upstream 地址