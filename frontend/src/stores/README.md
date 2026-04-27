# 状态管理

本目录用于存放 Pinia stores，管理应用全局状态。

## Store 列表（待实现）

### 用户相关
- `user.js` - 用户信息和认证状态

### 业务相关
- `seat.js` - 座位状态
- `reservation.js` - 预约信息

### 应用相关
- `app.js` - 应用全局状态（主题、语言等）

## 文件结构示例

```javascript
// stores/user.js
import { defineStore } from 'pinia'

export const useUserStore = defineStore('user', {
  state: () => ({
    token: '',
    userInfo: null,
    permissions: []
  }),
  
  getters: {
    isLoggedIn: (state) => !!state.token,
    isAdmin: (state) => state.permissions.includes('admin')
  },
  
  actions: {
    async login(credentials) {
      // 登录逻辑
    },
    
    logout() {
      // 登出逻辑
    }
  }
})
```

## 使用方式

```javascript
import { useUserStore } from '@/stores/user'

const userStore = useUserStore()
userStore.login({ username, password })