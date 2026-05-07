<template>
  <main class="console-page">
    <header class="topbar">
      <div>
        <h1>图书馆座位管理系统</h1>
        <p>ESP32 主动上报，Web 从后端读取设备状态</p>
      </div>
      <div class="api-config">
        <label for="baseUrl">后端地址</label>
        <div class="api-row">
          <input id="baseUrl" v-model="baseUrlInput" type="text" placeholder="默认 /library-seat，或填完整后端地址" />
          <button type="button" @click="saveBaseUrl">保存</button>
        </div>
      </div>
    </header>

    <section class="toolbar">
      <div class="connection" :class="{ error: loadError }">
        {{ loadError || `后端 API：${apiBaseText}` }}
      </div>
      <div class="actions">
        <span class="updated">最后更新：{{ lastRefreshText }}</span>
        <button type="button" class="primary" @click="refreshAll">刷新</button>
        <button type="button" @click="handleClearEvents">清空记录</button>
      </div>
    </section>

    <section class="status-grid">
      <article class="status-card">
        <span>后端服务</span>
        <strong :class="loadError ? 'bad' : 'ok'">
          {{ loadError ? '不可用' : '正常' }}
        </strong>
      </article>
      <article class="status-card">
        <span>ESP32 网关</span>
        <strong :class="espOnline ? 'ok' : status ? 'bad' : 'pending'">
          {{ status ? (espOnline ? '在线' : '连接丢失') : '等待数据' }}
        </strong>
        <small>最近心跳：{{ heartbeatAgeText }}</small>
      </article>
      <article class="status-card">
        <span>STM32 主控</span>
        <strong :class="status?.stm32_online ? 'ok' : status ? 'bad' : 'pending'">
          {{ status ? (status.stm32_online ? '在线' : '离线') : '等待数据' }}
        </strong>
      </article>
      <article class="status-card">
        <span>WiFi 状态</span>
        <strong :class="status?.wifi_connected ? 'ok' : status ? 'bad' : 'pending'">
          {{ status?.wifi_connected ? status.ssid : status ? '离线' : '等待数据' }}
        </strong>
        <small>信号强度 {{ status?.rssi ?? '-' }}</small>
      </article>
      <article class="status-card">
        <span>当前时间</span>
        <strong>{{ status?.ds1302_valid ? status.current_time : '无有效时间' }}</strong>
      </article>
      <article class="status-card">
        <span>后端时间</span>
        <strong>{{ serverTimeText }}</strong>
      </article>
      <article class="status-card">
        <span>注册卡数量</span>
        <strong>{{ status?.card_count ?? 0 }}</strong>
      </article>
      <article class="status-card">
        <span>通行记录数量</span>
        <strong>{{ status?.log_count ?? 0 }}</strong>
      </article>
      <article class="status-card">
        <span>闸门状态</span>
        <strong :class="gateClass">{{ gateText }}</strong>
      </article>
    </section>

    <section class="seats-section">
      <div class="section-title">
        <h2>座位状态</h2>
        <span>当前来自主控临时/模拟状态</span>
      </div>
      <div class="seat-grid">
        <article v-for="seat in seats" :key="seat.id" class="seat-card" :class="seatClass(seat.state)">
          <span>座位 {{ seat.id }}</span>
          <strong>{{ seatText(seat.state) }}</strong>
        </article>
      </div>
    </section>

    <section class="events-section">
      <div class="section-title">
        <h2>最近刷卡记录</h2>
        <span>后端内存缓存 {{ cardEvents.length }} 条</span>
      </div>
      <div class="table-wrap">
        <table>
          <thead>
            <tr>
              <th>时间</th>
              <th>卡号 UID</th>
              <th>类型</th>
              <th>结果</th>
            </tr>
          </thead>
          <tbody>
            <tr v-if="cardEvents.length === 0">
              <td colspan="4" class="empty">暂无刷卡记录</td>
            </tr>
            <tr v-for="event in newestEvents" :key="`${event.uid}-${event.received_ms}-${event.server_received_at}`">
              <td>{{ event.time || '-' }}</td>
              <td class="uid">{{ event.uid || '-' }}</td>
              <td>{{ accessTypeText(event.type) }}</td>
              <td>
                <span class="pill" :class="event.allowed ? 'allow' : 'deny'">
                  {{ event.allowed ? '通过' : '拒绝' }}
                </span>
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    </section>
  </main>
</template>

<script setup>
import { computed, onBeforeUnmount, onMounted, ref } from 'vue'
import { clearEvents, getCardEvents, getDefaultBaseUrl, getStatus } from '@/api/iotBackend'

const STORAGE_KEY = 'kento_backend_base_url'
const ESP_OFFLINE_TIMEOUT_MS = 10000

const baseUrl = ref(localStorage.getItem(STORAGE_KEY) || getDefaultBaseUrl())
const baseUrlInput = ref(baseUrl.value)
const status = ref(null)
const cardEvents = ref([])
const loadError = ref('')
const lastRefreshAt = ref(0)
let refreshTimer = null

const newestEvents = computed(() => [...cardEvents.value].reverse())

const apiBaseText = computed(() => baseUrl.value || '当前域名')

const seats = computed(() => {
  return status.value?.device_status?.seats || status.value?.seats || [
    { id: 1, state: 'UNKNOWN' },
    { id: 2, state: 'UNKNOWN' },
    { id: 3, state: 'UNKNOWN' }
  ]
})

const gateState = computed(() => status.value?.device_status?.gate?.state || status.value?.gate?.state || 'UNKNOWN')

const gateText = computed(() => {
  const map = {
    CLOSED: '已关闭',
    OPENING: '开门中',
    OPEN: '已开启',
    CLOSING: '关门中',
    ERROR: '异常'
  }
  return map[gateState.value] || '未知'
})

const gateClass = computed(() => {
  if (gateState.value === 'ERROR') {
    return 'bad'
  }
  if (gateState.value === 'UNKNOWN') {
    return 'pending'
  }
  return 'ok'
})

const espOnline = computed(() => {
  if (!status.value) {
    return false
  }
  if (typeof status.value.esp_online === 'boolean') {
    return status.value.esp_online
  }
  if (typeof status.value.esp32_online === 'boolean') {
    return status.value.esp32_online
  }
  const last = Number(status.value.last_heartbeat_at_ms || 0)
  return last > 0 && Date.now() - last < ESP_OFFLINE_TIMEOUT_MS
})

const heartbeatAgeText = computed(() => {
  const last = Number(status.value?.last_heartbeat_at_ms || 0)
  const now = Number(status.value?.server_now_ms || Date.now())
  if (!last) {
    return '暂无'
  }
  const seconds = Math.max(0, Math.round((now - last) / 1000))
  return seconds <= 1 ? '刚刚' : `${seconds} 秒前`
})

const lastRefreshText = computed(() => {
  if (!lastRefreshAt.value) {
    return '尚未刷新'
  }
  return new Date(lastRefreshAt.value).toLocaleTimeString()
})

const serverTimeText = computed(() => {
  if (!status.value?.server_time) {
    return '未知'
  }
  return new Date(status.value.server_time).toLocaleString()
})

function saveBaseUrl() {
  const nextUrl = baseUrlInput.value.trim().replace(/\/+$/, '')
  if (nextUrl) {
    baseUrl.value = nextUrl
    localStorage.setItem(STORAGE_KEY, nextUrl)
  } else {
    baseUrl.value = getDefaultBaseUrl()
    baseUrlInput.value = baseUrl.value
    localStorage.removeItem(STORAGE_KEY)
  }
  refreshAll()
}

function accessTypeText(type) {
  if (type === 'CHECK_IN') {
    return '入馆'
  }
  if (type === 'CHECK_OUT') {
    return '离馆'
  }
  return '拒绝'
}

function seatText(state) {
  if (state === 'FREE') {
    return '空闲'
  }
  if (state === 'OCCUPIED') {
    return '占用'
  }
  return '未知'
}

function seatClass(state) {
  if (state === 'FREE') {
    return 'free'
  }
  if (state === 'OCCUPIED') {
    return 'occupied'
  }
  return 'unknown'
}

async function refreshAll() {
  try {
    const [nextStatus, nextEvents] = await Promise.all([
      getStatus(baseUrl.value),
      getCardEvents(baseUrl.value)
    ])
    status.value = nextStatus
    cardEvents.value = nextEvents.events || []
    lastRefreshAt.value = Date.now()
    loadError.value = ''
  } catch (error) {
    loadError.value = `后端连接失败：${error.message}`
  }
}

async function handleClearEvents() {
  try {
    await clearEvents(baseUrl.value)
    await refreshAll()
  } catch (error) {
    loadError.value = `清空失败：${error.message}`
  }
}

onMounted(() => {
  refreshAll()
  refreshTimer = window.setInterval(refreshAll, 2000)
})

onBeforeUnmount(() => {
  if (refreshTimer) {
    window.clearInterval(refreshTimer)
  }
})
</script>

<style scoped>
.console-page {
  max-width: 1120px;
  margin: 0 auto;
  padding: 24px;
}

.topbar {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 20px;
  margin-bottom: 18px;
}

h1,
h2,
p {
  margin: 0;
}

h1 {
  font-size: 30px;
  line-height: 1.2;
  color: #101828;
}

.topbar p {
  margin-top: 8px;
  color: #667085;
}

.api-config {
  min-width: 320px;
}

.api-config label {
  display: block;
  margin-bottom: 6px;
  font-size: 13px;
  color: #667085;
}

.api-row {
  display: flex;
  gap: 8px;
}

input,
button {
  height: 38px;
  border-radius: 6px;
  font: inherit;
}

input {
  flex: 1;
  min-width: 0;
  border: 1px solid #c9d2dc;
  padding: 0 10px;
  background: #fff;
}

button {
  border: 1px solid #b9c3cf;
  padding: 0 14px;
  background: #fff;
  color: #17202a;
  cursor: pointer;
}

button.primary {
  border-color: #2563eb;
  background: #2563eb;
  color: #fff;
}

.toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  margin-bottom: 16px;
}

.connection,
.updated {
  color: #667085;
  font-size: 14px;
}

.connection.error {
  color: #b42318;
}

.actions {
  display: flex;
  align-items: center;
  gap: 8px;
}

.status-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(170px, 1fr));
  gap: 12px;
}

.status-card {
  min-height: 112px;
  border: 1px solid #d8e0e8;
  border-radius: 8px;
  padding: 14px;
  background: #fff;
}

.status-card span,
.status-card small {
  display: block;
  color: #667085;
  font-size: 13px;
}

.status-card strong {
  display: block;
  margin-top: 10px;
  overflow-wrap: anywhere;
  font-size: 22px;
  line-height: 1.2;
}

.ok {
  color: #127348;
}

.bad {
  color: #b42318;
}

.pending {
  color: #667085;
}

.seats-section,
.events-section {
  margin-top: 20px;
}

.section-title {
  display: flex;
  align-items: baseline;
  justify-content: space-between;
  gap: 12px;
  margin-bottom: 10px;
}

.section-title h2 {
  font-size: 20px;
}

.section-title span {
  color: #667085;
  font-size: 14px;
}

.table-wrap {
  overflow-x: auto;
  border: 1px solid #d8e0e8;
  border-radius: 8px;
  background: #fff;
}

.seat-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
  gap: 12px;
}

.seat-card {
  border: 1px solid #d8e0e8;
  border-radius: 8px;
  padding: 14px;
  background: #fff;
}

.seat-card span {
  display: block;
  color: #667085;
  font-size: 13px;
}

.seat-card strong {
  display: block;
  margin-top: 8px;
  font-size: 22px;
}

.seat-card.free strong {
  color: #127348;
}

.seat-card.occupied strong {
  color: #b54708;
}

.seat-card.unknown strong {
  color: #667085;
}

table {
  width: 100%;
  border-collapse: collapse;
}

th,
td {
  padding: 12px;
  border-bottom: 1px solid #edf1f5;
  text-align: left;
  font-size: 14px;
  white-space: nowrap;
}

th {
  background: #eef2f7;
  color: #344054;
}

tr:last-child td {
  border-bottom: 0;
}

.uid {
  font-family: "Cascadia Mono", Consolas, monospace;
}

.pill {
  display: inline-block;
  min-width: 72px;
  border-radius: 999px;
  padding: 4px 8px;
  text-align: center;
  font-size: 12px;
  font-weight: 700;
}

.pill.allow {
  background: #e7f6ee;
  color: #127348;
}

.pill.deny {
  background: #fdecec;
  color: #b42318;
}

.empty {
  color: #667085;
  text-align: center;
}

@media (max-width: 720px) {
  .console-page {
    padding: 16px;
  }

  .topbar,
  .toolbar,
  .section-title {
    display: block;
  }

  .api-config,
  .actions {
    margin-top: 12px;
  }

  .api-config {
    min-width: 0;
  }
}
</style>
