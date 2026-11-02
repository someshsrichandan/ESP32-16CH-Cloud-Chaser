import * as React from 'react'
import { Badge } from './badge'
import { DeviceMode } from '@/types/device'
import { Play, Pause, Sliders } from 'lucide-react'

interface ModeBadgeProps {
  mode: DeviceMode
  className?: string
}

export function ModeBadge({ mode, className }: ModeBadgeProps) {
  switch (mode) {
    case 'auto':
      return (
        <Badge
          variant="default"
          className={`flex items-center space-x-1.5 bg-cyan-500/15 text-cyan-300 border-cyan-500/40 shadow-glow-cyan-sm uppercase font-mono tracking-wider ${className}`}
        >
          <Play className="h-3 w-3 fill-cyan-400 text-cyan-400" />
          <span>Auto Chaser</span>
        </Badge>
      )
    case 'manual':
      return (
        <Badge
          variant="warning"
          className={`flex items-center space-x-1.5 bg-amber-500/15 text-amber-300 border-amber-500/40 shadow-glow-amber uppercase font-mono tracking-wider ${className}`}
        >
          <Sliders className="h-3 w-3 text-amber-400" />
          <span>Manual Mode</span>
        </Badge>
      )
    case 'stopped':
    default:
      return (
        <Badge
          variant="secondary"
          className={`flex items-center space-x-1.5 bg-zinc-800 text-zinc-400 border-zinc-700 uppercase font-mono tracking-wider ${className}`}
        >
          <Pause className="h-3 w-3 text-zinc-400" />
          <span>Standby / Idle</span>
        </Badge>
      )
  }
}
