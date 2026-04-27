# 公共组件

本目录用于存放可复用的 Vue 组件。

## 组件分类（待实现）

### 布局组件
- `Layout.vue` - 主布局
- `Header.vue` - 页头
- `Sidebar.vue` - 侧边栏
- `Footer.vue` - 页脚

### 业务组件
- `SeatMap.vue` - 座位地图
- `SeatItem.vue` - 座位项
- `ReservationForm.vue` - 预约表单
- `UserCard.vue` - 用户卡片

### 通用组件
- `Loading.vue` - 加载动画
- `Empty.vue` - 空状态
- `Error.vue` - 错误显示
- `ConfirmDialog.vue` - 确认对话框

## 组件开发规范

1. 组件名使用 PascalCase
2. 使用 `<script setup>` 语法
3. Props 使用 defineProps 定义
4. Events 使用 defineEmits 定义
5. 提供组件文档和使用示例