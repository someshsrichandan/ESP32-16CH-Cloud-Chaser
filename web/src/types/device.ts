export type DeviceMode = 'auto' | 'manual' | 'stopped'

export enum CommandType {
  RELAY = 'relay',
  EFFECT = 'effect',
  SPEED = 'speed',
  REPEATS = 'repeats',
  ACTION = 'action',
  STREAM_FRAME = 'stream_frame',
  LOAD_PATTERN = 'load_pattern',
  REQUEST_STATE = 'request_state',
}

export type ActionCommandType =
  | 'all_on'
  | 'all_off'
  | 'toggle_run'
  | 'toggle_auto'
  | 'auto_on'
  | 'auto_off'
  | 'pause_auto'
  | 'toggle_shuffle'
  | 'toggle_manual'
  | 'next'
  | 'prev'
  | 'odd'
  | 'even'
  | 'invert'

export interface DeviceStatus {
  deviceId: string
  mode: DeviceMode
  relays: boolean[] // Exactly 16 elements
  effect: string
  effectIndex: number
  cycle: number
  totalCycles: number
  nextEffect?: string
  speed: number // in ms (40..1000)
  uptime: number // seconds
  rssi: number // dBm
  heap: number // bytes
  minHeap: number // bytes
  ip: string
  ts: number // unix epoch ms
  online: boolean
  temp?: number
  staSsid?: string
  staConnected?: boolean
  httpConnected?: boolean
  customPatternName?: string
  customPatternCount?: number
}

export interface DeviceCommand {
  cmd: CommandType | string
  payload?: Record<string, unknown>
  ts?: number
  // Common payload fields
  ch?: number
  v?: number | string
  id?: number
  a?: ActionCommandType | string
  mask?: number
  name?: string
  speed?: number
  frames?: number[]
}

export interface PlaylistEntry {
  id: string
  effectIndex: number
  effectName: string
  cycles: number
  speedMs: number
  enabled: boolean
}

export interface EffectPreset {
  id: number
  name: string
  description: string
  category: 'chase' | 'strobe' | 'random' | 'pattern' | 'fill' | 'wave'
  defaultSpeed: number
  tags: string[]
  animationPattern: string
}

export interface DeviceRegistration {
  deviceId: string
  name: string
  createdAt: number
  lastSeen?: number
  notes?: string
}
