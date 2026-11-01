'use client'

import * as React from 'react'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { Progress } from '@/components/ui/progress'
import { ModeBadge } from '@/components/ui/mode-badge'
import { DeviceStatus } from '@/types/device'
import { RotateCw, FastForward, PlayCircle } from 'lucide-react'

interface CycleProgressProps {
  status: DeviceStatus | null
}

export function CycleProgress({ status }: CycleProgressProps) {
  const currentEffect = status?.effect || 'Left Chaser'
  const currentCycle = status?.cycle || 1
  const totalCycles = status?.totalCycles || 3
  const progressPercent = Math.min(100, Math.round((currentCycle / Math.max(totalCycles, 1)) * 100))
  const mode = status?.mode || 'auto'

  return (
    <Card className="border-zinc-800 bg-[#12121a]">
      <CardHeader className="flex flex-row items-center justify-between pb-3">
        <div className="flex items-center space-x-2">
          <RotateCw className="h-4 w-4 text-cyan-400" />
          <CardTitle className="text-sm font-semibold">Active Sequence & Cycles</CardTitle>
        </div>
        <ModeBadge mode={mode} />
      </CardHeader>

      <CardContent className="space-y-4">
        {/* Active Effect & Next */}
        <div className="flex items-center justify-between rounded-lg border border-zinc-800/80 bg-zinc-950/40 p-3">
          <div>
            <span className="text-[10px] uppercase font-mono tracking-wider text-zinc-500">
              Playing Effect
            </span>
            <div className="flex items-center space-x-2 mt-0.5">
              <PlayCircle className="h-4 w-4 text-cyan-400" />
              <span className="font-bold text-base text-zinc-100">{currentEffect}</span>
              <span className="font-mono text-xs text-zinc-500">
                (FX {status?.effectIndex ?? 0})
              </span>
            </div>
          </div>

          <div className="text-right">
            <span className="text-[10px] uppercase font-mono tracking-wider text-zinc-500">
              Next Queued
            </span>
            <div className="flex items-center justify-end space-x-1.5 mt-0.5 text-zinc-400 text-xs">
              <FastForward className="h-3.5 w-3.5 text-zinc-500" />
              <span>{status?.nextEffect || 'Auto Sequence'}</span>
            </div>
          </div>
        </div>

        {/* Cycle Progress Bar */}
        <div className="space-y-2">
          <div className="flex items-center justify-between text-xs font-mono">
            <span className="text-zinc-400">Repeats Completed</span>
            <span className="font-bold text-cyan-400">
              Cycle {currentCycle} / {totalCycles} ({progressPercent}%)
            </span>
          </div>

          <Progress value={progressPercent} className="h-2" />

          <div className="flex justify-between text-[10px] text-zinc-500 font-mono pt-1">
            <span>Cycle Start</span>
            <span>Advance to Next Effect ({totalCycles}x repeat target)</span>
          </div>
        </div>
      </CardContent>
    </Card>
  )
}
