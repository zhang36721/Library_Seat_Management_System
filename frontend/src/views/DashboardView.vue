<!--
  首页 Dashboard
  - 统计概览
  - 最近刷卡记录
-->
<template>
  <div class="dashboard">
    <h2>📊 系统概览</h2>

    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :span="6">
        <el-card shadow="hover">
          <el-statistic title="座位总数" :value="stats.totalSeats" />
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <el-statistic title="空闲座位" :value="stats.freeSeats">
            <template #suffix>
              <el-tag type="success" size="small">空闲</el-tag>
            </template>
          </el-statistic>
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <el-statistic title="占用座位" :value="stats.occupiedSeats">
            <template #suffix>
              <el-tag type="danger" size="small">占用</el-tag>
            </template>
          </el-statistic>
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <el-statistic title="注册卡片" :value="stats.totalCards" />
        </el-card>
      </el-col>
    </el-row>

    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :span="12">
        <el-card>
          <template #header><b>在线设备</b></template>
          <div v-if="deviceStore.devices.length === 0">暂无设备数据</div>
          <el-table v-else :data="deviceStore.devices" size="small">
            <el-table-column prop="device_id" label="设备ID" />
            <el-table-column label="状态">
              <template #default="{ row }">
                <el-tag :type="deviceStore.getStatusColor(row.status)" size="small">
                  {{ deviceStore.getStatusText(row.status) }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="last_heartbeat" label="最后心跳" />
          </el-table>
        </el-card>
      </el-col>

      <el-col :span="12">
        <el-card>
          <template #header><b>最近刷卡记录</b></template>
          <div v-if="!authStore.isLoggedIn" style="color: #999; padding: 20px;">
            请登录后查看
          </div>
          <div v-else-if="logStore.logs.length === 0">暂无记录</div>
          <el-table v-else :data="logStore.logs.slice(0, 10)" size="small">
            <el-table-column prop="card_id" label="卡片ID" width="120" />
            <el-table-column label="座位">
              <template #default="{ row }">
                座位 {{ row.seat_id }}
              </template>
            </el-table-column>
            <el-table-column label="动作" width="100">
              <template #default="{ row }">
                <el-tag :type="row.action === 'check_in' ? 'success' : 'warning'" size="small">
                  {{ row.action === 'check_in' ? '签到' : '签出' }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="timestamp" label="时间" width="180" />
          </el-table>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useSeatStore, useCardStore, useLogStore, useDeviceStore, useAuthStore } from '../stores'

const seatStore = useSeatStore()
const cardStore = useCardStore()
const logStore = useLogStore()
const deviceStore = useDeviceStore()
const authStore = useAuthStore()

const stats = computed(() => {
  const seats = seatStore.seats || []
  return {
    totalSeats: seats.length,
    freeSeats: seats.filter(s => s.status === 0).length,
    occupiedSeats: seats.filter(s => s.status === 1).length,
    totalCards: (cardStore.cards || []).length
  }
})

onMounted(async () => {
  await Promise.all([
    seatStore.fetchSeats(),
    cardStore.fetchCards(),
    logStore.fetchLogs(),
    deviceStore.fetchDevices()
  ])
})
</script>

<style scoped>
.dashboard h2 {
  margin: 0 0 10px 0;
}
</style>