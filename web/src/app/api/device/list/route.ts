import { NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { getDb } from '@/lib/mongodb'

export const dynamic = 'force-dynamic'

export async function GET() {
  try {
    const session = await getServerSession(authOptions)
    if (!session) {
      return NextResponse.json(
        { error: 'Unauthorized: Operator authentication required' },
        { status: 401 }
      )
    }

    const db = await getDb()
    const devices = await db.collection('devices').find({}).toArray()
    const statuses = await db.collection('device_status').find({}).toArray()

    const statusMap = new Map(statuses.map((s) => [s.deviceId, s]))
    const now = Date.now()

    const result = devices.map((d) => {
      const s = statusMap.get(d.deviceId)
      const updatedAt = s?.updatedAt ? new Date(s.updatedAt).getTime() : 0
      const isOnline = (now - updatedAt) < 10000

      return {
        deviceId: d.deviceId,
        deviceName: d.deviceName || d.deviceId,
        online: isOnline,
        lastSeenAt: d.lastSeenAt || s?.updatedAt || null,
        firmware: d.firmware || s?.firmware || 'v2.0',
        mode: s?.mode || 'auto',
        effect: s?.effect || 'Standby',
        ip: s?.ip || 'N/A',
        rssi: s?.rssi ?? 0,
      }
    })

    return NextResponse.json(result)
  } catch (error: any) {
    console.error('[API Device List Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while listing devices' },
      { status: 500 }
    )
  }
}
