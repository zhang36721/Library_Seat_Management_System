<template>
  <div class="home">
    <h2>📊 系统概览</h2>

    <!-- 统计卡片 -->
    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :span="8">
        <el-card shadow="hover">
          <el-statistic title="座位总数" :value="stats.totalSeats" />
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card shadow="hover">
          <el-statistic title="空闲座位" :value="stats.freeSeats">
            <template #suffix>
              <el-tag type="success" size="small">空闲</el-tag>
            </template>
          </el-statistic>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card shadow="hover">
          <el-statistic title="注册卡片" :value="stats.totalCards" />
        </el-card>
      </el-col>
    </el-row>

    <!-- 最近记录 -->
    <el-card style="margin-top: 20px;">
      <template #header>
        <b>📋 最近刷卡记录</b>
      </template>
      <div v-if="logs.length === 0" style="color: #999; padding: 20px; text-align: center;">
        暂无刷卡记录
      </div>
      <el-table v-else :data="logs.slice(0, 10)" size="small">
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
    </el-card>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useSeatStore, useCardStore, useLogStore } from '../stores'

const seatStore = useSeatStore()
const cardStore = useCardStore()
const logStore = useLogStore()

const logs = computed(() => logStore.logs || [])

const stats = computed(() => {
  const seats = seatStore.seats || []
  return {
    totalSeats: seats.length,
    freeSeats: seats.filter(s => s.status === 0).length,
    totalCards: (cardStore.cards || []).length
  }
})

onMounted(async () => {
  await Promise.all([
    seatStore.fetchSeats(),
    cardStore.fetchCards(),
    logStore.fetchLogs()
  ])
})
</script>

<style scoped>
.home h2 {
  margin: 0 0 10px 0;
}
</style>