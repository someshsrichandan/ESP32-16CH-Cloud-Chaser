'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { EffectCard } from '@/components/effects/effect-card'
import { EFFECTS_DATA, EFFECT_CATEGORIES } from '@/lib/effects-data'
import { CommandType } from '@/types/device'
import { Input } from '@/components/ui/input'
import { Button } from '@/components/ui/button'
import { Search, Sparkles, Filter, CheckCircle2 } from 'lucide-react'
import { cn } from '@/lib/utils'

export default function EffectsPage() {
  const { activeDeviceId } = useActiveDevice()
  const { status } = useDeviceStatus(activeDeviceId)
  const { sendCommand, isPending } = useDeviceCommand(activeDeviceId)

  const [search, setSearch] = React.useState('')
  const [activeCategory, setActiveCategory] = React.useState('all')

  const activeEffectIndex = status?.effectIndex ?? -1

  const filteredEffects = React.useMemo(() => {
    return EFFECTS_DATA.filter((fx) => {
      const query = search.toLowerCase()
      const matchesSearch =
        fx.name.toLowerCase().includes(query) ||
        fx.description.toLowerCase().includes(query) ||
        fx.tags.some((t) => t.toLowerCase().includes(query))
      const matchesCategory =
        activeCategory === 'all' || fx.category === activeCategory
      return matchesSearch && matchesCategory
    })
  }, [search, activeCategory])

  const handleSelectEffect = async (effectId: number, speed?: number) => {
    // Dispatch effect switch to ESP32
    await sendCommand(
      CommandType.EFFECT,
      { id: effectId },
      { successMsg: `Triggered Effect #${effectId}: ${EFFECTS_DATA[effectId]?.name || ''}` }
    )

    if (speed) {
      await sendCommand(CommandType.SPEED, { v: speed }, { silent: true })
    }
  }

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
              Pattern & Effect Library
            </h1>
            <span className="rounded-full bg-cyan-500/20 px-2.5 py-0.5 text-xs font-mono font-bold text-cyan-300 border border-cyan-500/40">
              18 Native Arduino Chase Patterns
            </span>
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Browse and trigger dynamic lighting chasers with live 16-channel CSS wave simulation.
          </p>
        </div>

        {status && (
          <div className="flex items-center space-x-2 rounded-lg border border-cyan-500/30 bg-cyan-950/20 px-3 py-1.5 text-xs font-mono">
            <CheckCircle2 className="h-4 w-4 text-cyan-400" />
            <span className="text-zinc-400">Current FX:</span>
            <span className="font-bold text-cyan-300">{status.effect}</span>
          </div>
        )}
      </div>

      {/* Search and Category Filters */}
      <div className="flex flex-col md:flex-row items-stretch md:items-center justify-between gap-3 rounded-xl border border-zinc-800 bg-[#12121a] p-3.5 shadow-md">
        {/* Search */}
        <div className="relative flex-1">
          <Search className="absolute left-3 top-2.5 h-4 w-4 text-zinc-500" />
          <Input
            type="text"
            placeholder="Search 18 patterns by title, category, or style..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="pl-9 bg-zinc-950/70 border-zinc-800 text-xs"
          />
        </div>

        {/* Categories */}
        <div className="flex flex-wrap items-center gap-1.5 overflow-x-auto pb-1 md:pb-0">
          {EFFECT_CATEGORIES.map((cat) => {
            const isCatActive = activeCategory === cat.id
            return (
              <Button
                key={cat.id}
                type="button"
                variant={isCatActive ? 'cyanGlow' : 'secondary'}
                size="sm"
                onClick={() => setActiveCategory(cat.id)}
                className={cn('h-8 text-xs font-medium', isCatActive && 'font-bold')}
              >
                {cat.label}
              </Button>
            )
          })}
        </div>
      </div>

      {/* Grid of 30 Effect Cards */}
      {filteredEffects.length === 0 ? (
        <div className="rounded-xl border border-zinc-800 bg-[#12121a] p-12 text-center">
          <Sparkles className="mx-auto h-8 w-8 text-zinc-600 mb-2" />
          <p className="text-sm font-semibold text-zinc-300">No matching effects found</p>
          <p className="text-xs text-zinc-500 mt-1">Try clearing your search query or selecting a different category.</p>
          <Button
            variant="outline"
            size="sm"
            onClick={() => {
              setSearch('')
              setActiveCategory('all')
            }}
            className="mt-4 text-xs"
          >
            Reset Filters
          </Button>
        </div>
      ) : (
        <div className="grid grid-cols-1 gap-5 sm:grid-cols-2 lg:grid-cols-3">
          {filteredEffects.map((effect) => (
            <EffectCard
              key={effect.id}
              effect={effect}
              isActive={activeEffectIndex === effect.id}
              onSelect={handleSelectEffect}
              disabled={isPending}
            />
          ))}
        </div>
      )}
    </motion.div>
  )
}
