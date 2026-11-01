'use client'

import { useState, useEffect, useCallback } from 'react'
import { useDeviceCommand } from './use-device-command'
import { CommandType } from '@/types/device'

export function useRelayGrid(
  deviceId: string = 'esp32_relay_01',
  incomingRelays?: boolean[]
) {
  const [relays, setRelays] = useState<boolean[]>(new Array(16).fill(false))
  const [selectedChannels, setSelectedChannels] = useState<Set<number>>(new Set())
  const { sendCommand, isPending } = useDeviceCommand(deviceId)

  useEffect(() => {
    if (incomingRelays && incomingRelays.length === 16) {
      setRelays(incomingRelays)
    }
  }, [incomingRelays])

  const toggleRelay = useCallback(
    async (channelIndex: number) => {
      const targetState = !relays[channelIndex]
      // Optimistic update
      setRelays((prev) => {
        const next = [...prev]
        next[channelIndex] = targetState
        return next
      })

      const success = await sendCommand(
        CommandType.RELAY,
        { ch: channelIndex, v: targetState ? 1 : 0 },
        { silent: true }
      )

      if (!success) {
        // Revert on failure
        setRelays((prev) => {
          const next = [...prev]
          next[channelIndex] = !targetState
          return next
        })
      }
    },
    [relays, sendCommand]
  )

  const masterAllOn = useCallback(async () => {
    setRelays(new Array(16).fill(true))
    await sendCommand(CommandType.ACTION, { a: 'all_on' }, { successMsg: 'All 16 Relays Energized' })
  }, [sendCommand])

  const masterAllOff = useCallback(async () => {
    setRelays(new Array(16).fill(false))
    await sendCommand(CommandType.ACTION, { a: 'all_off' }, { successMsg: 'All 16 Relays De-energized' })
  }, [sendCommand])

  const setOdd = useCallback(async () => {
    setRelays((prev) => prev.map((_, idx) => idx % 2 === 0))
    await sendCommand(CommandType.ACTION, { a: 'odd' })
  }, [sendCommand])

  const setEven = useCallback(async () => {
    setRelays((prev) => prev.map((_, idx) => idx % 2 === 1))
    await sendCommand(CommandType.ACTION, { a: 'even' })
  }, [sendCommand])

  const invertAll = useCallback(async () => {
    setRelays((prev) => prev.map((val) => !val))
    await sendCommand(CommandType.ACTION, { a: 'invert' })
  }, [sendCommand])

  // Multi-selection tools
  const toggleSelectChannel = useCallback((channelIndex: number) => {
    setSelectedChannels((prev) => {
      const next = new Set(prev)
      if (next.has(channelIndex)) {
        next.delete(channelIndex)
      } else {
        next.add(channelIndex)
      }
      return next
    })
  }, [])

  const selectAll = useCallback(() => {
    setSelectedChannels(new Set(Array.from({ length: 16 }, (_, i) => i)))
  }, [])

  const deselectAll = useCallback(() => {
    setSelectedChannels(new Set())
  }, [])

  const turnSelected = useCallback(
    async (turnOn: boolean) => {
      if (selectedChannels.size === 0) return

      const updated = [...relays]
      selectedChannels.forEach((ch) => {
        updated[ch] = turnOn
      })
      setRelays(updated)

      // Send bitmask or sequential commands
      for (const ch of Array.from(selectedChannels)) {
        await sendCommand(CommandType.RELAY, { ch, v: turnOn ? 1 : 0 }, { silent: true })
      }
    },
    [selectedChannels, relays, sendCommand]
  )

  return {
    relays,
    selectedChannels,
    isPending,
    toggleRelay,
    masterAllOn,
    masterAllOff,
    setOdd,
    setEven,
    invertAll,
    toggleSelectChannel,
    selectAll,
    deselectAll,
    turnSelected,
  }
}
