/**
 * 刷卡记录 API
 */

import http from './http'

export const logApi = {
  list(params = {}) {
    return http.get('/logs', { params }).then(r => r.data)
  }
}