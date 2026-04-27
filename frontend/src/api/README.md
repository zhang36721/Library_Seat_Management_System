# API 请求封装

本目录用于封装所有与后端 API 的交互。

## 文件结构（待实现）

```
api/
├── request.js      # Axios 实例配置
├── auth.js         # 认证相关 API
├── user.js         # 用户相关 API
├── seat.js         # 座位相关 API
├── reservation.js  # 预约相关 API
└── statistics.js   # 统计相关 API
```

## 使用示例

```javascript
import { getUserList } from '@/api/user'

// 在组件中使用
const { data } = await getUserList()