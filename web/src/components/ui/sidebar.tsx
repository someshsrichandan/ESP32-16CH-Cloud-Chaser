'use client'

import * as React from 'react'
import { useState } from 'react'
import Link from 'next/link'
import { usePathname } from 'next/navigation'
import {
  LayoutDashboard,
  ToggleLeft,
  Sparkles,
  ListMusic,
  Settings,
  Activity,
  Zap,
  Menu,
  X,
  Radio,
  LogOut,
} from 'lucide-react'
import { cn } from '@/lib/utils'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useActiveDevice } from '@/context/device-context'
import { signOut, useSession } from 'next-auth/react'

const NAV_ITEMS = [
  {
    name: 'Dashboard',
    href: '/',
    icon: LayoutDashboard,
  },
  {
    name: 'Manual Control',
    href: '/manual',
    icon: ToggleLeft,
  },
  {
    name: 'Effect Library',
    href: '/effects',
    icon: Sparkles,
  },
  {
    name: 'Playlist Manager',
    href: '/playlist',
    icon: ListMusic,
  },
  {
    name: 'Settings',
    href: '/settings',
    icon: Settings,
  },
  {
    name: 'Diagnostics',
    href: '/diagnostics',
    icon: Activity,
  },
]

export function Sidebar() {
  const pathname = usePathname()
  const [mobileOpen, setMobileOpen] = useState(false)
  const { activeDeviceId, activeDevice } = useActiveDevice()
  const { online, isConnected } = useDeviceStatus(activeDeviceId)
  const { data: session } = useSession()

  return (
    <>
      {/* Mobile Top Bar */}
      <div className="flex h-16 items-center justify-between border-b border-zinc-800 bg-[#0c0c14] px-4 md:hidden">
        <div className="flex items-center space-x-2.5">
          <div className="flex h-8 w-8 items-center justify-center rounded-lg bg-cyan-500/20 border border-cyan-500/40 text-cyan-400 shadow-glow-cyan-sm">
            <Zap className="h-4 w-4" />
          </div>
          <span className="font-bold tracking-tight text-zinc-100">ESP32 Chaser</span>
        </div>
        <Button
          variant="ghost"
          size="icon"
          onClick={() => setMobileOpen(!mobileOpen)}
          className="text-zinc-400 hover:text-zinc-100"
        >
          {mobileOpen ? <X className="h-5 w-5" /> : <Menu className="h-5 w-5" />}
        </Button>
      </div>

      {/* Backdrop for Mobile */}
      {mobileOpen && (
        <div
          className="fixed inset-0 z-40 bg-black/70 backdrop-blur-sm md:hidden"
          onClick={() => setMobileOpen(false)}
        />
      )}

      {/* Sidebar Container */}
      <aside
        className={cn(
          'fixed inset-y-0 left-0 z-50 flex w-64 flex-col border-r border-zinc-800/80 bg-[#0a0a0f] transition-transform duration-200 ease-in-out md:static md:translate-x-0',
          mobileOpen ? 'translate-x-0' : '-translate-x-full'
        )}
      >
        {/* Brand Header */}
        <div className="flex h-20 items-center justify-between border-b border-zinc-800/80 px-6">
          <div className="flex items-center space-x-3">
            <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-gradient-to-br from-cyan-500/20 to-blue-500/10 border border-cyan-500/40 text-cyan-400 shadow-glow-cyan-sm">
              <Zap className="h-5 w-5" />
            </div>
            <div>
              <div className="flex items-center space-x-1.5">
                <span className="font-bold text-sm tracking-wide text-zinc-100">
                  RELAY CHASER
                </span>
                <span className="rounded bg-cyan-500/20 px-1 py-0.2 text-[9px] font-mono font-bold text-cyan-300">
                  PRO
                </span>
              </div>
              <p className="text-[11px] text-zinc-500 font-mono">16-CH ESP32 Core</p>
            </div>
          </div>
        </div>

        {/* Device Status Pill in Sidebar */}
        <div className="p-4 border-b border-zinc-800/60 bg-zinc-950/40">
          <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-900/60 p-2.5">
            <div className="flex items-center space-x-2">
              <span className="relative flex h-2.5 w-2.5">
                {online ? (
                  <>
                    <span className="absolute inline-flex h-full w-full animate-ping rounded-full bg-emerald-400 opacity-75" />
                    <span className="relative inline-flex h-2.5 w-2.5 rounded-full bg-emerald-400 shadow-glow-green" />
                  </>
                ) : (
                  <span className="relative inline-flex h-2.5 w-2.5 rounded-full bg-zinc-600" />
                )}
              </span>
              <div className="flex flex-col truncate pr-2">
                <span className="text-xs font-medium text-zinc-200 truncate">
                  {activeDevice?.deviceName || activeDeviceId || 'ESP-32 Core'}
                </span>
                <span className="text-[10px] font-mono text-zinc-500">
                  {online ? (activeDevice?.ip ? `${activeDevice.ip} (Online)` : 'Online (HTTP)') : 'Offline / Waiting'}
                </span>
              </div>
            </div>

            <Badge
              variant={online ? 'online' : 'offline'}
              className="px-2 py-0.5 text-[9px] uppercase tracking-wider"
            >
              {online ? 'Active' : 'Offline'}
            </Badge>
          </div>
        </div>

        {/* Navigation Menu */}
        <nav className="flex-1 space-y-1 overflow-y-auto px-3 py-4">
          {NAV_ITEMS.map((item) => {
            const isActive = pathname === item.href
            return (
              <Link
                key={item.href}
                href={item.href}
                onClick={() => setMobileOpen(false)}
                className={cn(
                  'group flex items-center rounded-lg px-3 py-2.5 text-xs font-medium transition-all duration-150',
                  isActive
                    ? 'bg-cyan-500/15 text-cyan-300 border border-cyan-500/30 shadow-glow-cyan-sm font-semibold'
                    : 'text-zinc-400 hover:bg-zinc-800/60 hover:text-zinc-200'
                )}
              >
                <item.icon
                  className={cn(
                    'mr-3 h-4 w-4 transition-colors',
                    isActive ? 'text-cyan-400' : 'text-zinc-500 group-hover:text-zinc-300'
                  )}
                />
                <span>{item.name}</span>
                {isActive && (
                  <span className="ml-auto h-1.5 w-1.5 rounded-full bg-cyan-400 shadow-glow-cyan-sm" />
                )}
              </Link>
            )
          })}
        </nav>

        {/* Footer / Operator Area */}
        <div className="border-t border-zinc-800/80 p-4 bg-zinc-950/60">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-2.5">
              <div className="flex h-7 w-7 items-center justify-center rounded-full bg-zinc-800 border border-zinc-700 text-zinc-300 text-xs font-mono">
                {session?.user?.name ? session.user.name.charAt(0) : 'O'}
              </div>
              <div className="overflow-hidden">
                <p className="truncate text-xs font-medium text-zinc-200">
                  {session?.user?.name || 'Operator'}
                </p>
                <p className="truncate text-[10px] text-zinc-500 font-mono">
                  {session ? 'Authenticated' : 'Guest view'}
                </p>
              </div>
            </div>

            {session ? (
              <Button
                variant="ghost"
                size="iconSm"
                onClick={() => signOut({ callbackUrl: '/login' })}
                title="Sign out"
                className="text-zinc-400 hover:text-red-400"
              >
                <LogOut className="h-3.5 w-3.5" />
              </Button>
            ) : (
              <Link href="/login">
                <Button variant="outline" size="sm" className="h-7 text-[11px] px-2.5">
                  Sign In
                </Button>
              </Link>
            )}
          </div>
        </div>
      </aside>
    </>
  )
}

