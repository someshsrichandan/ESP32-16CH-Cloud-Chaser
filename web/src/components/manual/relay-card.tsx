'use client'

import * as React from 'react'
import { Card } from '@/components/ui/card'
import { Switch } from '@/components/ui/switch'
import { RelayIndicator } from '@/components/ui/relay-indicator'
import { Zap, CheckSquare, Square } from 'lucide-react'
import { cn } from '@/lib/utils'

interface RelayCardProps {
  channelIndex: number
  name?: string
  active: boolean
  isSelected: boolean
  onToggle: (index: number) => void
  onToggleSelect: (index: number) => void
  disabled?: boolean
}

export function RelayCard({
  channelIndex,
  name,
  active,
  isSelected,
  onToggle,
  onToggleSelect,
  disabled = false,
}: RelayCardProps) {
  const channelNum = channelIndex + 1

  return (
    <Card
      className={cn(
        'group relative overflow-hidden rounded-xl border transition-all duration-200 select-none',
        active
          ? 'border-cyan-500/60 bg-gradient-to-b from-[#14232c] to-[#12121a] shadow-glow-cyan-sm'
          : 'border-zinc-800 bg-[#12121a] hover:border-zinc-700',
        isSelected && 'ring-2 ring-cyan-400 border-cyan-400'
      )}
    >
      {/* Glow highlight top edge when ON */}
      {active && (
        <div className="absolute top-0 left-0 right-0 h-1 bg-gradient-to-r from-cyan-500 via-cyan-300 to-cyan-500 shadow-glow-cyan" />
      )}

      <div className="p-4">
        {/* Card Header: Channel & Select Checkbox */}
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-2">
            <RelayIndicator channelIndex={channelIndex} active={active} size="md" />
            <div>
              <div className="flex items-center space-x-1.5">
                <span className="font-mono text-sm font-bold text-zinc-100">
                  CH {channelNum}
                </span>
                <span
                  className={cn(
                    'rounded px-1.5 py-0.2 text-[9px] font-mono font-bold uppercase tracking-wider',
                    active
                      ? 'bg-cyan-500/20 text-cyan-300'
                      : 'bg-zinc-800 text-zinc-400'
                  )}
                >
                  {active ? 'ON' : 'OFF'}
                </span>
              </div>
              <p className="text-[11px] text-zinc-400">
                {name || `Relay #${channelNum}`}
              </p>
            </div>
          </div>

          {/* Selection toggle */}
          <button
            type="button"
            onClick={(e) => {
              e.stopPropagation()
              onToggleSelect(channelIndex)
            }}
            className="text-zinc-500 hover:text-cyan-400 transition-colors p-1"
            title={isSelected ? 'Deselect relay' : 'Select relay for batch action'}
          >
            {isSelected ? (
              <CheckSquare className="h-4 w-4 text-cyan-400" />
            ) : (
              <Square className="h-4 w-4" />
            )}
          </button>
        </div>

        {/* Card Body: Switch & Quick Toggle Button */}
        <div className="mt-4 flex items-center justify-between pt-2 border-t border-zinc-800/60">
          <div className="flex items-center space-x-1 text-[11px] text-zinc-400">
            <Zap className={cn('h-3 w-3', active ? 'text-cyan-400' : 'text-zinc-500')} />
            <span>{active ? 'Energized' : 'Open'}</span>
          </div>

          <div className="flex items-center space-x-2">
            <Switch
              checked={active}
              onCheckedChange={() => onToggle(channelIndex)}
              disabled={disabled}
              className="data-[state=checked]:bg-cyan-500 data-[state=checked]:shadow-glow-cyan-sm"
            />
          </div>
        </div>
      </div>
    </Card>
  )
}
