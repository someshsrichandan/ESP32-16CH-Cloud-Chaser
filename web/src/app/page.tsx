'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { ConnectionStatusCard } from '@/components/dashboard/connection-status'
import { RelayMiniGrid } from '@/components/dashboard/relay-mini-grid'
import { CycleProgress } from '@/components/dashboard/cycle-progress'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { SpeedSlider } from '@/components/ui/speed-slider'
import { ModeBadge } from '@/components/ui/mode-badge'
import { CommandType } from '@/types/device'
import {
  Power,
  PowerOff,
  AlertOctagon,
  Play,
  Pause,
  SkipForward,
  SkipBack,
  Sliders,
  Shuffle,
  RefreshCw,
  Gauge,
  Activity,
  Layers,
} from 'lucide-react'

export default function DashboardPage() {
  const deviceId = 'esp32_relay_01'
  const { status, online, lastUpdate, refresh } = useDeviceStatus(deviceId)
  const { sendCommand, isPending } = useDeviceCommand(deviceId)

  const handleAction = async (action: string, successMsg?: string) => {
    await sendCommand(CommandType.ACTION, { a: action }, { successMsg })
  }

  const handleSpeedChange = async (speed: number) => {
    await sendCommand(CommandType.SPEED, { v: speed }, { silent: true })
  }

  const handleRepeatsChange = async (reps: number) => {
    await sendCommand(CommandType.REPEATS, { v: reps }, { successMsg: `Repeats set to ${reps}x` })
  }

  const currentMode = status?.mode || 'auto'
  const currentSpeed = status?.speed || 120
  const relays = status?.relays || new Array(16).fill(false)

  return (
    <motion.div
      initial={{ opacity: 0, y: 12 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.3 }}
      className="space-y-6"
    >
      {/* Top Banner / Quick Header */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 border-b border-zinc-800/80 pb-5">
        <div>
          <div className="flex items-center space-x-3">
            <h1 className="text-2xl font-bold tracking-tight text-zinc-100">
              Command Dashboard
            </h1>
            <ModeBadge mode={currentMode} />
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Real-time telemetry, master controls, and 16-channel relay state matrix.
          </p>
        </div>

        <div className="flex items-center space-x-2">
          <Button
            variant="outline"
            size="sm"
            onClick={() => refresh()}
            className="text-xs text-zinc-400 hover:text-zinc-200"
          >
            <RefreshCw className="mr-1.5 h-3.5 w-3.5" />
            Sync Now
          </Button>

          <Button
            variant="destructive"
            size="sm"
            disabled={isPending}
            onClick={() => handleAction('all_off', 'EMERGENCY STOP EXECUTED')}
            className="text-xs font-semibold shadow-glow-red"
          >
            <AlertOctagon className="mr-1.5 h-3.5 w-3.5" />
            EMERGENCY STOP
          </Button>
        </div>
      </div>

      {/* Primary Status Card */}
      <ConnectionStatusCard status={status} online={online} lastUpdate={lastUpdate} />

      {/* Quick Action Control Bar */}
      <Card className="border-zinc-800 bg-[#12121a] p-4 shadow-xl">
        <div className="flex flex-wrap items-center justify-between gap-3">
          <div className="flex flex-wrap items-center gap-2">
            <span className="text-[11px] font-mono uppercase tracking-wider text-zinc-500 mr-1">
              Sequence Controls:
            </span>

            {/* Play/Pause Chaser */}
            <Button
              variant="cyanGlow"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('toggle_run', 'Sequence Toggled')}
              className="text-xs font-semibold"
            >
              {currentMode === 'stopped' ? (
                <>
                  <Play className="mr-1.5 h-3.5 w-3.5 fill-cyan-400" />
                  Resume Run
                </>
              ) : (
                <>
                  <Pause className="mr-1.5 h-3.5 w-3.5" />
                  Pause Run
                </>
              )}
            </Button>

            {/* Auto Mode Toggle */}
            <Button
              variant={currentMode === 'auto' ? 'secondary' : 'outline'}
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('toggle_auto', 'Chaser Mode Switched')}
              className="text-xs font-mono"
            >
              <RotateCw className="mr-1.5 h-3 w-3 text-cyan-400" />
              {currentMode === 'auto' ? 'Auto Cycling (Active)' : 'Single FX Loop'}
            </Button>

            {/* Prev / Next Effect */}
            <Button
              variant="outline"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('prev', 'Previous Effect')}
              className="h-8 text-xs px-2.5 text-zinc-300"
              title="Previous Effect"
            >
              <SkipBack className="h-3.5 w-3.5" />
            </Button>

            <Button
              variant="outline"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('next', 'Advanced to Next Effect')}
              className="h-8 text-xs px-2.5 text-zinc-300"
              title="Next Effect"
            >
              <SkipForward className="h-3.5 w-3.5" />
            </Button>

            {/* Shuffle */}
            <Button
              variant="outline"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('toggle_shuffle', 'Shuffle Toggled')}
              className="h-8 text-xs px-2.5 text-zinc-300"
              title="Toggle Random Shuffle"
            >
              <Shuffle className="h-3.5 w-3.5 text-cyan-400" />
            </Button>
          </div>

          {/* Master Relay State Triggers */}
          <div className="flex items-center space-x-2">
            <Button
              variant="outline"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('all_on', 'Master Energized: ALL ON')}
              className="h-8 text-xs border-cyan-500/30 text-cyan-300 hover:bg-cyan-500/10"
            >
              <Power className="mr-1.5 h-3.5 w-3.5" />
              Master ON
            </Button>

            <Button
              variant="outline"
              size="sm"
              disabled={isPending}
              onClick={() => handleAction('all_off', 'Master De-energized: ALL OFF')}
              className="h-8 text-xs text-zinc-300 hover:text-red-400"
            >
              <PowerOff className="mr-1.5 h-3.5 w-3.5" />
              Master OFF
            </Button>
          </div>
        </div>
      </Card>

      {/* Main Grid Section: Cycle Progress & Mini Relay Grid */}
      <div className="grid grid-cols-1 gap-6 lg:grid-cols-2">
        <CycleProgress status={status} />
        <RelayMiniGrid relays={relays} />
      </div>

      {/* Speed Slider & Telemetry Section */}
      <div className="grid grid-cols-1 gap-6 lg:grid-cols-3">
        {/* Speed Adjustment Card */}
        <div className="lg:col-span-2">
          <Card className="border-zinc-800 bg-[#12121a] h-full flex flex-col justify-between">
            <CardHeader className="pb-3">
              <div className="flex items-center space-x-2">
                <Gauge className="h-4 w-4 text-cyan-400" />
                <CardTitle className="text-sm font-semibold">Live Timing & Velocity</CardTitle>
              </div>
              <p className="text-xs text-zinc-400">
                Adjust step pulse width on the ESP32 hardware timer in real-time.
              </p>
            </CardHeader>
            <CardContent>
              <SpeedSlider
                value={currentSpeed}
                onChange={handleSpeedChange}
                disabled={isPending}
              />
            </CardContent>
          </Card>
        </div>

        {/* System Health / Telemetry Card */}
        <div>
          <Card className="border-zinc-800 bg-[#12121a] h-full">
            <CardHeader className="pb-3">
              <div className="flex items-center space-x-2">
                <Activity className="h-4 w-4 text-cyan-400" />
                <CardTitle className="text-sm font-semibold">ESP32 Hardware Health</CardTitle>
              </div>
              <p className="text-xs text-zinc-400">Low-level controller diagnostic snapshot</p>
            </CardHeader>

            <CardContent className="space-y-3 font-mono text-xs">
              <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
                <span className="text-zinc-400">Free Heap:</span>
                <span className="font-bold text-cyan-400">
                  {status?.heap ? `${(status.heap / 1024).toFixed(1)} KB` : '182.4 KB'}
                </span>
              </div>

              <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
                <span className="text-zinc-400">Min Ever Heap:</span>
                <span className="font-bold text-zinc-200">
                  {status?.minHeap ? `${(status.minHeap / 1024).toFixed(1)} KB` : '158.0 KB'}
                </span>
              </div>

              <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
                <span className="text-zinc-400">Active Repeats:</span>
                <span className="font-bold text-amber-400">
                  {status?.totalCycles || 3}x cycles / FX
                </span>
              </div>

              <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
                <span className="text-zinc-400">Cloud Sync:</span>
                <span className="font-bold text-emerald-400">
                  {online ? 'HTTP + MongoDB Live' : 'HTTP Standby'}
                </span>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </motion.div>
  )
}

function RotateCw(props: any) {
  return (
    <svg
      {...props}
      xmlns="http://www.w3.org/2000/svg"
      width="24"
      height="24"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="2"
      strokeLinecap="round"
      strokeLinejoin="round"
    >
      <path d="M21 12a9 9 0 1 1-9-9c2.52 0 4.93 1 6.74 2.74L21 8" />
      <path d="M21 3v5h-5" />
    </svg>
  )
}
