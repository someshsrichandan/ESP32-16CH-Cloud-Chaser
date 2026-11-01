import { NextRequest, NextResponse } from 'next/server'
import { authenticateDevice } from '@/lib/device-auth'
import { getDb } from '@/lib/mongodb'

export const dynamic = 'force-dynamic'

export async function GET(req: NextRequest) {
  try {
    const deviceId = await authenticateDevice(req)
    if (!deviceId) {
      return NextResponse.json(
        { error: 'Unauthorized: Invalid device credentials' },
        { status: 401 }
      )
    }

    const db = await getDb()
    const queue = db.collection('command_queue')

    // Find pending commands in FIFO order, batch up to 10
    const pendingCommands = await queue
      .find({ deviceId, status: 'pending' })
      .sort({ createdAt: 1 })
      .limit(10)
      .toArray()

    if (pendingCommands.length === 0) {
      return NextResponse.json({ commands: [] })
    }

    // Remove retrieved commands from queue
    const ids = pendingCommands.map((doc) => doc._id)
    await queue.deleteMany({ _id: { $in: ids } })

    const commands = pendingCommands.map((doc) => ({
      _id: doc._id.toString(),
      cmd: doc.cmd,
      payload: doc.payload || {},
    }))

    return NextResponse.json({ commands })
  } catch (error: any) {
    console.error('[ESP Poll Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error polling commands' },
      { status: 500 }
    )
  }
}
