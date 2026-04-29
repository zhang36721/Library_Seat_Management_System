<!--
  刷卡记录页面（需登录）
  - 查看、筛选刷卡记录
-->
<template>
  <div class="logs-view">
    <h2>📋 刷卡记录</h2>

    <el-row :gutter="16" style="margin: 20px 0;">
      <el-col :span="6">
        <el-input v-model="filters.cardId" placeholder="卡片ID" clearable />
      </el-col>
      <el-col :span="6">
        <el-input v-model="filters.seatId" placeholder="座位ID" clearable />
      </el-col>
      <el-col :span="6">
        <el-select v-model="filters.action" placeholder="动作" clearable style="width: 100%;">
          <el-option label="全部" value="" />
          <el-option label="签到" value="check_in" />
          <el-option label="签出" value="check_out" />
        </el-select>
      </el-col>
      <el-col :span="6">
        <el-input v-model="filters.userId" placeholder="用户编号" clearable />
      </el-col>
    </el-row>

    <el-button type="primary" @click="doSearch">搜索</el-button>

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
      <el-table-column prop="user_id" label="用户编号" width="120" />
      <el-table-column prop="user_name" label="用户名" width="120" />
      <el-table-column prop="timestamp" label="时间" width="180" />
    </el-table>

    <div v-if="logs.length === 0 && !loading" style="text-align: center; padding: 40px; color: #999;">
      暂无刷卡记录
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useLogStore } from '../stores'

const logStore = useLogStore()
const logs = computed(() => logStore.logs)
const loading = computed(() => logStore.loading)

const filters = reactive({
  cardId: '',
  seatId: '',
  action: '',
  userId: ''
})

function doSearch() {
  const params = {}
  if (filters.cardId) params.card_id = filters.cardId
  if (filters.seatId) params.seat_id = filters.seatId
  if (filters.action) params.action = filters.action
  if (filters.userId) params.user_id = filters.userId
  logStore.fetchLogs(params)
}

onMounted(() => {
  logStore.fetchLogs()
})
</script>