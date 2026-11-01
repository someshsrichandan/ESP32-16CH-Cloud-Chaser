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
import { Badge } from '@/components/ui/badge'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { PlaylistEntry, CommandType } from '@/types/device'
import { toast } from 'sonner'
import {
  Play,
  Square,
  Plus,
  Shuffle,
  Save,
  Trash2,
  FolderOpen,
  ListMusic,
  RotateCcw,
  Sparkles,
  Zap,
  Activity,
  CheckCircle2,
  Layers,
} from 'lucide-react'

interface PresetDoc {
  _id: string
  name: string
  entries: PlaylistEntry[]
  isCustom?: boolean
}

export default function PlaylistPage() {
  const { activeDeviceId } = useActiveDevice()
  const { status, online } = useDeviceStatus(activeDeviceId)
  const { sendCommand, isPending } = useDeviceCommand(activeDeviceId)

  const [presets, setPresets] = React.useState<PresetDoc[]>([])
  const [selectedPresetId, setSelectedPresetId] = React.useState<string>('')
  const [presetName, setPresetName] = React.useState<string>('✨ 5-Cycle Complete Showcase (All 18 FX)')
  const [playlist, setPlaylist] = React.useState<PlaylistEntry[]>([])
  const [modalOpen, setModalOpen] = React.useState(false)
  const [globalCycles, setGlobalCycles] = React.useState<number>(5)
  const [shuffleMode, setShuffleMode] = React.useState(false)
  const [isPlaying, setIsPlaying] = React.useState(false)

  // Fetch presets from MongoDB API
  const fetchPresets = React.useCallback(async () => {
    if (!activeDeviceId) return
    try {
      const res = await fetch(`/api/device/${activeDeviceId}/playlist`)
      if (res.ok) {
        const data = await res.json()
        if (data.playlists && Array.isArray(data.playlists) && data.playlists.length > 0) {
          setPresets(data.playlists)

          // If no preset selected yet, pick the first one (5-Cycle Complete Showcase)
          setSelectedPresetId((prevId) => {
            const exists = data.playlists.some((p: PresetDoc) => p._id === prevId)
            if (prevId && exists) return prevId
            const first = data.playlists[0]
            setPresetName(first.name)
            setPlaylist(first.entries || [])
            return first._id
          })
        }
      }
    } catch (err) {
      console.warn('[Playlist] Error loading presets:', err)
    }
  }, [activeDeviceId])

  React.useEffect(() => {
    fetchPresets()
  }, [fetchPresets])

  // Switch preset when dropdown selection changes
  const handleSelectPreset = (presetId: string) => {
    setSelectedPresetId(presetId)
    const p = presets.find((it) => it._id === presetId)
    if (p) {
      setPresetName(p.name)
      setPlaylist(p.entries || [])
      toast.info(`Loaded Preset: ${p.name}`)
    }
  }

  // Create new blank preset
  const handleNewPreset = () => {
    const newId = `custom_${Date.now()}`
    setSelectedPresetId(newId)
    setPresetName('My Custom Chaser Preset')
    setPlaylist([])
    toast.success('Started new blank preset. Click "+ Add Effect" to build it!')
  }

  // Save current preset to MongoDB
  const handleSavePreset = async () => {
    if (!activeDeviceId) {
      toast.error('No active device selected')
      return
    }

    try {
      const payload = {
        _id: selectedPresetId.startsWith('system_') ? undefined : selectedPresetId,
        name: presetName.trim() || 'Custom Preset',
        entries: playlist,
      }

      const res = await fetch(`/api/device/${activeDeviceId}/playlist`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      })

      if (res.ok) {
        const data = await res.json()
        toast.success(`Preset "${presetName}" saved to Cloud Database!`)
        fetchPresets()
        if (data.playlist?._id) {
          setSelectedPresetId(data.playlist._id)
        }
      } else {
        toast.error('Failed to save preset to database')
      }
    } catch (err: any) {
      toast.error(err.message || 'Error saving preset')
    }
  }

  // Delete custom preset from MongoDB
  const handleDeletePreset = async () => {
    if (!selectedPresetId || selectedPresetId.startsWith('system_')) {
      toast.error('Cannot delete built-in system presets')
      return
    }

    try {
      const res = await fetch(
        `/api/device/${activeDeviceId}/playlist?id=${encodeURIComponent(selectedPresetId)}`,
        { method: 'DELETE' }
      )
      if (res.ok) {
        toast.success('Preset deleted successfully')
        fetchPresets()
      } else {
        toast.error('Failed to delete preset')
      }
    } catch (err: any) {
      toast.error('Error deleting preset')
    }
  }

  // Set all items in current preset to the same cycle count (e.g. 5x)
  const handleApplyGlobalCycles = () => {
    const count = Math.max(1, globalCycles)
    setPlaylist((prev) =>
      prev.map((it) => ({
        ...it,
        cycles: count,
      }))
    )
    toast.success(`Set all ${playlist.length} effects to exactly ${count} cycles!`)
  }

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
    toast.info('Item removed from preset')
  }

  const handleAdd = (entry: Omit<PlaylistEntry, 'id'>) => {
    const newItem: PlaylistEntry = {
      ...entry,
      cycles: globalCycles || 5,
      id: `pl_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`,
    }
    setPlaylist((prev) => [...prev, newItem])
    toast.success(`Added ${entry.effectName} (${newItem.cycles}x cycles)`)
  }

  // 🚀 Primary action: Transmit playlist to ESP32 with zero dead time!
  const handleStartPlaylist = async () => {
    const enabledItems = playlist.filter((it) => it.enabled)
    if (enabledItems.length === 0) {
      toast.error('No enabled effects in preset to run')
      return
    }

    setIsPlaying(true)

    // Build entries payload formatted for ESP32 CycleManager
    const espPlaylistEntries = enabledItems.map((it) => ({
      effectIndex: it.effectIndex,
      cycles: it.cycles,
      speed: it.speedMs,
    }))

    // 1. Send SET_PLAYLIST command with full array
    const ok = await sendCommand('SET_PLAYLIST', {
      playlist: espPlaylistEntries,
    })

    if (ok) {
      // 2. Ensure ESP32 is in auto mode
      await sendCommand('SET_MODE', { mode: 'auto' }, {
        successMsg: `🚀 Preset "${presetName}" Sent to ESP32! Running ${enabledItems.length} effects with Zero Dead Time!`,
      })
    }
  }

  const handleStopPlaylist = async () => {
    setIsPlaying(false)
    await sendCommand('STOP', {}, { successMsg: 'Playlist Execution Stopped' })
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
      {/* Page Header */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 border-b border-zinc-800/80 pb-5">
        <div>
          <div className="flex items-center space-x-3">
            <h1 className="text-2xl font-bold tracking-tight text-zinc-100">
              Preset & Playlist Manager
            </h1>
            <Badge variant="outline" className="font-mono text-xs border-cyan-500/40 text-cyan-300 bg-cyan-950/20">
              Zero-Dead-Time Engine
            </Badge>
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Group effects into custom presets, set exact cycle repeats (e.g. 5x each), and stream directly to ESP32.
          </p>
        </div>

        {/* Live ESP32 Execution Status Badge */}
        {status && (
          <div className="flex items-center space-x-2.5 rounded-xl border border-cyan-500/30 bg-cyan-950/20 px-4 py-2 text-xs font-mono">
            <span className={`h-2.5 w-2.5 rounded-full ${online ? 'bg-emerald-400 shadow-glow-green animate-pulse' : 'bg-zinc-600'}`} />
            <div>
              <div className="flex items-center space-x-2">
                <span className="text-zinc-400">Playing:</span>
                <span className="font-bold text-cyan-300">{status.effect || 'Standby'}</span>
              </div>
              <div className="text-[10px] text-zinc-500">
                Cycle {status.cycle || 1} of {status.totalCycles || 5} • Next: {status.nextEffect || 'Auto'}
              </div>
            </div>
          </div>
        )}
      </div>

      {/* Preset Toolbar & Selector */}
      <Card className="border-zinc-800 bg-[#12121a] p-4 shadow-xl">
        <div className="flex flex-col lg:flex-row items-stretch lg:items-center justify-between gap-4">
          {/* Preset Selector Dropdown & Name Input */}
          <div className="flex flex-1 flex-col sm:flex-row items-stretch sm:items-center gap-3">
            <div className="relative min-w-[240px]">
              <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500 block mb-1">
                Active Preset:
              </span>
              <select
                value={selectedPresetId}
                onChange={(e) => handleSelectPreset(e.target.value)}
                className="w-full rounded-lg border border-zinc-800 bg-zinc-950 px-3 py-2 text-xs font-semibold text-zinc-200 focus:border-cyan-500 focus:outline-none"
              >
                {presets.map((p) => (
                  <option key={p._id} value={p._id}>
                    {p.name} {p.isCustom ? '(Custom)' : ''}
                  </option>
                ))}
              </select>
            </div>

            <div className="flex-1">
              <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500 block mb-1">
                Preset Name (Editable):
              </span>
              <input
                type="text"
                value={presetName}
                onChange={(e) => setPresetName(e.target.value)}
                className="w-full rounded-lg border border-zinc-800 bg-zinc-950/80 px-3 py-2 text-xs font-semibold text-zinc-100 focus:border-cyan-500 focus:outline-none"
                placeholder="Preset Name..."
              />
            </div>
          </div>

          {/* Preset Actions: New, Save, Delete */}
          <div className="flex flex-wrap items-center gap-2 pt-2 lg:pt-4">
            <Button
              variant="outline"
              size="sm"
              onClick={handleNewPreset}
              className="h-8 text-xs text-zinc-300"
              title="Create new blank preset"
            >
              <Plus className="mr-1.5 h-3.5 w-3.5" />
              New Preset
            </Button>

            <Button
              variant="outline"
              size="sm"
              onClick={handleSavePreset}
              className="h-8 text-xs text-cyan-300 border-cyan-500/30 hover:bg-cyan-500/10"
              title="Save preset to cloud database"
            >
              <Save className="mr-1.5 h-3.5 w-3.5" />
              Save Preset
            </Button>

            {!selectedPresetId.startsWith('system_') && selectedPresetId && (
              <Button
                variant="ghost"
                size="sm"
                onClick={handleDeletePreset}
                className="h-8 text-xs text-red-400 hover:text-red-300 hover:bg-red-500/10"
                title="Delete this custom preset"
              >
                <Trash2 className="mr-1 h-3.5 w-3.5" />
                Delete
              </Button>
            )}
          </div>
        </div>

        {/* Global Cycle Setter & Run Controls */}
        <div className="mt-4 pt-4 border-t border-zinc-800/80 flex flex-wrap items-center justify-between gap-3">
          {/* Global Cycle Setter */}
          <div className="flex items-center space-x-2">
            <span className="text-xs font-mono text-zinc-400">Set All Cycles:</span>
            <div className="flex items-center rounded-lg border border-zinc-800 bg-zinc-950 px-2 py-1">
              <RotateCcw className="h-3 w-3 text-cyan-400 mr-1.5" />
              <input
                type="number"
                min={1}
                max={50}
                value={globalCycles}
                onChange={(e) => setGlobalCycles(Math.max(1, parseInt(e.target.value) || 1))}
                className="w-10 bg-transparent text-center font-mono text-xs font-bold text-zinc-100 focus:outline-none"
              />
              <span className="text-[10px] text-zinc-500 mr-1.5">cycles</span>
            </div>
            <Button
              variant="secondary"
              size="sm"
              onClick={handleApplyGlobalCycles}
              className="h-7 text-xs font-mono"
            >
              Apply to All FX
            </Button>
          </div>

          {/* Big Action Buttons: Send to ESP32 / Add Effect */}
          <div className="flex items-center space-x-2">
            <Button
              variant="cyanGlow"
              size="sm"
              onClick={() => setModalOpen(true)}
              className="h-8 text-xs font-semibold"
            >
              <Plus className="mr-1.5 h-3.5 w-3.5" />
              Add Effect
            </Button>

            {isPlaying || status?.mode === 'auto' ? (
              <Button
                variant="destructive"
                size="sm"
                onClick={handleStopPlaylist}
                disabled={isPending}
                className="h-8 text-xs font-semibold shadow-glow-red"
              >
                <Square className="mr-1.5 h-3.5 w-3.5 fill-white" />
                Stop Show
              </Button>
            ) : (
              <Button
                variant="cyanGlow"
                size="sm"
                onClick={handleStartPlaylist}
                disabled={isPending}
                className="h-8 text-xs font-bold shadow-glow-cyan"
              >
                <Play className="mr-1.5 h-3.5 w-3.5 fill-cyan-400" />
                🚀 Send to ESP32 & Run Show
              </Button>
            )}
          </div>
        </div>
      </Card>

      {/* Playlist Overview Statistics */}
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-3">
        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Total Cycle Repeats
          </span>
          <div className="mt-1 flex items-baseline space-x-2">
            <span className="text-2xl font-bold font-mono text-cyan-400">
              {totalRuntimeCycles}
            </span>
            <span className="text-xs text-zinc-400">Cycles across {playlist.filter((it) => it.enabled).length} FX</span>
          </div>
        </div>

        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Transition Timing
          </span>
          <div className="mt-1 flex items-center justify-between">
            <span className="text-sm font-semibold text-emerald-400">
              ⚡ Zero Dead Time
            </span>
            <span className="rounded bg-emerald-500/10 border border-emerald-500/30 px-2 py-0.5 text-[10px] font-mono text-emerald-300">
              0 ms gap
            </span>
          </div>
        </div>

        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-4">
          <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-500">
            Hardware Target
          </span>
          <div className="mt-1 flex items-center justify-between">
            <span className="text-sm font-mono font-bold text-zinc-200">
              {activeDeviceId || 'Auto-Detecting'}
            </span>
            <Badge variant={online ? 'online' : 'offline'} className="text-[9px] uppercase">
              {online ? 'Online' : 'Standby'}
            </Badge>
          </div>
        </div>
      </div>

      {/* Sortable List Area */}
      <div className="space-y-3">
        <div className="flex items-center justify-between px-1 text-xs font-mono text-zinc-400">
          <span>EFFECT SEQUENCE ({playlist.length} ITEMS)</span>
          <span>CYCLES & SPEED CONFIGURATION</span>
        </div>

        {playlist.length === 0 ? (
          <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-12 text-center">
            <ListMusic className="mx-auto h-8 w-8 text-zinc-600 mb-2" />
            <p className="text-sm font-semibold text-zinc-300">Preset is empty</p>
            <p className="text-xs text-zinc-500 mt-1">Click "Add Effect" to select from the 18 Arduino chase patterns.</p>
            <Button
              variant="cyanGlow"
              size="sm"
              onClick={() => setModalOpen(true)}
              className="mt-4 text-xs font-semibold"
            >
              <Plus className="mr-1.5 h-3.5 w-3.5" />
              Add First Pattern
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
