import { NextRequest, NextResponse } from 'next/server'
import { authenticateDevice } from '@/lib/device-auth'
import { getDb } from '@/lib/mongodb'

export async function POST(req: NextRequest) {
  try {
    const deviceId = await authenticateDevice(req)
    if (!deviceId) {
      return NextResponse.json(
        { error: 'Unauthorized: Invalid device credentials' },
        { status: 401 }
      )
    }

    const body = await req.json().catch(() => ({}))

    // Normalize relays to 16 numeric elements (0 or 1)
    const rawRelays = body.relays || body.ch || []
    let relays: number[] = []
    if (Array.isArray(rawRelays)) {
      relays = rawRelays.map((r: any) => (r === 1 || r === true ? 1 : 0))
    }
    while (relays.length < 16) {
      relays.push(0)
    }
    if (relays.length > 16) {
      relays = relays.slice(0, 16)
    }

    // Determine operational mode
    let mode: 'manual' | 'auto' | 'stopped' = 'auto'
    if (body.mode === 'manual' || body.mode === 'auto' || body.mode === 'stopped') {
      mode = body.mode
    } else if (body.m === 1) {
      mode = 'manual'
    } else if (body.r === 0) {
      mode = 'stopped'
    }

    const now = new Date()

    const statusDoc = {
      deviceId,
      mode,
      relays,
      effect: body.effect || body.ename || 'Left Chaser',
      effectIndex:
        typeof body.effectIndex === 'number'
          ? body.effectIndex
          : typeof body.e === 'number'
          ? body.e
          : 0,
      cycle:
        typeof body.cycle === 'number'
          ? body.cycle
          : typeof body.cur_rep === 'number'
          ? body.cur_rep
          : 1,
      totalCycles:
        typeof body.totalCycles === 'number'
          ? body.totalCycles
          : typeof body.reps === 'number'
          ? body.reps
          : 3,
      nextEffect: body.nextEffect || '',
      speed:
        typeof body.speed === 'number'
          ? body.speed
          : typeof body.s === 'number'
          ? body.s
          : 120,
      uptime: typeof body.uptime === 'number' ? body.uptime : 0,
      rssi:
        typeof body.rssi === 'number'
          ? body.rssi
          : body.wifi?.rssi ?? -60,
      heap: typeof body.heap === 'number' ? body.heap : 180000,
      minHeap:
        typeof body.minHeap === 'number'
          ? body.minHeap
          : typeof body.heap === 'number'
          ? body.heap - 20000
          : 160000,
      ip: body.ip || body.wifi?.sta_ip || '192.168.4.1',
      firmware: body.firmware || 'v2.0',
      online: true,
      updatedAt: now,
      ...(body.temp !== undefined ? { temp: body.temp } : {}),
      ...(body.wifi?.sta_ssid ? { staSsid: body.wifi.sta_ssid } : {}),
      ...(body.wifi?.sta_ok !== undefined ? { staConnected: body.wifi.sta_ok === 1 } : {}),
    }

    const db = await getDb()

    await Promise.all([
      db.collection('device_status').updateOne(
        { deviceId },
        { $set: statusDoc },
        { upsert: true }
      ),
      db.collection('devices').updateOne(
        { deviceId },
        { $set: { lastSeenAt: now, firmware: statusDoc.firmware } }
      ),
    ])

    return NextResponse.json({ ok: true })
  } catch (error: any) {
    console.error('[ESP Status Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error saving status' },
      { status: 500 }
    )
  }
}
