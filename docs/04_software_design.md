# 软件设计说明

## 1. 后端架构

### 1.1 技术栈
- **框架**: FastAPI
- **数据存储**: JSON 文件（轻量级，无需数据库）
- **认证**: JWT
- **文档**: Swagger / ReDoc

### 1.2 目录结构
```
backend/
├── app/
│   ├── api/          # API 路由
│   ├── models/       # 数据模型（JSON 文件操作）
│   ├── schemas/      # Pydantic 模型
│   ├── services/     # 业务逻辑
│   └── core/         # 核心配置
├── data_templates/   # JSON 数据模板
├── runtime_data/     # 运行时数据（不提交到 Git）
├── main.py           # 应用入口
├── requirements.txt  # 依赖
└── README.md         # 说明文档
```

### 1.3 主要功能模块
- 用户管理（管理员、学生）
- 座位状态管理
- 预约管理
- 签到管理
- 数据统计
- 设备管理

### 1.4 数据存储方案
- 当前版本使用 JSON 文件作为轻量数据表
- 无需安装和配置数据库，降低部署复杂度
- 适合当前 3 个座位的小规模场景
- 后期如果数据量变大，可迁移到 SQLite 或 MySQL

## 2. 前端架构

### 2.1 技术栈
- **框架**: Vue 3 (Composition API)
- **构建工具**: Vite
- **样式**: 原生 CSS / 自定义组件
- **状态管理**: Pinia
- **路由**: Vue Router
- **HTTP 客户端**: Axios
- **图表**: ECharts

### 2.2 目录结构
```
frontend/
├── public/           # 静态资源
├── src/
│   ├── api/          # API 请求
│   ├── assets/       # 资源文件
│   ├── components/   # 公共组件
│   ├── views/        # 页面组件
│   ├── router/       # 路由配置
│   └── stores/       # 状态管理
└── README.md         # 说明文档
```

### 2.3 主要页面
- 登录页
- 座位管理页（管理员）
- 座位预约页（用户）
- 统计报表页
- 系统设置页

## 3. 数据结构

### 3.1 座位数据
```json
{
  "id": 1,
  "name": "座位A",
  "status": 0,
  "node_id": 1,
  "description": "靠窗座位"
}
```

### 3.2 卡片数据
```json
{
  "card_id": "A1B2C3D4E5",
  "user_name": "张三",
  "user_id": "2024001",
  "status": "active",
  "created_at": "2024-01-01T00:00:00Z"
}
```

### 3.3 刷卡记录
```json
{
  "id": 1,
  "card_id": "A1B2C3D4E5",
  "user_name": "张三",
  "action": "check_in",
  "seat_id": 1,
  "timestamp": "2024-01-15T08:30:00Z",
  "result": "success"
}
```

## 4. API 设计规范

### 4.1 RESTful 风格
- 使用标准 HTTP 方法
- 资源命名使用复数名词
- 版本控制在 URL 中体现（如 `/api/v1/`）

### 4.2 统一响应格式
```json
{
  "code": 200,
  "message": "success",
  "data": {}
}
```

### 4.3 错误码规范
| 错误码 | 说明 |
|--------|------|
| 200 | 成功 |
| 400 | 请求参数错误 |
| 401 | 未授权 |
| 403 | 禁止访问 |
| 404 | 资源不存在 |
| 500 | 服务器内部错误 |