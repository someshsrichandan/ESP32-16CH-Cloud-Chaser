import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { z } from 'zod'
import { getDb } from '@/lib/mongodb'

const VALID_COMMAND_TYPES = [
  'SET_RELAY',
  'MASTER_ON',
  'MASTER_OFF',
  'EMERGENCY_STOP',
  'SET_MODE',
  'SET_EFFECT',
  'SET_SPEED',
  'PAUSE',
  'RESUME',
  'STOP',
  'NEXT_EFFECT',
  'PREV_EFFECT',
  'SET_PLAYLIST',
  'REQUEST_STATE',
] as const

type ValidCommandType = typeof VALID_COMMAND_TYPES[number]

const CommandSchema = z.object({
  cmd: z.string().min(1),
  payload: z.union([z.record(z.unknown()), z.array(z.unknown())]).optional(),
  ch: z.number().optional(),
  v: z.union([z.number(), z.string()]).optional(),
  id: z.number().optional(),
  a: z.string().optional(),
  mask: z.number().optional(),
  name: z.string().optional(),
  speed: z.number().optional(),
  playlist: z.array(z.unknown()).optional(),
  entries: z.array(z.unknown()).optional(),
})

export async function POST(
  request: NextRequest,
  { params }: { params: { deviceId: string } }
) {
  try {
    const session = await getServerSession(authOptions)
    if (!session) {
      return NextResponse.json(
        { error: 'Unauthorized: Operator authentication required' },
        { status: 401 }
      )
    }

    const { deviceId } = params
    if (!deviceId) {
      return NextResponse.json({ error: 'Device ID is required' }, { status: 400 })
    }

    const body = await request.json().catch(() => ({}))
    const parsed = CommandSchema.safeParse(body)

    if (!parsed.success) {
      return NextResponse.json(
        { error: 'Validation failed', details: parsed.error.format() },
        { status: 400 }
      )
    }

    const rawData = parsed.data
    let targetCmd = rawData.cmd.trim()
    let targetPayload: Record<string, unknown> = rawData.payload ? { ...rawData.payload } : {}

    // Normalize legacy command invocations into one of the valid commands
    if (targetCmd === 'relay' || targetCmd === 'RELAY') {
      targetCmd = 'SET_RELAY'
      const channel = rawData.ch ?? targetPayload.channel ?? targetPayload.relay ?? targetPayload.ch ?? 0
      const state = (rawData.v ?? targetPayload.state ?? targetPayload.v ?? 1) ? true : false
      targetPayload = {
        channel,
        relay: channel,
        state,
        ...targetPayload,
      }
    } else if (targetCmd === 'action' || targetCmd === 'ACTION') {
      const act = rawData.a || targetPayload.a
      if (act === 'all_on') {
        targetCmd = 'MASTER_ON'
      } else if (act === 'all_off') {
        targetCmd = 'MASTER_OFF'
      } else if (act === 'pause_auto' || act === 'pause') {
        targetCmd = 'PAUSE'
      } else if (act === 'toggle_run' || act === 'auto_on' || act === 'resume') {
        targetCmd = 'RESUME'
      } else if (act === 'auto_off' || act === 'stop') {
        targetCmd = 'STOP'
      } else if (act === 'next') {
        targetCmd = 'NEXT_EFFECT'
      } else if (act === 'prev') {
        targetCmd = 'PREV_EFFECT'
      } else if (act === 'emergency_stop') {
        targetCmd = 'EMERGENCY_STOP'
      } else if (act === 'odd') {
        targetCmd = 'SET_RELAY'
        targetPayload = { bitmask: 0xAAAA }
      } else if (act === 'even') {
        targetCmd = 'SET_RELAY'
        targetPayload = { bitmask: 0x5555 }
      } else if (act === 'invert') {
        targetCmd = 'SET_RELAY'
        targetPayload = { invert: true }
      } else if (act === 'toggle_auto') {
        targetCmd = 'SET_MODE'
        targetPayload = { mode: 'auto' }
      } else if (act === 'toggle_shuffle') {
        targetCmd = 'NEXT_EFFECT'
      }
    } else if (targetCmd === 'SET_PLAYLIST' || targetCmd === 'playlist' || targetCmd === 'PLAYLIST') {
      targetCmd = 'SET_PLAYLIST'
      const pl = rawData.playlist || rawData.entries || (Array.isArray(rawData.payload) ? rawData.payload : (targetPayload.playlist || targetPayload.entries))
      if (Array.isArray(pl)) {
        targetPayload = { playlist: pl }
      }
    } else if (targetCmd === 'effect' || targetCmd === 'EFFECT') {
      targetCmd = 'SET_EFFECT'
      targetPayload = {
        effectIndex: rawData.id ?? targetPayload.id ?? targetPayload.effectIndex ?? 0,
        ...targetPayload,
      }
    } else if (targetCmd === 'speed' || targetCmd === 'SPEED') {
      targetCmd = 'SET_SPEED'
      targetPayload = {
        speed: rawData.v ?? targetPayload.speed ?? targetPayload.v ?? 120,
        ...targetPayload,
      }
    } else if (targetCmd.toLowerCase() === 'request_state') {
      targetCmd = 'REQUEST_STATE'
    } else if (targetCmd.toLowerCase() === 'repeats') {
      targetCmd = 'SET_MODE'
      targetPayload = {
        cycles: rawData.v ?? targetPayload.v ?? targetPayload.cycles ?? 3,
        ...targetPayload,
      }
    }

    // Validate that final cmd is one of the 13 supported commands
    if (!VALID_COMMAND_TYPES.includes(targetCmd as ValidCommandType)) {
      return NextResponse.json(
        {
          error: `Invalid command type '${targetCmd}'. Must be one of: ${VALID_COMMAND_TYPES.join(', ')}`,
        },
        { status: 400 }
      )
    }

    const db = await getDb()

    // Verify that device is registered
    let device = await db.collection('devices').findOne({ deviceId })
    if (!device) {
      if (process.env.NODE_ENV === 'development' || deviceId === 'esp32_relay_01') {
        await db.collection('devices').insertOne({
          deviceId,
          deviceSecret: 'dev_secret_auto_created',
          deviceName: `ESP32 Controller (${deviceId})`,
          firmware: 'v2.0',
          registeredAt: new Date(),
          lastSeenAt: new Date(),
        })
        device = await db.collection('devices').findOne({ deviceId })
      } else {
        return NextResponse.json(
          { error: `Device '${deviceId}' not found in registered devices` },
          { status: 404 }
        )
      }
    }

    // Insert pending command into command_queue
    await db.collection('command_queue').insertOne({
      deviceId,
      cmd: targetCmd,
      payload: targetPayload,
      createdAt: new Date(),
      status: 'pending',
    })

    return NextResponse.json({ ok: true, queued: true, cmd: targetCmd })
  } catch (error: any) {
    console.error('[API Command Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while dispatching command' },
      { status: 500 }
    )
  }
}
