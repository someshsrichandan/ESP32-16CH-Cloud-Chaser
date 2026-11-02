'use client'

import * as React from 'react'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { DeviceStatus } from '@/types/device'
import { Wifi, Cpu, Clock, Thermometer, Radio, Globe } from 'lucide-react'
import { formatUptime, formatRssi, formatBytes } from '@/lib/utils'

interface ConnectionStatusCardProps {
  status: DeviceStatus | null
  online: boolean
  lastUpdate: number | null
}

export function ConnectionStatusCard({
  status,
  online,
  lastUpdate,
}: ConnectionStatusCardProps) {
  const rssiInfo = formatRssi(status?.rssi || 0)

  return (
    <Card className="relative overflow-hidden border-zinc-800 bg-gradient-to-br from-[#12121a] to-[#0d0d16]">
      {/* Decorative accent line */}
      <div
        className={`absolute top-0 left-0 right-0 h-[2px] transition-colors duration-300 ${
          online ? 'bg-gradient-to-r from-cyan-500 to-emerald-400' : 'bg-zinc-700'
        }`}
      />

      <CardHeader className="flex flex-row items-center justify-between pb-3">
        <div className="flex items-center space-x-2.5">
          <div className="flex h-8 w-8 items-center justify-center rounded-lg bg-zinc-800/80 border border-zinc-700 text-zinc-300">
            <Radio className="h-4 w-4" />
          </div>
          <div>
            <CardTitle className="text-sm font-semibold tracking-wide">
              {status?.deviceId || 'ESP32 Relay Chaser'}
            </CardTitle>
            <p className="text-[11px] text-zinc-400 font-mono">
              IP: {status?.ip || '192.168.4.1'}
            </p>
          </div>
        </div>

        <Badge
          variant={online ? 'online' : 'offline'}
          className="uppercase tracking-wider font-mono text-[10px] px-2.5 py-0.5"
        >
          {online ? 'Online (HTTP)' : 'Offline'}
        </Badge>
      </CardHeader>

      <CardContent className="grid grid-cols-2 gap-3 pt-1 sm:grid-cols-4">
        {/* Signal RSSI */}
        <div className="rounded-lg border border-zinc-800/80 bg-zinc-950/40 p-2.5">
          <div className="flex items-center space-x-1.5 text-zinc-400">
            <Wifi className="h-3.5 w-3.5 text-cyan-400" />
            <span className="text-[10px] uppercase font-mono tracking-wider">Wi-Fi RSSI</span>
          </div>
          <div className="mt-1 flex items-baseline space-x-2">
            <span className="text-sm font-bold font-mono text-zinc-100">
              {status?.rssi ? `${status.rssi} dBm` : '-'}
            </span>
            <div className="flex space-x-0.5">
              {[1, 2, 3, 4].map((bar) => (
                <div
                  key={bar}
                  className={`w-1 rounded-sm transition-all ${
                    bar <= rssiInfo.bars
                      ? 'bg-cyan-400 shadow-glow-cyan-sm'
                      : 'bg-zinc-700'
                  }`}
                  style={{ height: `${bar * 3.5 + 4}px` }}
                />
              ))}
            </div>
          </div>
        </div>

        {/* Uptime */}
        <div className="rounded-lg border border-zinc-800/80 bg-zinc-950/40 p-2.5">
          <div className="flex items-center space-x-1.5 text-zinc-400">
            <Clock className="h-3.5 w-3.5 text-cyan-400" />
            <span className="text-[10px] uppercase font-mono tracking-wider">System Uptime</span>
          </div>
          <div className="mt-1">
            <span className="text-sm font-bold font-mono text-zinc-100">
              {formatUptime(status?.uptime || 0)}
            </span>
          </div>
        </div>

        {/* Free Heap */}
        <div className="rounded-lg border border-zinc-800/80 bg-zinc-950/40 p-2.5">
          <div className="flex items-center space-x-1.5 text-zinc-400">
            <Cpu className="h-3.5 w-3.5 text-cyan-400" />
            <span className="text-[10px] uppercase font-mono tracking-wider">Free Heap</span>
          </div>
          <div className="mt-1">
            <span className="text-sm font-bold font-mono text-zinc-100">
              {formatBytes(status?.heap || 180000)}
            </span>
          </div>
        </div>

        {/* Core Temperature */}
        <div className="rounded-lg border border-zinc-800/80 bg-zinc-950/40 p-2.5">
          <div className="flex items-center space-x-1.5 text-zinc-400">
            <Thermometer className="h-3.5 w-3.5 text-amber-400" />
            <span className="text-[10px] uppercase font-mono tracking-wider">Core Temp</span>
          </div>
          <div className="mt-1">
            <span className="text-sm font-bold font-mono text-zinc-100">
              {status?.temp ? `${status.temp.toFixed(1)} °C` : '42.5 °C'}
            </span>
          </div>
        </div>
      </CardContent>
    </Card>
  )
}
