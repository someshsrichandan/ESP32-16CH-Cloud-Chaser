import * as React from 'react'
import { cn } from '@/lib/utils'

interface RelayIndicatorProps {
  channelIndex: number
  active: boolean
  size?: 'sm' | 'md' | 'lg'
  showNumber?: boolean
  className?: string
}

export function RelayIndicator({
  channelIndex,
  active,
  size = 'md',
  showNumber = false,
  className,
}: RelayIndicatorProps) {
  const sizeClasses = {
    sm: 'h-3.5 w-3.5 text-[8px]',
    md: 'h-6 w-6 text-[10px]',
    lg: 'h-8 w-8 text-xs',
  }

  return (
    <div
      className={cn(
        'relative flex items-center justify-center rounded-full font-mono font-bold transition-all duration-150',
        sizeClasses[size],
        active
          ? 'bg-cyan-400 text-zinc-950 shadow-glow-cyan'
          : 'bg-zinc-800 text-zinc-500 border border-zinc-700/60',
        className
      )}
      title={`Channel ${channelIndex + 1}: ${active ? 'ENERGIZED (ON)' : 'OFF'}`}
    >
      {showNumber ? channelIndex + 1 : null}
      {active && (
        <span className="absolute -inset-0.5 rounded-full bg-cyan-400/30 animate-pulse pointer-events-none" />
      )}
    </div>
  )
}
