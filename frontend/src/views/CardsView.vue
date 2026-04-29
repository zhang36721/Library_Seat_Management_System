<!--
  卡片管理页面（需登录）
  - 查看、搜索卡片
  - 管理员可注册/编辑/删除卡片
-->
<template>
  <div class="cards-view">
    <div class="page-header">
      <h2>💳 卡片管理</h2>
      <el-button v-if="auth.isAdmin" type="primary" @click="showCreateDialog = true">
        注册新卡片
      </el-button>
    </div>

    <el-row style="margin: 20px 0;">
      <el-input
        v-model="search"
        placeholder="搜索卡片ID或用户名"
        clearable
        style="width: 300px;"
        @input="debouncedSearch"
      />
    </el-row>

    <el-table v-loading="loading" :data="cards" border stripe>
      <el-table-column prop="card_id" label="卡片ID" width="140" />
      <el-table-column prop="user_name" label="用户名" />
      <el-table-column prop="user_id" label="用户编号" />
      <el-table-column prop="created_at" label="注册时间" width="180" />
      <el-table-column v-if="auth.isAdmin" label="操作" width="200">
        <template #default="{ row }">
          <el-button size="small" @click="openEdit(row)">编辑</el-button>
          <el-button size="small" type="danger" @click="handleDelete(row.card_id)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>

    <div v-if="cards.length === 0 && !loading" style="text-align: center; padding: 40px; color: #999;">
      暂无卡片数据
    </div>

    <!-- 新增/编辑弹窗 -->
    <el-dialog
      v-model="showCreateDialog"
      :title="editingCard ? '编辑卡片' : '注册新卡片'"
      width="500px"
      @close="resetForm"
    >
      <el-form ref="formRef" :model="form" :rules="rules" label-width="100px">
        <el-form-item label="卡片ID" prop="card_id">
          <el-input v-model="form.card_id" placeholder="如：A1B2C3D4" :disabled="!!editingCard" />
        </el-form-item>
        <el-form-item label="用户名" prop="user_name">
          <el-input v-model="form.user_name" placeholder="如：张三" />
        </el-form-item>
        <el-form-item label="用户编号" prop="user_id">
          <el-input v-model="form.user_id" placeholder="如：2024001" />
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
import { ref, reactive, computed, onMounted, watch } from 'vue'
import { useCardStore, useAuthStore } from '../stores'
import { ElMessage, ElMessageBox } from 'element-plus'

const cardStore = useCardStore()
const auth = useAuthStore()
const cards = computed(() => cardStore.cards)
const loading = computed(() => cardStore.loading)

const search = ref('')
let searchTimer = null

function debouncedSearch() {
  clearTimeout(searchTimer)
  searchTimer = setTimeout(() => {
    cardStore.fetchCards(search.value)
  }, 300)
}

const showCreateDialog = ref(false)
const saving = ref(false)
const editingCard = ref(null)
const formRef = ref(null)

const form = reactive({
  card_id: '',
  user_name: '',
  user_id: ''
})

const rules = {
  card_id: [{ required: true, message: '请输入卡片ID', trigger: 'blur' }],
  user_name: [{ required: true, message: '请输入用户名', trigger: 'blur' }],
  user_id: [{ required: true, message: '请输入用户编号', trigger: 'blur' }]
}

function openEdit(card) {
  editingCard.value = card
  form.card_id = card.card_id
  form.user_name = card.user_name
  form.user_id = card.user_id
  showCreateDialog.value = true
}

function resetForm() {
  editingCard.value = null
  form.card_id = ''
  form.user_name = ''
  form.user_id = ''
  formRef.value?.resetFields()
}

async function handleSave() {
  const valid = await formRef.value?.validate().catch(() => false)
  if (!valid) return

  saving.value = true
  try {
    if (editingCard.value) {
      await cardStore.updateCard(editingCard.value.card_id, {
        user_name: form.user_name,
        user_id: form.user_id
      })
      ElMessage.success('更新成功')
    } else {
      await cardStore.createCard({
        card_id: form.card_id,
        user_name: form.user_name,
        user_id: form.user_id
      })
      ElMessage.success('注册成功')
    }
    showCreateDialog.value = false
    resetForm()
  } catch (e) {
    ElMessage.error(e.message || '操作失败')
  } finally {
    saving.value = false
  }
}

async function handleDelete(cardId) {
  try {
    await ElMessageBox.confirm('确定删除该卡片吗？', '确认删除', { type: 'warning' })
    await cardStore.deleteCard(cardId)
    ElMessage.success('删除成功')
  } catch {
    // 取消
  }
}

onMounted(() => {
  cardStore.fetchCards()
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
