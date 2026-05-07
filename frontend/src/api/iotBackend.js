const DEFAULT_BASE_URL = import.meta.env.VITE_API_BASE_URL ?? ''

function normalizeBaseUrl(baseUrl) {
  return (baseUrl || DEFAULT_BASE_URL).trim().replace(/\/+$/, '')
}

async function requestJson(baseUrl, path, options = {}) {
  const response = await fetch(`${normalizeBaseUrl(baseUrl)}${path}`, {
    cache: 'no-store',
    ...options
  })

  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`)
  }

  return response.json()
}

export function getDefaultBaseUrl() {
  return DEFAULT_BASE_URL
}

export async function getStatus(baseUrl) {
  return requestJson(baseUrl, '/api/iot/status')
}

export async function getCardEvents(baseUrl) {
  return requestJson(baseUrl, '/api/iot/card-events')
}

export async function clearEvents(baseUrl) {
  return requestJson(baseUrl, '/api/iot/clear-events', { method: 'POST' })
}
