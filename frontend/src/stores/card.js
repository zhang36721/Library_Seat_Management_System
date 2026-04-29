/**
 * 卡片状态管理
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'
import { cardApi } from '../api'

export const useCardStore = defineStore('card', () => {
  const cards = ref([])
  const loading = ref(false)

  async function fetchCards(search = '') {
    loading.value = true
    try {
      const res = await cardApi.list(search)
      cards.value = res.data || []
    } finally {
      loading.value = false
    }
  }

  async function createCard(data) {
    const res = await cardApi.create(data)
    cards.value.push(res.data)
    return res.data
  }

  async function updateCard(cardId, data) {
    const res = await cardApi.update(cardId, data)
    const idx = cards.value.findIndex(c => c.card_id === cardId)
    if (idx !== -1) cards.value[idx] = res.data
    return res.data
  }

  async function deleteCard(cardId) {
    await cardApi.delete(cardId)
    cards.value = cards.value.filter(c => c.card_id !== cardId)
  }

  return { cards, loading, fetchCards, createCard, updateCard, deleteCard }
})