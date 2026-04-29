import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  {
    path: '/',
    name: 'Home',
    component: () => import('@/views/HomeView.vue')
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
    path: '/records',
    name: 'Records',
    component: () => import('@/views/RecordsView.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/:pathMatch(.*)*',
    name: 'NotFound',
    component: () => import('@/views/HomeView.vue')
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// 路由守卫：检查认证
router.beforeEach((to, from, next) => {
  const token = localStorage.getItem('token')

  // 需要认证的页面
  if (to.meta.requiresAuth && !token) {
    return next('/login')
  }

  // 已登录用户访问登录页，重定向到首页
  if (to.path === '/login' && token) {
    return next('/')
  }

  next()
})

export default router