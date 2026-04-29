/**
 * 座位状态管理
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { seatApi } from '../api'

export const useSeatStore = defineStore('seat', () => {
  const seats = ref([])
  const loading = ref(false)
  const currentSeat = ref(null)

  async function fetchSeats() {
    loading.value = true
    try {
      const res = await seatApi.list()
      seats.value = res.data || []
    } finally {
      loading.value = false
    }
  }

  async function fetchSeat(seatId) {
    const res = await seatApi.get(seatId)
    currentSeat.value = res.data
    return res.data
  }

  async function createSeat(data) {
    const res = await seatApi.create(data)
    seats.value.push(res.data)
    return res.data
  }

  async function updateSeat(seatId, data) {
    const res = await seatApi.update(seatId, data)
    const idx = seats.value.findIndex(s => s.id === seatId)
    if (idx !== -1) seats.value[idx] = res.data
    return res.data
  }

  async function deleteSeat(seatId) {
    await seatApi.delete(seatId)
    seats.value = seats.value.filter(s => s.id !== seatId)
  }

  function getStatusText(status) {
    const map = { 0: '空闲', 1: '占用', 2: '疑似占用' }
    return map[status] || '未知'
  }

  function getStatusColor(status) {
    const map = { 0: 'success', 1: 'danger', 2: 'warning' }
    return map[status] || 'info'
  }

  return { seats, loading, currentSeat, fetchSeats, fetchSeat, createSeat, updateSeat, deleteSeat, getStatusText, getStatusColor }
})