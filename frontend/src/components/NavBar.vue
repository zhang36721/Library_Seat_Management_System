<!--
  导航栏组件
  - 根据登录状态和角色显示不同菜单
-->
<template>
  <el-menu
    mode="horizontal"
    :ellipsis="false"
    class="navbar"
    router
  >
    <el-menu-item index="/">
      <el-icon><HomeFilled /></el-icon>
      <span>首页</span>
    </el-menu-item>

    <el-menu-item index="/seats">
      <el-icon><Grid /></el-icon>
      <span>座位管理</span>
    </el-menu-item>

    <template v-if="auth.isLoggedIn">
      <el-menu-item index="/cards">
        <el-icon><Tickets /></el-icon>
        <span>卡片管理</span>
      </el-menu-item>

      <el-menu-item index="/logs">
        <el-icon><Document /></el-icon>
        <span>刷卡记录</span>
      </el-menu-item>

      <el-menu-item index="/devices">
        <el-icon><Monitor /></el-icon>
        <span>设备状态</span>
      </el-menu-item>

      <el-menu-item v-if="auth.isAdmin" index="/system">
        <el-icon><Setting /></el-icon>
        <span>系统配置</span>
      </el-menu-item>
    </template>

    <div class="flex-spacer" />

    <template v-if="auth.isLoggedIn">
      <el-sub-menu index="user">
        <template #title>
          <el-icon><User /></el-icon>
          <span>{{ auth.user?.username || '用户' }}</span>
        </template>
        <el-menu-item @click="handleLogout">
          <el-icon><SwitchButton /></el-icon>
          <span>退出登录</span>
        </el-menu-item>
      </el-sub-menu>
    </template>

    <template v-else>
      <el-menu-item index="/login">
        <el-icon><Key /></el-icon>
        <span>登录</span>
      </el-menu-item>
    </template>
  </el-menu>
</template>

<script setup>
import { useRouter } from 'vue-router'
import { useAuthStore } from '../stores'
import { ElMessage } from 'element-plus'

const auth = useAuthStore()
const router = useRouter()

function handleLogout() {
  auth.logout()
  ElMessage.success('已退出登录')
  router.push('/login')
}
</script>

<style scoped>
.navbar {
  padding: 0 20px;
  border-bottom: 1px solid var(--el-border-color-light);
}
.flex-spacer {
  flex: 1;
}
</style>