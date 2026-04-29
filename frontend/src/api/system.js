/**
 * 系统配置 API
 */

import http from './http'

export const systemApi = {
  getConfig() {
    return http.get('/system/config').then(r => r.data)
  },
  updateConfig(data) {
    return http.put('/system/config', data).then(r => r.data)
  }
}