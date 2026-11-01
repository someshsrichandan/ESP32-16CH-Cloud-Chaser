'use client'

import * as React from 'react'
import { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import { EffectPreview } from './effect-preview'
import { EffectPreset } from '@/types/device'
import { Play, Check, Zap, Sliders } from 'lucide-react'
import { cn } from '@/lib/utils'

interface EffectCardProps {
  effect: EffectPreset
  isActive: boolean
  onSelect: (id: number, speed?: number) => void
  disabled?: boolean
}

export function EffectCard({
  effect,
  isActive,
  onSelect,
  disabled = false,
}: EffectCardProps) {
  const [speed, setSpeed] = React.useState(effect.defaultSpeed)

  const categoryColorMap: Record<string, 'default' | 'secondary' | 'warning' | 'destructive' | 'outline'> = {
    chase: 'default',
    wave: 'secondary',
    pattern: 'outline',
    strobe: 'destructive',
    fill: 'warning',
    random: 'secondary',
  }

  return (
    <Card
      className={cn(
        'group flex flex-col justify-between overflow-hidden rounded-xl border transition-all duration-200',
        isActive
          ? 'border-cyan-400/80 bg-gradient-to-b from-[#13232c] to-[#12121a] shadow-glow-cyan'
          : 'border-zinc-800 bg-[#12121a] hover:border-zinc-700/80 hover:bg-[#151522]'
      )}
    >
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <Badge
            variant={categoryColorMap[effect.category] || 'default'}
            className="uppercase tracking-wider font-mono text-[9px] px-2 py-0.5"
          >
            {effect.category}
          </Badge>

          <span className="font-mono text-xs text-zinc-500 font-semibold">
            FX #{effect.id.toString().padStart(2, '0')}
          </span>
        </div>

        <CardTitle className="text-base font-bold tracking-tight text-zinc-100 group-hover:text-cyan-300 transition-colors mt-2">
          {effect.name}
        </CardTitle>

        <CardDescription className="line-clamp-2 min-h-[36px] text-xs text-zinc-400">
          {effect.description}
        </CardDescription>
      </CardHeader>

      <CardContent className="space-y-3 pb-3">
        {/* Real-time CSS Animated 16-channel preview */}
        <div className="space-y-1">
          <div className="flex items-center justify-between text-[10px] font-mono text-zinc-500">
            <span>CH 1</span>
            <span className="text-cyan-400/80">16-Channel Waveform</span>
            <span>CH 16</span>
          </div>
          <EffectPreview
            pattern={effect.animationPattern}
            category={effect.category}
            isActive={isActive}
            speed={speed}
          />
        </div>

        {/* Speed Adjustment */}
        <div className="flex items-center justify-between rounded-lg bg-zinc-950/40 border border-zinc-800/80 px-2.5 py-1.5 text-xs">
          <div className="flex items-center space-x-1.5 text-zinc-400">
            <Zap className="h-3 w-3 text-cyan-400" />
            <span className="font-mono text-[11px]">Speed:</span>
          </div>
          <div className="flex items-center space-x-1">
            <input
              type="range"
              min={40}
              max={500}
              step={10}
              value={speed}
              onChange={(e) => setSpeed(Number(e.target.value))}
              className="h-1.5 w-20 cursor-pointer accent-cyan-400 bg-zinc-800 rounded-lg"
            />
            <span className="w-12 text-right font-mono text-[11px] font-bold text-cyan-300">
              {speed}ms
            </span>
          </div>
        </div>
      </CardContent>

      <CardFooter className="pt-2 border-t border-zinc-800/60">
        <Button
          variant={isActive ? 'cyanGlow' : 'secondary'}
          size="sm"
          disabled={disabled}
          onClick={() => onSelect(effect.id, speed)}
          className="w-full text-xs font-semibold"
        >
          {isActive ? (
            <>
              <Check className="mr-1.5 h-3.5 w-3.5 text-cyan-400" />
              Active on Relays
            </>
          ) : (
            <>
              <Play className="mr-1.5 h-3.5 w-3.5" />
              Trigger Effect
            </>
          )}
        </Button>
      </CardFooter>
    </Card>
  )
}
