/**
 * 座位 API
 */

import http from './http'

export const seatApi = {
  list() {
    return http.get('/seats').then(r => r.data)
  },
  get(id) {
    return http.get(`/seats/${id}`).then(r => r.data)
  },
  create(data) {
    return http.post('/seats', data).then(r => r.data)
  },
  update(id, data) {
    return http.put(`/seats/${id}`, data).then(r => r.data)
  },
  delete(id) {
    return http.delete(`/seats/${id}`).then(r => r.data)
  }
}