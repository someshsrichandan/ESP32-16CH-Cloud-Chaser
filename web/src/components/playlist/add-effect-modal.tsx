'use client'

import * as React from 'react'
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogDescription,
} from '@/components/ui/dialog'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import { Badge } from '@/components/ui/badge'
import { EFFECTS_DATA } from '@/lib/effects-data'
import { EffectPreset, PlaylistEntry } from '@/types/device'
import { Search, Plus, Sparkles } from 'lucide-react'

interface AddEffectModalProps {
  open: boolean
  onOpenChange: (open: boolean) => void
  onAddEffect: (entry: Omit<PlaylistEntry, 'id'>) => void
}

export function AddEffectModal({
  open,
  onOpenChange,
  onAddEffect,
}: AddEffectModalProps) {
  const [search, setSearch] = React.useState('')
  const [selectedCategory, setSelectedCategory] = React.useState('all')

  const filteredEffects = React.useMemo(() => {
    return EFFECTS_DATA.filter((fx) => {
      const matchesSearch =
        fx.name.toLowerCase().includes(search.toLowerCase()) ||
        fx.description.toLowerCase().includes(search.toLowerCase()) ||
        fx.tags.some((t) => t.toLowerCase().includes(search.toLowerCase()))
      const matchesCategory =
        selectedCategory === 'all' || fx.category === selectedCategory
      return matchesSearch && matchesCategory
    })
  }, [search, selectedCategory])

  const handleSelect = (fx: EffectPreset) => {
    onAddEffect({
      effectIndex: fx.id,
      effectName: fx.name,
      cycles: 3,
      speedMs: fx.defaultSpeed,
      enabled: true,
    })
    onOpenChange(false)
  }

  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="max-w-2xl bg-[#12121a] border-zinc-800 text-zinc-100 max-h-[85vh] flex flex-col">
        <DialogHeader className="pb-3 border-b border-zinc-800">
          <div className="flex items-center space-x-2">
            <Sparkles className="h-5 w-5 text-cyan-400" />
            <DialogTitle>Add Effect to Playlist</DialogTitle>
          </div>
          <DialogDescription className="text-zinc-400 text-xs">
            Select a lighting pattern from the 18 native Arduino chase patterns.
          </DialogDescription>

          {/* Search Box */}
          <div className="relative mt-2">
            <Search className="absolute left-3 top-2.5 h-4 w-4 text-zinc-500" />
            <Input
              type="text"
              placeholder="Search effects by name, tag, or aesthetic..."
              value={search}
              onChange={(e) => setSearch(e.target.value)}
              className="pl-9 bg-zinc-950/60 border-zinc-800 text-xs"
            />
          </div>
        </DialogHeader>

        {/* Scrollable Effects List */}
        <div className="flex-1 overflow-y-auto space-y-2 py-3 pr-1">
          {filteredEffects.length === 0 ? (
            <div className="py-12 text-center text-zinc-500 text-xs">
              No matching effects found. Try a different search term.
            </div>
          ) : (
            filteredEffects.map((fx) => (
              <div
                key={fx.id}
                onClick={() => handleSelect(fx)}
                className="group flex items-center justify-between rounded-lg border border-zinc-800/80 bg-zinc-900/40 p-3 hover:border-cyan-500/40 hover:bg-cyan-950/20 cursor-pointer transition-all duration-150"
              >
                <div className="space-y-1">
                  <div className="flex items-center space-x-2">
                    <span className="font-mono text-xs font-bold text-zinc-500 group-hover:text-cyan-400">
                      #{fx.id.toString().padStart(2, '0')}
                    </span>
                    <h4 className="font-semibold text-sm text-zinc-200 group-hover:text-zinc-100">
                      {fx.name}
                    </h4>
                    <Badge variant="outline" className="text-[9px] uppercase px-1.5 py-0">
                      {fx.category}
                    </Badge>
                  </div>
                  <p className="text-xs text-zinc-400 line-clamp-1">{fx.description}</p>
                </div>

                <Button
                  variant="cyanGlow"
                  size="sm"
                  className="h-8 px-3 text-xs opacity-90 group-hover:opacity-100"
                >
                  <Plus className="mr-1 h-3.5 w-3.5" />
                  Add
                </Button>
              </div>
            ))
          )}
        </div>
      </DialogContent>
    </Dialog>
  )
}
