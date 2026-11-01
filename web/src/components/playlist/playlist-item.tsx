'use client'

import * as React from 'react'
import { useSortable } from '@dnd-kit/sortable'
import { CSS } from '@dnd-kit/utilities'
import { GripVertical, Trash2, Clock, RotateCcw } from 'lucide-react'
import { Switch } from '@/components/ui/switch'
import { Button } from '@/components/ui/button'
import { PlaylistEntry } from '@/types/device'
import { cn } from '@/lib/utils'

interface PlaylistItemProps {
  item: PlaylistEntry
  index: number
  onUpdate: (id: string, updates: Partial<PlaylistEntry>) => void
  onRemove: (id: string) => void
}

export function PlaylistItem({
  item,
  index,
  onUpdate,
  onRemove,
}: PlaylistItemProps) {
  const {
    attributes,
    listeners,
    setNodeRef,
    transform,
    transition,
    isDragging,
  } = useSortable({ id: item.id })

  const style = {
    transform: CSS.Transform.toString(transform),
    transition,
  }

  return (
    <div
      ref={setNodeRef}
      style={style}
      className={cn(
        'group flex flex-wrap items-center justify-between gap-3 rounded-xl border border-zinc-800 bg-[#12121a] p-3.5 shadow-md transition-all duration-150',
        isDragging && 'opacity-60 ring-2 ring-cyan-400 z-50 bg-[#1a1a26]',
        !item.enabled && 'opacity-50'
      )}
    >
      {/* Left: Drag Handle, Number, Effect Name */}
      <div className="flex items-center space-x-3">
        <button
          type="button"
          {...attributes}
          {...listeners}
          className="cursor-grab touch-none p-1 text-zinc-500 hover:text-cyan-400 active:cursor-grabbing"
          title="Drag to reorder"
        >
          <GripVertical className="h-4 w-4" />
        </button>

        <span className="flex h-6 w-6 items-center justify-center rounded-md bg-zinc-900 border border-zinc-800 font-mono text-xs text-zinc-400 font-bold">
          {index + 1}
        </span>

        <div>
          <h4 className="font-semibold text-sm text-zinc-100">{item.effectName}</h4>
          <span className="font-mono text-[10px] text-zinc-500">
            FX #{item.effectIndex.toString().padStart(2, '0')}
          </span>
        </div>
      </div>

      {/* Right Controls: Cycles, Speed, Enable Switch, Remove */}
      <div className="flex flex-wrap items-center gap-3">
        {/* Cycle input */}
        <div className="flex items-center space-x-1.5 rounded-lg border border-zinc-800 bg-zinc-950/60 px-2.5 py-1">
          <RotateCcw className="h-3 w-3 text-cyan-400" />
          <span className="text-[10px] font-mono text-zinc-400">Cycles:</span>
          <input
            type="number"
            min={1}
            max={50}
            value={item.cycles}
            onChange={(e) => onUpdate(item.id, { cycles: Math.max(1, parseInt(e.target.value) || 1) })}
            className="w-12 bg-transparent text-center font-mono text-xs font-bold text-zinc-100 focus:outline-none"
          />
        </div>

        {/* Speed input */}
        <div className="flex items-center space-x-1.5 rounded-lg border border-zinc-800 bg-zinc-950/60 px-2.5 py-1">
          <Clock className="h-3 w-3 text-cyan-400" />
          <span className="text-[10px] font-mono text-zinc-400">Speed:</span>
          <input
            type="number"
            min={40}
            max={1000}
            step={10}
            value={item.speedMs}
            onChange={(e) => onUpdate(item.id, { speedMs: Math.max(40, parseInt(e.target.value) || 120) })}
            className="w-14 bg-transparent text-center font-mono text-xs font-bold text-zinc-100 focus:outline-none"
          />
          <span className="text-[10px] text-zinc-500 font-mono">ms</span>
        </div>

        {/* Enable / Disable toggle */}
        <div className="flex items-center space-x-1.5">
          <Switch
            checked={item.enabled}
            onCheckedChange={(checked) => onUpdate(item.id, { enabled: checked })}
          />
        </div>

        {/* Delete button */}
        <Button
          variant="ghost"
          size="iconSm"
          onClick={() => onRemove(item.id)}
          className="text-zinc-500 hover:text-red-400 hover:bg-red-500/10"
          title="Remove from playlist"
        >
          <Trash2 className="h-4 w-4" />
        </Button>
      </div>
    </div>
  )
}
