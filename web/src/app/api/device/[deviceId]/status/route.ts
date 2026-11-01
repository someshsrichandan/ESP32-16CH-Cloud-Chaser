import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { getDb } from '@/lib/mongodb'
import { DeviceStatus } from '@/types/device'

export const dynamic = 'force-dynamic'

export async function GET(
  _request: NextRequest,
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

    const db = await getDb()
    const statusDoc = await db.collection('device_status').findOne({ deviceId })

    if (!statusDoc) {
      return NextResponse.json({
        deviceId,
        mode: 'stopped',
        relays: new Array(16).fill(false),
        effect: 'Standby',
        effectIndex: 0,
        cycle: 0,
        totalCycles: 0,
        speed: 120,
        uptime: 0,
        rssi: 0,
        heap: 0,
        minHeap: 0,
        ip: '0.0.0.0',
        ts: Date.now(),
        online: false,
      })
    }

    const updatedAt = statusDoc.updatedAt ? new Date(statusDoc.updatedAt).getTime() : 0
    const online = (Date.now() - updatedAt) < 10000

    const rawRelays = Array.isArray(statusDoc.relays) ? statusDoc.relays : []
    const relays: boolean[] = []
    for (let i = 0; i < 16; i++) {
      const val = rawRelays[i]
      relays.push(val === true || val === 1)
    }

    const responseStatus: DeviceStatus = {
      deviceId: statusDoc.deviceId,
      mode: statusDoc.mode || 'auto',
      relays,
      effect: statusDoc.effect || 'Left Chaser',
      effectIndex: statusDoc.effectIndex ?? 0,
      cycle: statusDoc.cycle ?? 1,
      totalCycles: statusDoc.totalCycles ?? 3,
      nextEffect: statusDoc.nextEffect || '',
      speed: statusDoc.speed ?? 120,
      uptime: statusDoc.uptime ?? 0,
      rssi: statusDoc.rssi ?? -60,
      heap: statusDoc.heap ?? 180000,
      minHeap: statusDoc.minHeap ?? 160000,
      ip: statusDoc.ip || '192.168.4.1',
      ts: updatedAt || Date.now(),
      online,
      temp: statusDoc.temp,
      staSsid: statusDoc.staSsid,
      staConnected: statusDoc.staConnected,
    }

    return NextResponse.json(responseStatus)
  } catch (error: any) {
    console.error('[API Status Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while retrieving status' },
      { status: 500 }
    )
  }
}
