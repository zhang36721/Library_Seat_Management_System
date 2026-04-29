import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  {
    path: '/',
    name: 'Dashboard',
    component: () => import('@/views/DashboardView.vue')
  },
  {
    path: '/login',
    name: 'Login',
    component: () => import('@/views/LoginView.vue')
  },
  {
    path: '/seats',
    name: 'Seats',
    component: () => import('@/views/SeatsView.vue')
  },
  {
    path: '/cards',
    name: 'Cards',
    component: () => import('@/views/CardsView.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/logs',
    name: 'Logs',
    component: () => import('@/views/LogsView.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/devices',
    name: 'Devices',
    component: () => import('@/views/DevicesView.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/system',
    name: 'SystemConfig',
    component: () => import('@/views/SystemView.vue'),
    meta: { requiresAuth: true, requiresAdmin: true }
  },
  {
    path: '/:pathMatch(.*)*',
    name: 'NotFound',
    component: () => import('@/views/DashboardView.vue')
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// 路由守卫：检查认证和权限
router.beforeEach((to, from, next) => {
  const token = localStorage.getItem('token')
  const userStr = localStorage.getItem('user')

  let user = null
  try {
    user = userStr ? JSON.parse(userStr) : null
  } catch {
    user = null
  }

  // 需要认证的页面
  if (to.meta.requiresAuth && !token) {
    return next('/login')
  }

  // 需要管理员权限的页面
  if (to.meta.requiresAdmin && user?.role !== 'admin') {
    return next('/')
  }

  // 已登录用户访问登录页，重定向到首页
  if (to.path === '/login' && token) {
    return next('/')
  }

  next()
})

export default router