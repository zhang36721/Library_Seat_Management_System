/**
 * 卡片 API
 */

import http from './http'

export const cardApi = {
  list(search = '') {
    return http.get('/cards', { params: { search } }).then(r => r.data)
  },
  get(id) {
    return http.get(`/cards/${id}`).then(r => r.data)
  },
  create(data) {
    return http.post('/cards', data).then(r => r.data)
  },
  update(id, data) {
    return http.put(`/cards/${id}`, data).then(r => r.data)
  },
  delete(id) {
    return http.delete(`/cards/${id}`).then(r => r.data)
  }
}