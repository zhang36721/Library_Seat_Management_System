# Python 后端服务

基于 FastAPI 的图书馆座位管理系统后端服务。

## 技术栈

- **框架**: FastAPI
- **数据存储**: JSON 文件（轻量级，无需数据库）
- **验证**: Pydantic

## 目录结构

```
backend/
├── app/
│   ├── api/          # API 路由
│   ├── models/       # 数据模型（JSON 文件操作）
│   ├── schemas/      # Pydantic 模型
│   ├── services/     # 业务逻辑
│   └── core/         # 核心配置
├── data_templates/   # JSON 数据模板（可提交到 Git）
├── runtime_data/     # 运行时数据（不提交到 Git）
├── main.py           # 应用入口
├── requirements.txt  # 依赖
└── README.md         # 本文件
```

## 数据存储方案

### JSON 文件存储
- 当前版本使用 JSON 文件作为轻量数据表
- 无需安装和配置数据库，降低部署复杂度
- 适合当前 3 个座位的小规模场景

### 数据模板（data_templates/）
- 可提交到 GitHub 的示例 JSON 模板
- 包含数据结构定义和示例数据
- 用于初始化新环境

### 运行时数据（runtime_data/）
- 本地或云端真实运行数据
- **不提交到 GitHub**
- 通过环境变量 `DATA_DIR` 指定实际数据目录
- 云端建议路径：`/var/lib/library_seat/data/`

## 主要功能

- 接收 ESP32S3 上传的座位状态和刷卡事件
- 读取 JSON 数据文件
- 更新 seat 状态
- 追加 card_logs 刷卡记录
- 提供 Web 前端查询接口
- 提供设备状态查询接口

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

配置 `DATA_DIR` 指定 JSON 数据目录。

### 3. 初始化数据

首次运行时，后端会自动从 `data_templates/` 复制模板文件到此目录。

### 4. 启动服务

```bash
# 开发模式
uvicorn main:app --reload

# 生产模式
uvicorn main:app --host 0.0.0.0 --port 8000
```

## API 文档

启动服务后访问：
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

## 开发规范

- 遵循 PEP 8 代码风格
- 使用类型注解
- 编写单元测试
- API 遵循 RESTful 规范

## 后期扩展

如果数据量变大，可迁移到 SQLite 或 MySQL 数据库。