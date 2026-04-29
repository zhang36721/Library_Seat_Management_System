/**
 * 设备状态管理
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { deviceApi } from '../api'

export const useDeviceStore = defineStore('device', () => {
  const devices = ref([])
  const loading = ref(false)

  async function fetchDevices() {
    loading.value = true
    try {
      const res = await deviceApi.list()
      devices.value = res.data || []
    } finally {
      loading.value = false
    }
  }

  function getStatusText(status) {
    const map = { 0: '离线', 1: '在线', 2: '异常' }
    return map[status] || '未知'
  }

  function getStatusColor(status) {
    const map = { 0: 'danger', 1: 'success', 2: 'warning' }
    return map[status] || 'info'
  }

  return { devices, loading, fetchDevices, getStatusText, getStatusColor }
})