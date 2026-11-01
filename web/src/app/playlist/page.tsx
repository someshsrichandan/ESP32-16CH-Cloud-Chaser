'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import {
  DndContext,
  closestCenter,
  KeyboardSensor,
  PointerSensor,
  useSensor,
  useSensors,
  DragEndEvent,
} from '@dnd-kit/core'
import {
  arrayMove,
  SortableContext,
  sortableKeyboardCoordinates,
  verticalListSortingStrategy,
} from '@dnd-kit/sortable'
import { PlaylistItem } from '@/components/playlist/playlist-item'
import { AddEffectModal } from '@/components/playlist/add-effect-modal'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Switch } from '@/components/ui/switch'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { PlaylistEntry, CommandType } from '@/types/device'
import { toast } from 'sonner'
import {
  Play,
  Square,
  Plus,
  Shuffle,
  Save,
  Download,
  ListMusic,
  RotateCcw,
  Sparkles,
} from 'lucide-react'

const DEFAULT_PLAYLIST: PlaylistEntry[] = [
  { id: '1', effectIndex: 0, effectName: 'Left Chaser', cycles: 3, speedMs: 100, enabled: true },
  { id: '2', effectIndex: 3, effectName: 'Center Out', cycles: 4, speedMs: 120, enabled: true },
  { id: '3', effectIndex: 12, effectName: 'Cylon Scanner', cycles: 3, speedMs: 90, enabled: true },
  { id: '4', effectIndex: 18, effectName: 'Knight Rider', cycles: 3, speedMs: 85, enabled: true },
  { id: '5', effectIndex: 8, effectName: 'Sparkle / Stardust', cycles: 5, speedMs: 70, enabled: true },
  { id: '6', effectIndex: 28, effectName: 'Police Strobe', cycles: 2, speedMs: 60, enabled: true },
]

export default function PlaylistPage() {
  const { activeDeviceId } = useActiveDevice()
  const { sendCommand, isPending } = useDeviceCommand(activeDeviceId)

  const [playlist, setPlaylist] = React.useState<PlaylistEntry[]>(DEFAULT_PLAYLIST)
  const [modalOpen, setModalOpen] = React.useState(false)
  const [shuffleMode, setShuffleMode] = React.useState(false)
  const [isPlaying, setIsPlaying] = React.useState(false)

  // Load from localStorage on mount
  React.useEffect(() => {
    try {
      const saved = localStorage.getItem('esp32_relay_playlist')
      if (saved) {
        const parsed = JSON.parse(saved)
        if (Array.isArray(parsed) && parsed.length > 0) {
          setPlaylist(parsed)
        }
      }
    } catch {
      // Ignore fallback
    }
  }, [])

  const sensors = useSensors(
    useSensor(PointerSensor),
    useSensor(KeyboardSensor, {
      coordinateGetter: sortableKeyboardCoordinates,
    })
  )

  const handleDragEnd = (event: DragEndEvent) => {
    const { active, over } = event
    if (over && active.id !== over.id) {
      setPlaylist((items) => {
        const oldIndex = items.findIndex((i) => i.id === active.id)
        const newIndex = items.findIndex((i) => i.id === over.id)
        return arrayMove(items, oldIndex, newIndex)
      })
    }
  }

  const handleUpdate = (id: string, updates: Partial<PlaylistEntry>) => {
    setPlaylist((items) =>
      items.map((it) => (it.id === id ? { ...it, ...updates } : it))
    )
  }

  const handleRemove = (id: string) => {
    setPlaylist((items) => items.filter((it) => it.id !== id))
    toast.info('Item removed from playlist')
  }

  const handleAdd = (entry: Omit<PlaylistEntry, 'id'>) => {
    const newItem: PlaylistEntry = {
      ...entry,
      id: `pl_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
    }
    setPlaylist((prev) => [...prev, newItem])
    toast.success(`Added ${entry.effectName} to playlist`)
  }

  const handleSave = () => {
    try {
      localStorage.setItem('esp32_relay_playlist', JSON.stringify(playlist))
      toast.success('Playlist saved to local browser cache')
    } catch (err: any) {
      toast.error('Failed to save playlist')
    }
  }

  const handleStartPlaylist = async () => {
    const enabledItems = playlist.filter((it) => it.enabled)
    if (enabledItems.length === 0) {
      toast.error('No enabled effects in playlist to play')
      return
    }

    setIsPlaying(true)
    const first = enabledItems[0]

    // Set first effect and cycle repeats
    await sendCommand(CommandType.EFFECT, { id: first.effectIndex })
    await sendCommand(CommandType.REPEATS, { v: first.cycles })
    await sendCommand(CommandType.SPEED, { v: first.speedMs })
    await sendCommand(CommandType.ACTION, { a: 'auto_on' }, {
      successMsg: `Playlist Started! Playing ${first.effectName} (${first.cycles}x cycles)`,
    })
  }

  const handleStopPlaylist = async () => {
    setIsPlaying(false)
    await sendCommand(CommandType.ACTION, { a: 'all_off' }, { successMsg: 'Playlist Execution Halted' })
  }

  const totalRuntimeCycles = playlist
    .filter((it) => it.enabled)
    .reduce((acc, curr) => acc + curr.cycles, 0)

  return (
    <motion.div
      initial={{ opacity: 0, y: 12 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.3 }}
      className="space-y-6"
    >
      {/* Header */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 border-b border-zinc-800/80 pb-5">
        <div>
          <div className="flex items-center space-x-3">
            <h1 className="text-2xl font-bold tracking-tight text-zinc-100">
              Show & Playlist Sequencer
            </h1>
            <span className="rounded-full bg-cyan-500/20 px-2.5 py-0.5 text-xs font-mono font-bold text-cyan-300 border border-cyan-500/40">
              {playlist.length} Queued Items
            </span>
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Construct automated shows by ordering effects, configuring repeat cycles, and timing speeds.
          </p>
        </div>

        {/* Action Buttons */}
        <div className="flex items-center space-x-2">
          {isPlaying ? (
            <Button
              variant="destructive"
              size="sm"
              onClick={handleStopPlaylist}
              disabled={isPending}
              className="text-xs font-semibold shadow-glow-red"
            >
              <Square className="mr-1.5 h-3.5 w-3.5 fill-white" />
              Stop Playlist
            </Button>
          ) : (
            <Button
              variant="default"
              size="sm"
              onClick={handleStartPlaylist}
              disabled={isPending}
              className="text-xs font-semibold shadow-glow-cyan-sm"
            >
              <Play className="mr-1.5 h-3.5 w-3.5 fill-zinc-950" />
              Run Playlist
            </Button>
          )}

          <Button
            variant="outline"
            size="sm"
            onClick={handleSave}
            className="text-xs text-zinc-300 hover:text-zinc-100"
          >
            <Save className="mr-1.5 h-3.5 w-3.5" />
            Save
          </Button>

          <Button
            variant="cyanGlow"
            size="sm"
            onClick={() => setModalOpen(true)}
            className="text-xs font-semibold"
          >
            <Plus className="mr-1.5 h-3.5 w-3.5" />
            Add Effect
          </Button>
        </div>
      </div>

      {/* Playlist Overview Card */}
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-3">
        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Total Steps / Loops
          </span>
          <div className="mt-1 flex items-baseline space-x-2">
            <span className="text-2xl font-bold font-mono text-cyan-400">
              {totalRuntimeCycles}
            </span>
            <span className="text-xs text-zinc-400">Repeats in cycle</span>
          </div>
        </div>

        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Playback Mode
          </span>
          <div className="mt-1 flex items-center justify-between">
            <span className="text-sm font-semibold text-zinc-200">
              {shuffleMode ? 'Random Shuffle' : 'Strict Sequential'}
            </span>
            <div className="flex items-center space-x-2">
              <Shuffle className="h-4 w-4 text-cyan-400" />
              <Switch
                checked={shuffleMode}
                onCheckedChange={(checked) => {
                  setShuffleMode(checked)
                  sendCommand(CommandType.ACTION, { a: 'toggle_shuffle' })
                }}
              />
            </div>
          </div>
        </div>

        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Execution Status
          </span>
          <div className="mt-1 flex items-center space-x-2">
            <span className={`h-2.5 w-2.5 rounded-full ${isPlaying ? 'bg-cyan-400 shadow-glow-cyan animate-pulse' : 'bg-zinc-600'}`} />
            <span className="text-sm font-semibold text-zinc-200">
              {isPlaying ? 'Sequencer Running' : 'Standby'}
            </span>
          </div>
        </div>
      </div>

      {/* Sortable List Area */}
      <div className="space-y-3">
        <div className="flex items-center justify-between px-1 text-xs font-mono text-zinc-400">
          <span>DRAG ORDER / SEQUENCE</span>
          <span>PARAMETERS & CONTROLS</span>
        </div>

        {playlist.length === 0 ? (
          <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-12 text-center">
            <ListMusic className="mx-auto h-8 w-8 text-zinc-600 mb-2" />
            <p className="text-sm font-semibold text-zinc-300">Playlist is empty</p>
            <p className="text-xs text-zinc-500 mt-1">Click "Add Effect" to build your custom sequence show.</p>
            <Button
              variant="cyanGlow"
              size="sm"
              onClick={() => setModalOpen(true)}
              className="mt-4 text-xs"
            >
              <Plus className="mr-1.5 h-3.5 w-3.5" />
              Add First Effect
            </Button>
          </div>
        ) : (
          <DndContext
            sensors={sensors}
            collisionDetection={closestCenter}
            onDragEnd={handleDragEnd}
          >
            <SortableContext
              items={playlist.map((i) => i.id)}
              strategy={verticalListSortingStrategy}
            >
              <div className="space-y-2.5">
                {playlist.map((item, index) => (
                  <PlaylistItem
                    key={item.id}
                    item={item}
                    index={index}
                    onUpdate={handleUpdate}
                    onRemove={handleRemove}
                  />
                ))}
              </div>
            </SortableContext>
          </DndContext>
        )}
      </div>

      {/* Add Effect Modal Dialog */}
      <AddEffectModal
        open={modalOpen}
        onOpenChange={setModalOpen}
        onAddEffect={handleAdd}
      />
    </motion.div>
  )
}
