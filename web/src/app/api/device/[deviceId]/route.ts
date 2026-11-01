import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { getDb } from '@/lib/mongodb'

export async function DELETE(
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
      return NextResponse.json({ error: 'Device ID required' }, { status: 400 })
    }

    const db = await getDb()
    await Promise.all([
      db.collection('devices').deleteOne({ deviceId }),
      db.collection('device_status').deleteOne({ deviceId }),
      db.collection('command_queue').deleteMany({ deviceId }),
    ])

    return NextResponse.json({ ok: true, deleted: deviceId })
  } catch (error: any) {
    console.error('[API Delete Device Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error deleting device' },
      { status: 500 }
    )
  }
}

