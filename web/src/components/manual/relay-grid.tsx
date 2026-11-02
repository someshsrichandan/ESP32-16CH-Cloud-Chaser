'use client'

import * as React from 'react'
import { RelayCard } from './relay-card'
import { Button } from '@/components/ui/button'
import { useRelayGrid } from '@/hooks/use-relay-grid'
import {
  Power,
  PowerOff,
  CheckSquare,
  Square,
  Binary,
  RotateCcw,
  Sliders,
  CheckCircle2,
} from 'lucide-react'

interface RelayGridProps {
  deviceId: string
  incomingRelays?: boolean[]
  disabled?: boolean
}

export function RelayGrid({
  deviceId,
  incomingRelays,
  disabled = false,
}: RelayGridProps) {
  const {
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
  } = useRelayGrid(deviceId, incomingRelays)

  const activeCount = relays.filter(Boolean).length
  const selectedCount = selectedChannels.size

  return (
    <div className="space-y-6">
      {/* Top Action Toolbar */}
      <div className="flex flex-wrap items-center justify-between gap-3 rounded-xl border border-zinc-800 bg-[#12121a] p-4 shadow-xl">
        <div className="flex flex-wrap items-center gap-2">
          {/* Master Controls */}
          <Button
            variant="cyanGlow"
            size="sm"
            onClick={masterAllOn}
            disabled={disabled || isPending}
            className="h-8 font-semibold"
          >
            <Power className="mr-1.5 h-3.5 w-3.5" />
            Master ALL ON
          </Button>

          <Button
            variant="outline"
            size="sm"
            onClick={masterAllOff}
            disabled={disabled || isPending}
            className="h-8 text-zinc-300 hover:text-red-400 hover:border-red-500/40"
          >
            <PowerOff className="mr-1.5 h-3.5 w-3.5" />
            Master ALL OFF
          </Button>

          {/* Patterns */}
          <div className="hidden sm:flex items-center space-x-1 pl-2 border-l border-zinc-800">
            <Button
              variant="secondary"
              size="sm"
              onClick={setOdd}
              disabled={disabled || isPending}
              className="h-8 text-xs font-mono"
            >
              <Binary className="mr-1 h-3 w-3" />
              Odd CH
            </Button>
            <Button
              variant="secondary"
              size="sm"
              onClick={setEven}
              disabled={disabled || isPending}
              className="h-8 text-xs font-mono"
            >
              <Binary className="mr-1 h-3 w-3" />
              Even CH
            </Button>
            <Button
              variant="secondary"
              size="sm"
              onClick={invertAll}
              disabled={disabled || isPending}
              className="h-8 text-xs font-mono"
            >
              <RotateCcw className="mr-1 h-3 w-3" />
              Invert
            </Button>
          </div>
        </div>

        {/* Selection Batch Controls */}
        <div className="flex items-center space-x-2">
          {selectedCount > 0 ? (
            <div className="flex items-center space-x-2 bg-cyan-950/40 border border-cyan-500/30 rounded-lg px-2.5 py-1">
              <span className="text-xs font-mono text-cyan-300 font-semibold">
                {selectedCount} Selected
              </span>
              <Button
                variant="cyanGlow"
                size="sm"
                onClick={() => turnSelected(true)}
                disabled={disabled || isPending}
                className="h-6 text-[10px] px-2"
              >
                ON
              </Button>
              <Button
                variant="outline"
                size="sm"
                onClick={() => turnSelected(false)}
                disabled={disabled || isPending}
                className="h-6 text-[10px] px-2 text-zinc-300"
              >
                OFF
              </Button>
              <Button
                variant="ghost"
                size="sm"
                onClick={deselectAll}
                className="h-6 text-[10px] px-1.5 text-zinc-400"
              >
                Clear
              </Button>
            </div>
          ) : (
            <div className="flex items-center space-x-1.5">
              <Button
                variant="ghost"
                size="sm"
                onClick={selectAll}
                className="h-8 text-xs text-zinc-400 hover:text-zinc-200"
              >
                <CheckSquare className="mr-1.5 h-3.5 w-3.5" />
                Select All
              </Button>
            </div>
          )}

          <div className="hidden lg:flex items-center space-x-1.5 pl-3 border-l border-zinc-800 text-xs font-mono text-zinc-400">
            <span className="font-bold text-cyan-400">{activeCount}</span>
            <span>/ 16 Energized</span>
          </div>
        </div>
      </div>

      {/* 4x4 Grid Matrix */}
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4">
        {relays.map((active, index) => (
          <RelayCard
            key={index}
            channelIndex={index}
            name={`Output Stage ${index + 1}`}
            active={active}
            isSelected={selectedChannels.has(index)}
            onToggle={toggleRelay}
            onToggleSelect={toggleSelectChannel}
            disabled={disabled || isPending}
          />
        ))}
      </div>
    </div>
  )
}
