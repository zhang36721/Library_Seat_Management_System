<template>
  <div class="records-view">
    <h2>📋 刷卡记录</h2>

    <el-table v-loading="loading" :data="logs" border stripe style="margin-top: 20px;">
      <el-table-column prop="card_id" label="卡片ID" width="140" />
      <el-table-column label="座位" width="100">
        <template #default="{ row }">座位 {{ row.seat_id }}</template>
      </el-table-column>
      <el-table-column label="动作" width="100">
        <template #default="{ row }">
          <el-tag :type="row.action === 'check_in' ? 'success' : 'warning'" size="small">
            {{ row.action === 'check_in' ? '签到' : '签出' }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="user_name" label="用户名" width="120" />
      <el-table-column prop="timestamp" label="时间" min-width="180" />
    </el-table>

    <div v-if="logs.length === 0 && !loading" style="text-align: center; padding: 40px; color: #999;">
      暂无刷卡记录
    </div>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useLogStore } from '../stores'

const logStore = useLogStore()
const logs = computed(() => logStore.logs)
const loading = computed(() => logStore.loading)

onMounted(() => {
  logStore.fetchLogs()
})
</script>