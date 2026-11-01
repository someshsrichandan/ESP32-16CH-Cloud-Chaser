import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { z } from 'zod'
import { getDb } from '@/lib/mongodb'

export const dynamic = 'force-dynamic'

const PlaylistEntrySchema = z.object({
  id: z.string().optional(),
  effectIndex: z.number().int().min(0),
  effectName: z.string().min(1),
  cycles: z.number().int().min(1),
  speedMs: z.number().int().min(10),
  enabled: z.boolean().default(true),
})

const PlaylistSchema = z.object({
  name: z.string().min(1).default('Untitled Playlist'),
  entries: z.array(PlaylistEntrySchema).default([]),
})

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
    const playlists = await db
      .collection('playlists')
      .find({ deviceId })
      .sort({ updatedAt: -1 })
      .toArray()

    const formattedPlaylists = playlists.map((p) => ({
      _id: p._id.toString(),
      deviceId: p.deviceId,
      name: p.name,
      entries: p.entries,
      createdAt: p.createdAt,
      updatedAt: p.updatedAt,
    }))

    return NextResponse.json({ playlists: formattedPlaylists })
  } catch (error: any) {
    console.error('[API Playlist GET Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while retrieving playlists' },
      { status: 500 }
    )
  }
}

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
    const parsed = PlaylistSchema.safeParse(body)
    if (!parsed.success) {
      return NextResponse.json(
        { error: 'Validation failed', details: parsed.error.format() },
        { status: 400 }
      )
    }

    const db = await getDb()
    const now = new Date()

    const doc = {
      deviceId,
      name: parsed.data.name,
      entries: parsed.data.entries,
      createdAt: now,
      updatedAt: now,
    }

    const result = await db.collection('playlists').insertOne(doc)

    return NextResponse.json({
      ok: true,
      playlist: {
        _id: result.insertedId.toString(),
        ...doc,
      },
    })
  } catch (error: any) {
    console.error('[API Playlist POST Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while creating playlist' },
      { status: 500 }
    )
  }
}
