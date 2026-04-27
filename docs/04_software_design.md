# 软件设计说明

## 1. 后端架构

### 1.1 技术栈
- **框架**: FastAPI
- **ORM**: SQLAlchemy
- **数据库**: MySQL / PostgreSQL
- **认证**: JWT
- **文档**: Swagger / ReDoc

### 1.2 目录结构
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
└── README.md         # 说明文档
```

### 1.3 主要功能模块
- 用户管理（管理员、学生）
- 座位状态管理
- 预约管理
- 签到管理
- 数据统计
- 设备管理

## 2. 前端架构

### 2.1 技术栈
- **框架**: Vue 3 (Composition API)
- **构建工具**: Vite
- **UI 组件库**: Element Plus
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

## 3. 数据库设计

### 3.1 主要表结构

#### 用户表 (users)
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INT | 主键 |
| username | VARCHAR(50) | 用户名 |
| password | VARCHAR(255) | 密码哈希 |
| role | ENUM | 角色(admin/student) |
| created_at | DATETIME | 创建时间 |

#### 座位表 (seats)
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INT | 主键 |
| seat_code | VARCHAR(20) | 座位编号 |
| location | VARCHAR(100) | 位置描述 |
| status | INT | 当前状态 |
| node_id | INT | 关联节点ID |

#### 预约记录表 (reservations)
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INT | 主键 |
| user_id | INT | 用户ID |
| seat_id | INT | 座位ID |
| start_time | DATETIME | 开始时间 |
| end_time | DATETIME | 结束时间 |
| status | INT | 预约状态 |

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