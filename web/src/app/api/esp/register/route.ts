import { NextRequest, NextResponse } from 'next/server'
import { getDb } from '@/lib/mongodb'

export async function POST(req: NextRequest) {
  try {
    const body = await req.json().catch(() => ({}))
    const headerDeviceId = req.headers.get('x-device-id')
    const headerDeviceSecret = req.headers.get('x-device-secret')

    const deviceId = (headerDeviceId || body.deviceId || '').trim()
    const deviceSecret = (headerDeviceSecret || body.deviceSecret || '').trim()
    const deviceName = body.deviceName || `Relay Controller ${deviceId}`
    const firmware = body.firmware || 'v2.0'

    if (!deviceId || !deviceSecret) {
      return NextResponse.json(
        { error: 'Missing deviceId or deviceSecret' },
        { status: 400 }
      )
    }

    const db = await getDb()
    const devices = db.collection('devices')
    const existing = await devices.findOne({ deviceId })

    if (existing) {
      if (existing.deviceSecret && existing.deviceSecret !== deviceSecret) {
        return NextResponse.json(
          { error: 'Unauthorized: Device secret mismatch' },
          { status: 401 }
        )
      }
      await devices.updateOne(
        { deviceId },
        {
          $set: {
            deviceName: body.deviceName || existing.deviceName || deviceName,
            firmware: body.firmware || existing.firmware || firmware,
            lastSeenAt: new Date(),
          },
        }
      )
    } else {
      await devices.insertOne({
        deviceId,
        deviceSecret,
        deviceName,
        firmware,
        registeredAt: new Date(),
        lastSeenAt: new Date(),
      })
    }

    return NextResponse.json({ ok: true, deviceId })
  } catch (error: any) {
    console.error('[ESP Register Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error registering device' },
      { status: 500 }
    )
  }
}
