'use client'

import { useEffect, useState, useRef, useCallback } from 'react'
import { DeviceStatus } from '@/types/device'

interface UseDeviceStatusOptions {
  pollingFallbackIntervalMs?: number
  defaultDeviceId?: string
}

export function useDeviceStatus(
  deviceId: string = 'esp32_relay_01',
  options: UseDeviceStatusOptions = {}
) {
  const { pollingFallbackIntervalMs = 3000 } = options

  const [status, setStatus] = useState<DeviceStatus | null>(null)
  const [online, setOnline] = useState<boolean>(false)
  const [error, setError] = useState<string | null>(null)
  const [lastUpdate, setLastUpdate] = useState<number | null>(null)
  const [isConnected, setIsConnected] = useState<boolean>(false)

  const eventSourceRef = useRef<EventSource | null>(null)
  const retryTimeoutRef = useRef<NodeJS.Timeout | null>(null)
  const pollIntervalRef = useRef<NodeJS.Timeout | null>(null)

  // Fetch status via REST (used initially and as fallback)
  const fetchStatusFallback = useCallback(async () => {
    if (!deviceId) return
    try {
      const res = await fetch(`/api/device/${deviceId}/status`)
      if (res.ok) {
        const data: DeviceStatus = await res.json()
        setStatus(data)
        setOnline(Boolean(data.online))
        setLastUpdate(Date.now())
        setError(null)
      }
    } catch (err: any) {
      setError(err.message || 'Failed to poll status')
    }
  }, [deviceId])

  useEffect(() => {
    if (!deviceId) {
      setStatus(null)
      setOnline(false)
      return
    }
    let mounted = true

    // Initial fetch
    fetchStatusFallback()

    const setupSSE = () => {
      if (eventSourceRef.current) {
        eventSourceRef.current.close()
      }

      const es = new EventSource(`/api/device/${deviceId}/stream`)
      eventSourceRef.current = es

      es.onopen = () => {
        if (!mounted) return
        setIsConnected(true)
        setError(null)
        if (pollIntervalRef.current) {
          clearInterval(pollIntervalRef.current)
          pollIntervalRef.current = null
        }
      }

      const handleIncomingStatus = (raw: string) => {
        if (!mounted) return
        try {
          const parsed: DeviceStatus = JSON.parse(raw)
          if (parsed && typeof parsed === 'object') {
            setStatus(parsed)
            setOnline(Boolean(parsed.online))
            setLastUpdate(Date.now())
            setError(null)
          }
        } catch (err) {
          console.warn('[SSE Hook] Failed to parse status event:', err)
        }
      }

      es.onmessage = (e: MessageEvent) => {
        handleIncomingStatus(e.data)
      }

      es.addEventListener('status', (e: MessageEvent) => {
        handleIncomingStatus(e.data)
      })

      es.addEventListener('connectivity', (e: MessageEvent) => {
        if (!mounted) return
        try {
          const data = JSON.parse(e.data)
          setOnline(Boolean(data.online))
          setLastUpdate(Date.now())
        } catch (err) {
          console.warn('[SSE Hook] Parse error on connectivity event:', err)
        }
      })

      es.addEventListener('ping', () => {
        if (!mounted) return
        setLastUpdate(Date.now())
      })

      es.onerror = () => {
        if (!mounted) return
        setIsConnected(false)
        es.close()

        // Start fallback polling if not already running
        if (!pollIntervalRef.current) {
          pollIntervalRef.current = setInterval(fetchStatusFallback, pollingFallbackIntervalMs)
        }

        // Schedule auto-reconnect for SSE
        if (retryTimeoutRef.current) clearTimeout(retryTimeoutRef.current)
        retryTimeoutRef.current = setTimeout(() => {
          if (mounted) setupSSE()
        }, 5000)
      }
    }

    setupSSE()

    return () => {
      mounted = false
      if (eventSourceRef.current) {
        eventSourceRef.current.close()
      }
      if (retryTimeoutRef.current) {
        clearTimeout(retryTimeoutRef.current)
      }
      if (pollIntervalRef.current) {
        clearInterval(pollIntervalRef.current)
      }
    }
  }, [deviceId, fetchStatusFallback, pollingFallbackIntervalMs])

  return {
    status,
    online,
    error,
    lastUpdate,
    isConnected,
    refresh: fetchStatusFallback,
  }
}
