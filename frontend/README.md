# Vue3 前端

基于 Vue 3 的图书馆座位管理系统前端界面。

## 技术栈

- **框架**: Vue 3 (Composition API)
- **构建工具**: Vite
- **UI 组件库**: Element Plus
- **状态管理**: Pinia
- **路由**: Vue Router 4
- **HTTP 客户端**: Axios
- **图表**: ECharts

## 目录结构

```
frontend/
├── public/           # 静态资源
├── src/
│   ├── api/          # API 请求封装
│   ├── assets/       # 资源文件（图片、样式等）
│   ├── components/   # 公共组件
│   ├── views/        # 页面组件
│   ├── router/       # 路由配置
│   └── stores/       # Pinia 状态管理
└── README.md         # 本文件
```

## 快速开始

### 1. 安装依赖

```bash
npm install
# 或
pnpm install
# 或
yarn install
```

### 2. 配置环境变量

复制 `.env.example` 为 `.env.local` 并修改配置：

```bash
cp .env.example .env.local
```

### 3. 启动开发服务器

```bash
npm run dev
# 或
pnpm dev
# 或
yarn dev
```

访问 http://localhost:5173

### 4. 构建生产版本

```bash
npm run build
# 或
pnpm build
# 或
yarn build
```

## 主要页面

- **登录页**: 用户登录
- **座位管理**: 管理员查看和管理座位
- **座位预约**: 用户预约座位
- **统计报表**: 数据统计和可视化
- **系统设置**: 系统参数配置

## 开发规范

- 使用 Composition API
- 组件采用 `<script setup>` 语法
- 使用 TypeScript（可选）
- 遵循 Vue 3 最佳实践
- 组件命名使用 PascalCase