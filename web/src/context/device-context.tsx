'use client'

import * as React from 'react'

export interface RegisteredDevice {
  deviceId: string
  deviceName: string
  online: boolean
  lastSeenAt: string | null
  firmware: string
  ip: string
  mode: string
  effect?: string
  rssi?: number
}

interface DeviceContextValue {
  activeDeviceId: string
  setActiveDeviceId: (id: string) => void
  devices: RegisteredDevice[]
  activeDevice: RegisteredDevice | null
  isLoadingDevices: boolean
  refreshDevices: () => Promise<void>
  deleteDevice: (deviceId: string) => Promise<boolean>
}

const DeviceContext = React.createContext<DeviceContextValue | undefined>(undefined)

const STORAGE_KEY = 'esp32_active_device_id'

export function DeviceProvider({ children }: { children: React.ReactNode }) {
  const [activeDeviceId, setActiveDeviceIdState] = React.useState<string>(() => {
    if (typeof window !== 'undefined') {
      const saved = localStorage.getItem(STORAGE_KEY) || ''
      if (saved === 'esp32_relay_01') return ''
      return saved
    }
    return ''
  })
  const [devices, setDevices] = React.useState<RegisteredDevice[]>([])
  const [isLoadingDevices, setIsLoadingDevices] = React.useState<boolean>(true)

  const setActiveDeviceId = React.useCallback((id: string) => {
    setActiveDeviceIdState(id)
    if (typeof window !== 'undefined') {
      localStorage.setItem(STORAGE_KEY, id)
    }
  }, [])

  const fetchDevices = React.useCallback(async () => {
    setIsLoadingDevices(true)
    try {
      const res = await fetch('/api/device/list')
      if (res.ok) {
        const data: RegisteredDevice[] = await res.json()
        setDevices(data)

        setActiveDeviceIdState((currentId) => {
          if (currentId === 'esp32_relay_01') {
            currentId = ''
          }

          const currentDev = data.find((d) => d.deviceId === currentId)
          // If current device is valid and online, keep it
          if (currentId && currentDev && currentDev.online) {
            return currentId
          }

          // If an online real hardware device exists, prioritize it immediately
          const onlineDev = data.find((d) => d.online && d.deviceId !== 'esp32_relay_01')
          if (onlineDev) {
            if (typeof window !== 'undefined') {
              localStorage.setItem(STORAGE_KEY, onlineDev.deviceId)
            }
            return onlineDev.deviceId
          }

          // Otherwise keep current if it still exists
          if (currentId && currentDev) {
            return currentId
          }

          // Fallback to first non-dummy device
          const fallback = data.find((d) => d.deviceId !== 'esp32_relay_01') || data[0]
          const chosen = fallback?.deviceId || ''

          if (chosen && typeof window !== 'undefined') {
            localStorage.setItem(STORAGE_KEY, chosen)
          }
          return chosen
        })
      }
    } catch (err) {
      console.warn('[DeviceContext] Failed to load devices list:', err)
    } finally {
      setIsLoadingDevices(false)
    }
  }, [])

  // Poll device list every 10 seconds to detect new/removed devices
  React.useEffect(() => {
    fetchDevices()
    const interval = setInterval(fetchDevices, 10000)
    return () => clearInterval(interval)
  }, [fetchDevices])

  const deleteDevice = React.useCallback(
    async (deviceIdToDelete: string): Promise<boolean> => {
      try {
        const res = await fetch(`/api/device/${deviceIdToDelete}`, {
          method: 'DELETE',
        })
        if (res.ok) {
          setDevices((prev) => prev.filter((d) => d.deviceId !== deviceIdToDelete))
          if (activeDeviceId === deviceIdToDelete) {
            const remaining = devices.filter((d) => d.deviceId !== deviceIdToDelete)
            const nextId = remaining.find((d) => d.online)?.deviceId || remaining[0]?.deviceId || ''
            setActiveDeviceId(nextId)
          }
          return true
        }
        return false
      } catch (err) {
        console.error('[DeviceContext] Error deleting device:', err)
        return false
      }
    },
    [activeDeviceId, devices, setActiveDeviceId]
  )

  const activeDevice = React.useMemo(() => {
    return devices.find((d) => d.deviceId === activeDeviceId) || null
  }, [devices, activeDeviceId])

  const value = React.useMemo<DeviceContextValue>(
    () => ({
      activeDeviceId,
      setActiveDeviceId,
      devices,
      activeDevice,
      isLoadingDevices,
      refreshDevices: fetchDevices,
      deleteDevice,
    }),
    [activeDeviceId, setActiveDeviceId, devices, activeDevice, isLoadingDevices, fetchDevices, deleteDevice]
  )

  return <DeviceContext.Provider value={value}>{children}</DeviceContext.Provider>
}

export function useActiveDevice(): DeviceContextValue {
  const context = React.useContext(DeviceContext)
  if (!context) {
    throw new Error('useActiveDevice must be used within a DeviceProvider')
  }
  return context
}

