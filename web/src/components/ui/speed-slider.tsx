'use client'

import * as React from 'react'
import { Slider } from './slider'
import { Button } from './button'
import { Gauge, Zap } from 'lucide-react'
import { SPEED_PRESETS } from '@/lib/utils'

interface SpeedSliderProps {
  value: number
  onChange: (value: number) => void
  disabled?: boolean
  min?: number
  max?: number
}

export function SpeedSlider({
  value,
  onChange,
  disabled = false,
  min = 40,
  max = 1000,
}: SpeedSliderProps) {
  return (
    <div className="space-y-3 rounded-lg border border-zinc-800 bg-zinc-950/40 p-4">
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-2">
          <Gauge className="h-4 w-4 text-cyan-400" />
          <span className="text-xs font-medium text-zinc-300 uppercase tracking-wider">
            Step Interval / Speed
          </span>
        </div>
        <div className="flex items-center space-x-1.5">
          <span className="font-mono text-sm font-bold text-cyan-400">{value}</span>
          <span className="text-xs text-zinc-400">ms/step</span>
        </div>
      </div>

      <Slider
        value={[value]}
        onValueChange={([val]) => onChange(val)}
        min={min}
        max={max}
        step={5}
        disabled={disabled}
        className="py-1"
      />

      <div className="flex flex-wrap items-center gap-1.5 pt-1">
        {SPEED_PRESETS.map((preset) => {
          const isActive = value === preset.value
          return (
            <Button
              key={preset.value}
              type="button"
              variant={isActive ? 'cyanGlow' : 'outline'}
              size="sm"
              disabled={disabled}
              onClick={() => onChange(preset.value)}
              className="h-6 px-2 text-[10px] font-mono"
            >
              <Zap className="mr-1 h-2.5 w-2.5" />
              {preset.label}
            </Button>
          )
        })}
      </div>
    </div>
  )
}
