'use client'

import { useState, useCallback } from 'react'
import { toast } from 'sonner'
import { DeviceCommand, CommandType } from '@/types/device'

export function useDeviceCommand(deviceId: string = 'esp32_relay_01') {
  const [isPending, setIsPending] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const sendCommand = useCallback(
    async (
      cmd: CommandType | string,
      payload: Partial<DeviceCommand> = {},
      options?: { silent?: boolean; successMsg?: string }
    ): Promise<boolean> => {
      setIsPending(true)
      setError(null)

      try {
        const body: DeviceCommand = {
          cmd,
          ...payload,
        }

        const res = await fetch(`/api/device/${deviceId}/command`, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify(body),
        })

        const data = await res.json()

        if (!res.ok) {
          const errMsg = data.error || `Command failed with status ${res.status}`
          setError(errMsg)
          if (!options?.silent) {
            toast.error(errMsg)
          }
          setIsPending(false)
          return false
        }

        if (options?.successMsg) {
          toast.success(options.successMsg)
        }

        setIsPending(false)
        return true
      } catch (err: any) {
        const errMsg = err.message || 'Network error while transmitting command'
        setError(errMsg)
        if (!options?.silent) {
          toast.error(errMsg)
        }
        setIsPending(false)
        return false
      }
    },
    [deviceId]
  )

  return {
    sendCommand,
    isPending,
    error,
  }
}
