/**
 * Axios HTTP 客户端封装
 * - 自动携带 JWT Token
 * - 统一错误处理
 */

import axios from 'axios'

const http = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL || '/api/v1',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器：自动携带 Token
http.interceptors.request.use(
  config => {
    const token = localStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  error => Promise.reject(error)
)

// 响应拦截器：统一处理错误
http.interceptors.response.use(
  response => response,
  error => {
    if (error.response) {
      const { status, data } = error.response
      if (status === 401) {
        localStorage.removeItem('token')
        localStorage.removeItem('user')
        // 不在登录页才跳转
        if (!window.location.pathname.includes('/login')) {
          window.location.href = '/login'
        }
      }
      const message = data?.detail || data?.message || '请求失败'
      error.message = message
    } else {
      error.message = '网络连接失败'
    }
    return Promise.reject(error)
  }
)

export default http