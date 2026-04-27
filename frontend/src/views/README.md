# 页面组件

本目录用于存放页面级别的 Vue 组件。

## 页面列表（待实现）

### 认证页面
- `Login.vue` - 登录页
- `Register.vue` - 注册页

### 用户页面
- `Dashboard.vue` - 用户首页
- `Reservation.vue` - 座位预约页
- `MyReservations.vue` - 我的预约

### 管理员页面
- `AdminDashboard.vue` - 管理员首页
- `SeatManagement.vue` - 座位管理
- `UserManagement.vue` - 用户管理
- `DeviceManagement.vue` - 设备管理
- `Statistics.vue` - 数据统计

### 系统页面
- `NotFound.vue` - 404 页面
- `Settings.vue` - 系统设置

## 页面开发规范

1. 每个页面对应一个路由
2. 页面组件负责组合业务组件
3. 业务逻辑尽量下沉到 components 或 stores
4. 使用路由守卫进行权限控制