<!--
  设备状态页面（需登录）
  - 查看所有 ZigBee 设备状态
-->
<template>
  <div class="devices-view">
    <div class="page-header">
      <h2>🖥️ 设备状态</h2>
      <el-button @click="refresh">刷新</el-button>
    </div>

    <el-table v-loading="loading" :data="devices" border stripe style="margin-top: 20px;">
      <el-table-column prop="device_id" label="设备ID" width="200" />
      <el-table-column label="状态" width="120">
        <template #default="{ row }">
          <el-tag :type="deviceStore.getStatusColor(row.status)">
            {{ deviceStore.getStatusText(row.status) }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="last_heartbeat" label="最后心跳" width="200" />
      <el-table-column prop="battery" label="电池电量" width="120">
        <template #default="{ row }">
          {{ row.battery !== undefined ? row.battery + '%' : '-' }}
        </template>
      </el-table-column>
      <el-table-column prop="firmware_version" label="固件版本" />
    </el-table>

    <div v-if="devices.length === 0 && !loading" style="text-align: center; padding: 40px; color: #999;">
      暂无设备数据
    </div>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useDeviceStore } from '../stores'

const deviceStore = useDeviceStore()
const devices = computed(() => deviceStore.devices)
const loading = computed(() => deviceStore.loading)

function refresh() {
  deviceStore.fetchDevices()
}

onMounted(() => {
  deviceStore.fetchDevices()
})
</script>

<style scoped>
.page-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 10px;
}
.page-header h2 {
  margin: 0;
}
</style>