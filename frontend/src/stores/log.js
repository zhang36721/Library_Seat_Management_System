/**
 * 刷卡记录状态管理
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { logApi } from '../api'

export const useLogStore = defineStore('log', () => {
  const logs = ref([])
  const total = ref(0)
  const loading = ref(false)
  const page = ref(1)
  const pageSize = ref(20)

  async function fetchLogs(params = {}) {
    loading.value = true
    try {
      const res = await logApi.list({
        page: page.value,
        page_size: pageSize.value,
        ...params
      })
      logs.value = res.data.items || []
      total.value = res.data.total || 0
    } finally {
      loading.value = false
    }
  }

  async function changePage(p, filters = {}) {
    page.value = p
    await fetchLogs(filters)
  }

  return { logs, total, loading, page, pageSize, fetchLogs, changePage }
})