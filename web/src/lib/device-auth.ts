import { getDb } from './mongodb'
import { NextRequest } from 'next/server'

// Validates X-Device-Id and X-Device-Secret headers
// Returns deviceId string if valid, null if invalid
export async function authenticateDevice(req: NextRequest): Promise<string | null> {
  const deviceId = req.headers.get('x-device-id')
  const deviceSecret = req.headers.get('x-device-secret')
  if (!deviceId || !deviceSecret) return null

  try {
    const db = await getDb()
    const device = await db.collection('devices').findOne({ deviceId })
    if (!device) return null

    // Compare secret directly (stored as plain in devices, or hash — use plain for simplicity)
    if (device.deviceSecret !== deviceSecret) return null

    return deviceId
  } catch (err) {
    console.error('[authenticateDevice] Error querying database:', err)
    return null
  }
}
