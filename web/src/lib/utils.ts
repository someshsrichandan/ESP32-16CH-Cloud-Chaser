import { clsx, type ClassValue } from 'clsx'
import { twMerge } from 'tailwind-merge'

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export function formatUptime(seconds: number): string {
  if (!seconds || seconds <= 0) return '0s'
  const d = Math.floor(seconds / (3600 * 24))
  const h = Math.floor((seconds % (3600 * 24)) / 3600)
  const m = Math.floor((seconds % 3600) / 60)
  const s = Math.floor(seconds % 60)

  const parts: string[] = []
  if (d > 0) parts.push(`${d}d`)
  if (h > 0) parts.push(`${h}h`)
  if (m > 0) parts.push(`${m}m`)
  if (s > 0 || parts.length === 0) parts.push(`${s}s`)

  return parts.join(' ')
}

export function formatRssi(rssi: number): { text: string; quality: 'excellent' | 'good' | 'fair' | 'poor' | 'none'; bars: number } {
  if (!rssi || rssi === 0) return { text: 'No Signal', quality: 'none', bars: 0 }
  if (rssi >= -50) return { text: `${rssi} dBm (Excellent)`, quality: 'excellent', bars: 4 }
  if (rssi >= -65) return { text: `${rssi} dBm (Good)`, quality: 'good', bars: 3 }
  if (rssi >= -75) return { text: `${rssi} dBm (Fair)`, quality: 'fair', bars: 2 }
  return { text: `${rssi} dBm (Weak)`, quality: 'poor', bars: 1 }
}

export function formatBytes(bytes: number): string {
  if (!bytes || bytes <= 0) return '0 B'
  const k = 1024
  const sizes = ['B', 'KB', 'MB', 'GB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  return `${(bytes / Math.pow(k, i)).toFixed(1)} ${sizes[i]}`
}

export function relaysToMask(relays: boolean[]): number {
  let mask = 0
  for (let i = 0; i < Math.min(relays.length, 16); i++) {
    if (relays[i]) {
      mask |= 1 << i
    }
  }
  return mask
}

export function maskToRelays(mask: number): boolean[] {
  const relays: boolean[] = new Array(16).fill(false)
  for (let i = 0; i < 16; i++) {
    relays[i] = (mask & (1 << i)) !== 0
  }
  return relays
}

export const SPEED_PRESETS = [
  { label: 'Hyper (40ms)', value: 40 },
  { label: 'Fast (80ms)', value: 80 },
  { label: 'Optimal (120ms)', value: 120 },
  { label: 'Smooth (200ms)', value: 200 },
  { label: 'Moderate (350ms)', value: 350 },
  { label: 'Slow (500ms)', value: 500 },
  { label: 'Crawl (1000ms)', value: 1000 },
]
