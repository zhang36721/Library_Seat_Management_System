/**
 * Pinia 全局状态管理 - 统一入口
 */

import { createPinia } from 'pinia'

const pinia = createPinia()

export default pinia
export { useAuthStore } from './auth'
export { useSeatStore } from './seat'
export { useCardStore } from './card'
export { useLogStore } from './log'
