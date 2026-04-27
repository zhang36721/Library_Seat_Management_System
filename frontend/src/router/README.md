# 路由配置

本目录用于配置 Vue Router 路由。

## 文件结构（待实现）

```
router/
├── index.js          # 路由主入口
├── routes.js         # 路由配置
└── guards.js         # 路由守卫
```

## 路由配置

### 公开路由
- `/login` - 登录
- `/register` - 注册

### 用户路由（需要登录）
- `/` - 首页
- `/reservation` - 预约
- `/my-reservations` - 我的预约

### 管理员路由（需要管理员权限）
- `/admin` - 管理员首页
- `/admin/seats` - 座位管理
- `/admin/users` - 用户管理
- `/admin/devices` - 设备管理
- `/admin/statistics` - 数据统计

## 路由守卫

- 登录验证
- 权限检查
- 页面标题设置