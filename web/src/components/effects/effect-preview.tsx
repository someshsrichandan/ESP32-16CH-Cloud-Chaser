'use client'

import * as React from 'react'
import { cn } from '@/lib/utils'

interface EffectPreviewProps {
  pattern: string
  category: string
  isActive?: boolean
  speed?: number
  className?: string
}

export function EffectPreview({
  pattern,
  category,
  isActive = false,
  speed = 120,
  className,
}: EffectPreviewProps) {
  const [step, setStep] = React.useState(0)

  // Simulation loop for realistic preview visualization of the 16 channels
  React.useEffect(() => {
    // Keep preview interval smooth (capped between 60ms and 300ms for browser rendering)
    const intervalTime = Math.max(60, Math.min(speed, 250))

    const timer = setInterval(() => {
      setStep((prev) => (prev + 1) % 32)
    }, intervalTime)

    return () => clearInterval(timer)
  }, [speed])

  // Compute boolean state for each of the 16 channels based on pattern & current step
  const getRelayState = (ch: number): boolean => {
    switch (pattern) {
      case 'chase_left':
        return ch === 15 - (step % 16)

      case 'chase_right':
        return ch === step % 16

      case 'ping_pong': {
        const pStep = step % 30
        const currentPos = pStep < 16 ? pStep : 30 - pStep
        return ch === currentPos
      }

      case 'center_out': {
        const radius = step % 8
        return ch === 7 - radius || ch === 8 + radius
      }

      case 'edges_in': {
        const inward = step % 8
        return ch === inward || ch === 15 - inward
      }

      case 'zip_unzip': {
        const zStep = step % 32
        if (zStep < 16) {
          return ch <= zStep // Fill up
        } else {
          return ch >= zStep - 16 // Unzip
        }
      }

      case 'dual_snake': {
        const s1 = step % 16
        const s2 = (step + 8) % 16
        return (
          ch === s1 ||
          ch === (s1 + 15) % 16 ||
          ch === s2 ||
          ch === (s2 + 15) % 16
        )
      }

      case 'binary_counter':
        return ((step & (1 << (ch % 8))) !== 0)

      case 'sparkle':
        // pseudo-random glitter
        return ((step * 17 + ch * 13) % 7) === 0

      case 'twinkle':
        return ((step * 7 + ch * 23) % 9) < 2

      case 'wave_fill': {
        const wStep = step % 16
        return (ch >= wStep && ch < wStep + 4) || (ch + 16 >= wStep && ch + 16 < wStep + 4)
      }

      case 'heartbeat': {
        const hStep = step % 12
        return (hStep === 0 || hStep === 2) // double beat all 16 channels
      }

      case 'cylon': {
        const cStep = step % 28
        const cPos = cStep < 14 ? cStep + 1 : 28 - cStep
        return ch === cPos || ch === cPos - 1 || ch === cPos + 1
      }

      case 'alternating':
        return (step % 2 === 0 ? ch % 2 === 0 : ch % 2 === 1)

      case 'dominos': {
        const dStep = step % 18
        return ch <= dStep
      }

      case 'marquee':
        return (ch + step) % 3 === 0

      case 'raindrop': {
        const center = 8
        const rRad = step % 8
        return ch === center - rRad || ch === center + rRad
      }

      case 'fireworks': {
        const fStep = step % 16
        if (fStep < 12) return ch === fStep
        return true // finale explosion
      }

      case 'knight_rider': {
        const kStep = step % 30
        const kPos = kStep < 15 ? kStep : 30 - kStep
        return ch === kPos || ch === kPos - 1
      }

      case 'meteor': {
        const mPos = step % 20
        return ch === mPos || ch === mPos - 1 || ch === mPos - 2
      }

      case 'double_bounce': {
        const bStep = step % 16
        return ch === bStep || ch === 15 - bStep
      }

      case 'cross_over': {
        const x1 = step % 16
        const x2 = 15 - (step % 16)
        return ch === x1 || ch === x2
      }

      case 'strobe_burst':
        return step % 2 === 0

      case 'stack_build': {
        const stackStep = step % 16
        return ch >= 15 - stackStep
      }

      case 'reverse_stack': {
        const rStep = step % 16
        return ch <= 15 - rStep
      }

      case 'quad_runner': {
        const qStep = step % 4
        return ch % 4 === qStep
      }

      case 'in_out_breathe': {
        const bPhase = step % 16
        const dist = bPhase < 8 ? bPhase : 15 - bPhase
        return ch >= 7 - dist && ch <= 8 + dist
      }

      case 'segment_stride': {
        const stride = (Math.floor(step / 2)) % 4
        return Math.floor(ch / 4) === stride
      }

      case 'police_strobe': {
        const pPhase = step % 12
        if (pPhase < 6) {
          // Left side strobing
          return ch < 8 && (pPhase % 2 === 0)
        } else {
          // Right side strobing
          return ch >= 8 && (pPhase % 2 === 0)
        }
      }

      case 'curtain_reveal': {
        const cDist = step % 8
        return ch <= 7 - cDist || ch >= 8 + cDist
      }

      default:
        return (ch + step) % 4 === 0
    }
  }

  return (
    <div
      className={cn(
        'relative flex items-center justify-between rounded-lg border border-zinc-800/80 bg-zinc-950/80 p-2.5 shadow-inner transition-colors',
        isActive && 'border-cyan-500/40 bg-cyan-950/20',
        className
      )}
    >
      <div className="flex w-full items-center justify-between gap-1">
        {Array.from({ length: 16 }).map((_, i) => {
          const isLit = getRelayState(i)
          return (
            <div
              key={i}
              className={cn(
                'h-4 flex-1 rounded-sm transition-all duration-100',
                isLit
                  ? 'bg-cyan-400 shadow-glow-cyan-sm scale-y-110'
                  : 'bg-zinc-800/80 opacity-60'
              )}
            />
          )
        })}
      </div>
    </div>
  )
}
