/**
 * 系统配置状态管理
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { systemApi } from '../api'

export const useSystemStore = defineStore('system', () => {
  const config = ref(null)
  const loading = ref(false)

  async function fetchConfig() {
    loading.value = true
    try {
      const res = await systemApi.getConfig()
      config.value = res.data
    } finally {
      loading.value = false
    }
  }

  async function updateConfig(data) {
    const res = await systemApi.updateConfig(data)
    config.value = res.data
    return res.data
  }

  return { config, loading, fetchConfig, updateConfig }
})