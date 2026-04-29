<!--
  系统配置页面（管理员）
  - 修改系统配置参数
-->
<template>
  <div class="system-view">
    <h2>⚙️ 系统配置</h2>

    <el-card v-loading="loading" style="margin-top: 20px; max-width: 700px;">
      <el-form
        v-if="form"
        ref="formRef"
        :model="form"
        label-width="140px"
      >
        <el-form-item label="座位超时时长(分钟)">
          <el-input-number v-model="form.seat_timeout_minutes" :min="1" :max="1440" />
        </el-form-item>

        <el-form-item label="最大连续占用(小时)">
          <el-input-number v-model="form.max_continuous_hours" :min="1" :max="24" />
        </el-form-item>

        <el-form-item label="设备离线阈值(秒)">
          <el-input-number v-model="form.device_offline_threshold_seconds" :min="10" :max="3600" />
        </el-form-item>

        <el-form-item label="系统名称">
          <el-input v-model="form.system_name" />
        </el-form-item>

        <el-form-item>
          <el-button type="primary" :loading="saving" @click="handleSave">
            保存配置
          </el-button>
          <el-button @click="resetForm">重置</el-button>
        </el-form-item>
      </el-form>

      <div v-else style="text-align: center; padding: 40px; color: #999;">
        加载中...
      </div>
    </el-card>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { useSystemStore } from '../stores'
import { ElMessage } from 'element-plus'

const systemStore = useSystemStore()
const loading = ref(false)
const saving = ref(false)
const form = ref(null)

async function loadConfig() {
  loading.value = true
  try {
    await systemStore.fetchConfig()
    if (systemStore.config) {
      form.value = { ...systemStore.config }
    }
  } finally {
    loading.value = false
  }
}

function resetForm() {
  if (systemStore.config) {
    form.value = { ...systemStore.config }
  }
}

async function handleSave() {
  saving.value = true
  try {
    await systemStore.updateConfig(form.value)
    ElMessage.success('配置已保存')
  } catch (e) {
    ElMessage.error(e.message || '保存失败')
  } finally {
    saving.value = false
  }
}

onMounted(() => {
  loadConfig()
})
</script>