<!--
  座位管理页面
  - 公开页面，展示所有座位状态
  - 管理员可新增/编辑/删除座位
-->
<template>
  <div class="seats-view">
    <div class="page-header">
      <h2>🪑 座位管理</h2>
      <el-button v-if="auth.isAdmin" type="primary" @click="showCreateDialog = true">
        新增座位
      </el-button>
    </div>

    <!-- 统计栏 -->
    <el-row :gutter="20" style="margin: 20px 0;">
      <el-col :span="8">
        <el-card shadow="hover" class="stat-card">
          <span>总数：{{ seats.length }}</span>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card shadow="hover" class="stat-card stat-free">
          <span>空闲：{{ freeCount }}</span>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card shadow="hover" class="stat-card stat-occupied">
          <span>占用：{{ occupiedCount }}</span>
        </el-card>
      </el-col>
    </el-row>

    <!-- 座位网格（公开使用卡片展示） -->
    <el-row :gutter="16">
      <el-col v-for="seat in seats" :key="seat.id" :span="6" style="margin-bottom: 16px;">
        <el-card shadow="hover" class="seat-card">
          <div class="seat-info">
            <h3>{{ seat.name || '座位 ' + seat.id }}</h3>
            <el-tag :type="seatStore.getStatusColor(seat.status)">
              {{ seatStore.getStatusText(seat.status) }}
            </el-tag>
          </div>
          <p>节点ID: {{ seat.node_id }}</p>
          <p v-if="seat.description" class="desc">{{ seat.description }}</p>

          <div v-if="auth.isAdmin" class="seat-actions">
            <el-button size="small" @click="openEdit(seat)">编辑</el-button>
            <el-button size="small" type="danger" @click="handleDelete(seat.id)">删除</el-button>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <div v-if="seats.length === 0 && !seatStore.loading" style="text-align: center; padding: 40px; color: #999;">
      暂无座位数据
    </div>

    <!-- 新增/编辑弹窗 -->
    <el-dialog
      v-model="showCreateDialog"
      :title="editingSeat ? '编辑座位' : '新增座位'"
      width="500px"
      @close="resetForm"
    >
      <el-form ref="formRef" :model="form" :rules="rules" label-width="100px">
        <el-form-item label="座位名称" prop="name">
          <el-input v-model="form.name" placeholder="如：A-01" />
        </el-form-item>
        <el-form-item label="节点ID" prop="node_id">
          <el-input-number v-model="form.node_id" :min="1" placeholder="ZigBee 节点ID" style="width: 100%;" />
        </el-form-item>
        <el-form-item label="描述" prop="description">
          <el-input v-model="form.description" type="textarea" :rows="3" placeholder="可选" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="showCreateDialog = false">取消</el-button>
        <el-button type="primary" :loading="saving" @click="handleSave">保存</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useSeatStore, useAuthStore } from '../stores'
import { ElMessage, ElMessageBox } from 'element-plus'

const seatStore = useSeatStore()
const auth = useAuthStore()
const seats = computed(() => seatStore.seats)

const freeCount = computed(() => seats.value.filter(s => s.status === 0).length)
const occupiedCount = computed(() => seats.value.filter(s => s.status === 1).length)

const showCreateDialog = ref(false)
const saving = ref(false)
const editingSeat = ref(null)
const formRef = ref(null)

const form = reactive({
  name: '',
  node_id: 1,
  description: ''
})

const rules = {
  name: [{ required: true, message: '请输入座位名称', trigger: 'blur' }],
  node_id: [{ required: true, message: '请输入节点ID', trigger: 'blur' }]
}

function openEdit(seat) {
  editingSeat.value = seat
  form.name = seat.name
  form.node_id = seat.node_id
  form.description = seat.description || ''
  showCreateDialog.value = true
}

function resetForm() {
  editingSeat.value = null
  form.name = ''
  form.node_id = 1
  form.description = ''
  formRef.value?.resetFields()
}

async function handleSave() {
  const valid = await formRef.value?.validate().catch(() => false)
  if (!valid) return

  saving.value = true
  try {
    if (editingSeat.value) {
      await seatStore.updateSeat(editingSeat.value.id, { name: form.name, node_id: form.node_id, description: form.description })
      ElMessage.success('更新成功')
    } else {
      await seatStore.createSeat({ name: form.name, node_id: form.node_id, description: form.description })
      ElMessage.success('创建成功')
    }
    showCreateDialog.value = false
    resetForm()
  } catch (e) {
    ElMessage.error(e.message || '操作失败')
  } finally {
    saving.value = false
  }
}

async function handleDelete(seatId) {
  try {
    await ElMessageBox.confirm('确定删除该座位吗？', '确认删除', { type: 'warning' })
    await seatStore.deleteSeat(seatId)
    ElMessage.success('删除成功')
  } catch {
    // 取消
  }
}

onMounted(() => {
  seatStore.fetchSeats()
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
.stat-card span {
  font-size: 16px;
  font-weight: bold;
}
.stat-free { border-left: 4px solid var(--el-color-success); }
.stat-occupied { border-left: 4px solid var(--el-color-danger); }
.seat-card h3 { margin: 0 0 8px 0; }
.seat-info {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 8px;
}
.desc { color: #999; font-size: 12px; }
.seat-actions {
  margin-top: 12px;
  display: flex;
  gap: 8px;
}
</style>