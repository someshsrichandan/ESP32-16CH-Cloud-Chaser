'use client'

import * as React from 'react'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { RelayIndicator } from '@/components/ui/relay-indicator'
import { Zap } from 'lucide-react'

interface RelayMiniGridProps {
  relays: boolean[]
}

export function RelayMiniGrid({ relays }: RelayMiniGridProps) {
  const safeRelays = Array.isArray(relays) && relays.length === 16 ? relays : new Array(16).fill(false)
  const activeCount = safeRelays.filter(Boolean).length

  return (
    <Card className="border-zinc-800 bg-[#12121a]">
      <CardHeader className="flex flex-row items-center justify-between pb-3">
        <div className="flex items-center space-x-2">
          <Zap className="h-4 w-4 text-cyan-400" />
          <CardTitle className="text-sm font-semibold">16-Channel Relay State</CardTitle>
        </div>
        <div className="flex items-center space-x-1.5 font-mono text-xs">
          <span className="font-bold text-cyan-400">{activeCount}</span>
          <span className="text-zinc-500">/ 16 Active</span>
        </div>
      </CardHeader>

      <CardContent>
        {/* 4x4 Grid Matrix */}
        <div className="grid grid-cols-4 gap-2.5 rounded-lg border border-zinc-800/80 bg-zinc-950/60 p-3">
          {safeRelays.map((active, idx) => (
            <div
              key={idx}
              className={`flex items-center justify-between rounded-md p-2 transition-all duration-200 border ${
                active
                  ? 'border-cyan-500/50 bg-cyan-950/20 shadow-glow-cyan-sm'
                  : 'border-zinc-800/80 bg-zinc-900/40'
              }`}
            >
              <span className="font-mono text-[11px] font-semibold text-zinc-400">
                CH {idx + 1}
              </span>
              <RelayIndicator channelIndex={idx} active={active} size="sm" />
            </div>
          ))}
        </div>

        {/* Legend */}
        <div className="mt-3 flex items-center justify-between text-[11px] text-zinc-500 font-mono">
          <div className="flex items-center space-x-1.5">
            <span className="h-2 w-2 rounded-full bg-cyan-400 shadow-glow-cyan-sm" />
            <span>Energized (Active LOW)</span>
          </div>
          <div className="flex items-center space-x-1.5">
            <span className="h-2 w-2 rounded-full bg-zinc-700" />
            <span>Open (OFF)</span>
          </div>
        </div>
      </CardContent>
    </Card>
  )
}
