# Python 后端服务

基于 FastAPI 的图书馆座位管理系统后端服务。

## 技术栈

- **框架**: FastAPI
- **ORM**: SQLAlchemy
- **数据库**: MySQL / PostgreSQL
- **认证**: JWT
- **验证**: Pydantic

## 目录结构

```
backend/
├── app/
│   ├── api/          # API 路由
│   ├── models/       # 数据模型
│   ├── schemas/      # Pydantic 模型
│   ├── services/     # 业务逻辑
│   └── core/         # 核心配置
├── database/         # 数据库迁移脚本
├── main.py           # 应用入口
├── requirements.txt  # 依赖
└── README.md         # 本文件
```

## 快速开始

### 1. 安装依赖

```bash
pip install -r requirements.txt
```

### 2. 配置环境变量

复制 `.env.example` 为 `.env` 并修改配置：

```bash
cp .env.example .env
```

### 3. 数据库初始化

```bash
# 运行数据库迁移
alembic upgrade head
```

### 4. 启动服务

```bash
# 开发模式
uvicorn main:app --reload

# 生产模式
gunicorn main:app -w 4 -k uvicorn.workers.UvicornWorker
```

## API 文档

启动服务后访问：
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

## 主要功能

- 用户管理（注册、登录、权限）
- 座位状态管理
- 预约管理
- 签到管理
- 数据统计
- 设备管理

## 开发规范

- 遵循 PEP 8 代码风格
- 使用类型注解
- 编写单元测试
- API 遵循 RESTful 规范