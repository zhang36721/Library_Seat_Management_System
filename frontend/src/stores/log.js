/**
 * 刷卡记录状态管理（简化版 - 无需登录即可查看）
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { logApi } from '../api'

export const useLogStore = defineStore('log', () => {
  const logs = ref([])
  const loading = ref(false)

  async function fetchLogs(limit = 100) {
    loading.value = true
    try {
      const res = await logApi.list({ limit })
      logs.value = res.data || []
    } finally {
      loading.value = false
    }
  }

  return { logs, loading, fetchLogs }
})