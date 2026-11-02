import { NextRequest } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { getDb } from '@/lib/mongodb'
import { DeviceStatus } from '@/types/device'

export const dynamic = 'force-dynamic'

export async function GET(
  request: NextRequest,
  { params }: { params: { deviceId: string } }
) {
  const session = await getServerSession(authOptions)
  if (!session) {
    return new Response('Unauthorized: Operator authentication required', { status: 401 })
  }

  const { deviceId } = params
  if (!deviceId) {
    return new Response('Device ID is required', { status: 400 })
  }

  const encoder = new TextEncoder()

  const stream = new ReadableStream({
    async start(controller) {
      let isClosed = false

      const sendEvent = (event: string, data: any) => {
        if (isClosed) return
        try {
          const json = JSON.stringify(data)
          const payload = `event: ${event}\ndata: ${json}\n\n`
          controller.enqueue(encoder.encode(payload))
        } catch {
          // Stream controller might already be terminated
        }
      }

      const fetchAndSendStatus = async () => {
        if (isClosed) return
        try {
          const db = await getDb()
          const statusDoc = await db.collection('device_status').findOne({ deviceId })

          if (statusDoc) {
            const updatedAt = statusDoc.updatedAt ? new Date(statusDoc.updatedAt).getTime() : 0
            const online = (Date.now() - updatedAt) < 20000

            const rawRelays = Array.isArray(statusDoc.relays) ? statusDoc.relays : []
            const relays: boolean[] = []
            for (let i = 0; i < 16; i++) {
              const val = rawRelays[i]
              relays.push(val === true || val === 1)
            }

            const statusObj: DeviceStatus = {
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

            sendEvent('status', statusObj)
          } else {
            sendEvent('status', {
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
        } catch (err) {
          console.warn('[SSE Stream] Error querying MongoDB status:', err)
        }
      }

      // Send initial status immediately
      await fetchAndSendStatus()

      // Poll MongoDB status every 1500ms
      const pollInterval = setInterval(() => {
        if (isClosed) {
          clearInterval(pollInterval)
          return
        }
        fetchAndSendStatus()
      }, 1500)

      // Send keepalive ping every 15 seconds
      const pingInterval = setInterval(() => {
        if (isClosed) {
          clearInterval(pingInterval)
          return
        }
        sendEvent('ping', { ping: true, time: Date.now() })
      }, 15000)

      const cleanup = () => {
        if (isClosed) return
        isClosed = true
        clearInterval(pollInterval)
        clearInterval(pingInterval)
        try {
          controller.close()
        } catch {
          // Ignore if closed
        }
      }

      request.signal.addEventListener('abort', cleanup)
    },
  })

  return new Response(stream, {
    headers: {
      'Content-Type': 'text/event-stream; charset=utf-8',
      'Cache-Control': 'no-cache, no-transform',
      Connection: 'keep-alive',
      'X-Accel-Buffering': 'no',
    },
  })
}
