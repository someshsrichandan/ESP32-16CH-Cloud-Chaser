import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { z } from 'zod'
import { getDb } from '@/lib/mongodb'

const RegisterSchema = z.object({
  deviceId: z.string().optional(),
  deviceName: z.string().min(1).max(100).optional(),
  name: z.string().min(1).max(100).optional(),
})

export async function POST(request: NextRequest) {
  try {
    const session = await getServerSession(authOptions)
    if (!session) {
      return NextResponse.json(
        { error: 'Unauthorized: Operator authentication required' },
        { status: 401 }
      )
    }

    const body = await request.json().catch(() => ({}))
    const parsed = RegisterSchema.safeParse(body)
    if (!parsed.success) {
      return NextResponse.json(
        { error: 'Validation failed', details: parsed.error.format() },
        { status: 400 }
      )
    }

    const assignedId =
      parsed.data.deviceId?.trim() ||
      `esp32_${Math.random().toString(36).substring(2, 8)}`
    const assignedName =
      parsed.data.deviceName?.trim() ||
      parsed.data.name?.trim() ||
      `Relay Controller ${assignedId}`

    const db = await getDb()

    await db.collection('devices').updateOne(
      { deviceId: assignedId },
      {
        $setOnInsert: {
          deviceId: assignedId,
          deviceSecret: `secret_${Math.random().toString(36).substring(2, 12)}`,
          firmware: 'v2.0',
          registeredAt: new Date(),
        },
        $set: {
          deviceName: assignedName,
          lastSeenAt: new Date(),
        },
      },
      { upsert: true }
    )

    return NextResponse.json({
      ok: true,
      deviceId: assignedId,
      deviceName: assignedName,
      registeredAt: Date.now(),
    })
  } catch (error: any) {
    console.error('[API Device Register Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while registering device' },
      { status: 500 }
    )
  }
}
