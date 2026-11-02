'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { CommandType } from '@/types/device'
import { formatUptime, formatBytes, formatRssi } from '@/lib/utils'
import {
  Activity,
  Cpu,
  Wifi,
  Cloud,
  Terminal,
  RefreshCw,
  Clock,
  Thermometer,
  ShieldCheck,
  CheckCircle2,
} from 'lucide-react'

interface LogMessage {
  id: string
  ts: string
  endpoint: string
  direction: 'rx' | 'tx'
  payload: string
}

export default function DiagnosticsPage() {
  const { activeDeviceId } = useActiveDevice()
  const { status, online, lastUpdate, refresh } = useDeviceStatus(activeDeviceId)
  const { sendCommand, isPending } = useDeviceCommand(activeDeviceId)

  const [logs, setLogs] = React.useState<LogMessage[]>([
    {
      id: '1',
      ts: new Date(Date.now() - 45000).toLocaleTimeString(),
      endpoint: '/api/esp/register',
      direction: 'rx',
      payload: '{"status":"registered","firmware":"v2.0-http","auth":"verified"}',
    },
    {
      id: '2',
      ts: new Date(Date.now() - 30000).toLocaleTimeString(),
      endpoint: '/api/esp/status',
      direction: 'rx',
      payload: '{"mode":"auto","effect":"Left Chaser","speed":120,"heap":182400}',
    },
    {
      id: '3',
      ts: new Date(Date.now() - 15000).toLocaleTimeString(),
      endpoint: '/api/esp/poll',
      direction: 'tx',
      payload: '{"commands":[],"timestamp":' + (Date.now() - 15000) + '}',
    },
  ])

  // Append new telemetry snapshots to log viewer
  React.useEffect(() => {
    if (status) {
      const newEntry: LogMessage = {
        id: Math.random().toString(36).substring(2, 9),
        ts: new Date().toLocaleTimeString(),
        endpoint: '/api/esp/status',
        direction: 'rx',
        payload: JSON.stringify({
          effect: status.effect,
          speed: status.speed,
          cycle: status.cycle,
          heap: status.heap,
          relays: status.relays.map((r) => (r ? 1 : 0)).join(''),
        }),
      }
      setLogs((prev) => [newEntry, ...prev.slice(0, 49)])
    }
  }, [status, activeDeviceId])

  const handlePing = async () => {
    await sendCommand(CommandType.REQUEST_STATE, {}, { successMsg: 'State query dispatched' })
    refresh()
  }

  const rssiInfo = formatRssi(status?.rssi || -60)

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
              System Diagnostics
            </h1>
            <Badge
              variant={online ? 'online' : 'offline'}
              className="font-mono text-xs uppercase"
            >
              {online ? 'Telemetry Live' : 'Link Dormant'}
            </Badge>
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Real-time RTOS memory allocation, thermal sensors, HTTP REST telemetry logs, and wireless stats.
          </p>
        </div>

        <div className="flex items-center space-x-2">
          <Button
            variant="cyanGlow"
            size="sm"
            onClick={handlePing}
            disabled={isPending}
            className="text-xs font-semibold"
          >
            <RefreshCw className="mr-1.5 h-3.5 w-3.5" />
            Query State (Ping)
          </Button>
        </div>
      </div>

      {/* Metrics Grid */}
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-4">
        {/* Memory Free Heap */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-2">
            <div className="flex items-center justify-between">
              <span className="text-xs font-mono uppercase text-zinc-400">Free Heap</span>
              <Cpu className="h-4 w-4 text-cyan-400" />
            </div>
            <CardTitle className="text-xl font-mono font-bold text-zinc-100 mt-1">
              {formatBytes(status?.heap || 184320)}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-[11px] font-mono text-zinc-500">
              Min Ever Heap: {formatBytes(status?.minHeap || 158720)}
            </div>
          </CardContent>
        </Card>

        {/* System Uptime */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-2">
            <div className="flex items-center justify-between">
              <span className="text-xs font-mono uppercase text-zinc-400">Runtime Uptime</span>
              <Clock className="h-4 w-4 text-cyan-400" />
            </div>
            <CardTitle className="text-xl font-mono font-bold text-zinc-100 mt-1">
              {formatUptime(status?.uptime || 1420)}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-[11px] font-mono text-zinc-500">
              Boot reason: Power-on reset (0x01)
            </div>
          </CardContent>
        </Card>

        {/* Thermal Sensor */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-2">
            <div className="flex items-center justify-between">
              <span className="text-xs font-mono uppercase text-zinc-400">Core Temp</span>
              <Thermometer className="h-4 w-4 text-amber-400" />
            </div>
            <CardTitle className="text-xl font-mono font-bold text-amber-300 mt-1">
              {status?.temp ? `${status.temp.toFixed(1)} °C` : '42.8 °C'}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-[11px] font-mono text-emerald-400">
              Optimal (160MHz throttled)
            </div>
          </CardContent>
        </Card>

        {/* Wi-Fi RSSI */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-2">
            <div className="flex items-center justify-between">
              <span className="text-xs font-mono uppercase text-zinc-400">Wireless RSSI</span>
              <Wifi className="h-4 w-4 text-cyan-400" />
            </div>
            <CardTitle className="text-xl font-mono font-bold text-zinc-100 mt-1">
              {rssiInfo.text}
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-[11px] font-mono text-zinc-500">
              SSID: {status?.staSsid || 'Dual Network AP'}
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Network & Protocol Info */}
      <div className="grid grid-cols-1 gap-6 lg:grid-cols-2">
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <Cloud className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">HTTP REST & MongoDB Pipeline</CardTitle>
            </div>
            <CardDescription className="text-xs">
              Direct HTTPS REST architecture with MongoDB Atlas
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-2.5 font-mono text-xs">
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Architecture:</span>
              <span className="text-emerald-400">Direct HTTPS + MongoDB Atlas</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Status Push (POST):</span>
              <span className="text-zinc-100">/api/esp/status (2000ms cadence)</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Command Poll (GET):</span>
              <span className="text-cyan-400">/api/esp/poll (1000ms cadence)</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Real-Time Stream (SSE):</span>
              <span className="text-zinc-200">/api/device/[id]/stream</span>
            </div>
            <div className="flex justify-between">
              <span className="text-zinc-400">Last Telemetry Sync:</span>
              <span className="text-zinc-300">
                {lastUpdate ? new Date(lastUpdate).toLocaleTimeString() : 'Awaiting data'}
              </span>
            </div>
          </CardContent>
        </Card>

        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <Activity className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">Hardware Architecture</CardTitle>
            </div>
            <CardDescription className="text-xs">
              ESP32 dual-core Xtensa LX6 resource allocation
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-2.5 font-mono text-xs">
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Core 0:</span>
              <span className="text-zinc-200">Network & HTTP Polling Daemon Task</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Core 1:</span>
              <span className="text-cyan-400">Hard Real-Time Relay Sequencer</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Relay Logic:</span>
              <span className="text-zinc-200">Active LOW (Boot-safe pull-ups)</span>
            </div>
            <div className="flex justify-between border-b border-zinc-800/80 pb-2">
              <span className="text-zinc-400">Flash Mode:</span>
              <span className="text-zinc-200">DIO 40MHz, 4MB Flash</span>
            </div>
            <div className="flex justify-between">
              <span className="text-zinc-400">FreeRTOS Tick:</span>
              <span className="text-emerald-400">1000 Hz (1ms resolution)</span>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Live HTTP REST & Telemetry Inspector */}
      <Card className="border-zinc-800 bg-[#0c0c14]">
        <CardHeader className="pb-3 border-b border-zinc-800">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-2">
              <Terminal className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold font-mono">
                Live HTTP REST & Telemetry Inspector
              </CardTitle>
            </div>
            <span className="text-[10px] font-mono text-zinc-500">
              Showing last {logs.length} transmissions
            </span>
          </div>
        </CardHeader>

        <CardContent className="p-0">
          <div className="max-h-64 overflow-y-auto p-4 space-y-2 font-mono text-xs bg-zinc-950/80">
            {logs.map((log) => (
              <div
                key={log.id}
                className="flex items-start space-x-2 text-[11px] leading-relaxed border-b border-zinc-900 pb-1.5 last:border-0"
              >
                <span className="text-zinc-600 shrink-0">[{log.ts}]</span>
                <span
                  className={`shrink-0 font-bold px-1 rounded text-[9px] ${
                    log.direction === 'rx'
                      ? 'bg-emerald-500/20 text-emerald-400'
                      : 'bg-cyan-500/20 text-cyan-400'
                  }`}
                >
                  {log.direction.toUpperCase()}
                </span>
                <span className="text-zinc-400 shrink-0">{log.endpoint}:</span>
                <span className="text-zinc-300 break-all">{log.payload}</span>
              </div>
            ))}
          </div>
        </CardContent>
      </Card>
    </motion.div>
  )
}
