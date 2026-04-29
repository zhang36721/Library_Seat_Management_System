/**
 * 设备 API
 */

import http from './http'

export const deviceApi = {
  list() {
    return http.get('/devices').then(r => r.data)
  }
}