/**
 * 认证 API
 */

import http from './http'

export const authApi = {
  login(username, password) {
    return http.post('/auth/login', { username, password }).then(r => r.data)
  },
  getMe() {
    return http.get('/auth/me').then(r => r.data)
  },
  listUsers() {
    return http.get('/auth/users').then(r => r.data)
  },
  createUser(data) {
    return http.post('/auth/users', data).then(r => r.data)
  }
}