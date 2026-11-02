'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { RelayGrid } from '@/components/manual/relay-grid'
import { ModeBadge } from '@/components/ui/mode-badge'
import { Badge } from '@/components/ui/badge'
import { Sliders, ShieldCheck } from 'lucide-react'

export default function ManualControlPage() {
  const { activeDeviceId } = useActiveDevice()
  const { status, online } = useDeviceStatus(activeDeviceId)

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
              Manual Relay Matrix
            </h1>
            <ModeBadge mode={status?.mode || 'manual'} />
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Direct real-time hardware override. Toggling any relay automatically halts autonomous sequence chaser.
          </p>
        </div>

        <div className="flex items-center space-x-2">
          <Badge
            variant={online ? 'online' : 'offline'}
            className="uppercase tracking-wider font-mono text-xs px-3 py-1"
          >
            <ShieldCheck className="mr-1.5 h-3.5 w-3.5" />
            {online ? 'Cloud Link Live' : 'Offline'}
          </Badge>
        </div>
      </div>

      {/* 4x4 Interactive Matrix Grid */}
      <RelayGrid
        deviceId={activeDeviceId}
        incomingRelays={status?.relays}
        disabled={false}
      />
    </motion.div>
  )
}
