import { NextRequest, NextResponse } from 'next/server'
import { getServerSession } from 'next-auth/next'
import { authOptions } from '@/lib/auth'
import { z } from 'zod'
import { getDb } from '@/lib/mongodb'
import { ObjectId } from 'mongodb'
import { EFFECTS_DATA } from '@/lib/effects-data'

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
  id: z.string().optional(),
  _id: z.string().optional(),
  name: z.string().min(1).default('Untitled Preset'),
  entries: z.array(PlaylistEntrySchema).default([]),
})

// Built-in presets based on user requirements
const DEFAULT_SYSTEM_PRESETS = [
  {
    name: '✨ 5-Cycle Complete Showcase (All 18 FX)',
    entries: EFFECTS_DATA.map((fx, idx) => ({
      id: `sys_${idx}`,
      effectIndex: fx.id,
      effectName: fx.name,
      cycles: 5, // Exact 5 cycles as requested
      speedMs: fx.defaultSpeed,
      enabled: true,
    })),
  },
  {
    name: '⚡ High-Energy Strobes & Flashes (5x)',
    entries: [
      { id: 'he_1', effectIndex: 3, effectName: 'Odd / Even Alternating Flash', cycles: 5, speedMs: 200, enabled: true },
      { id: 'he_2', effectIndex: 7, effectName: 'All-Channel Synchronous Strobe', cycles: 5, speedMs: 180, enabled: true },
      { id: 'he_3', effectIndex: 15, effectName: 'Split-Half Alternating Blocks', cycles: 5, speedMs: 160, enabled: true },
    ],
  },
  {
    name: '🌟 Larson Scanners & Ping-Pong (5x)',
    entries: [
      { id: 'lp_1', effectIndex: 1, effectName: 'Single Forward Running Dot', cycles: 5, speedMs: 140, enabled: true },
      { id: 'lp_2', effectIndex: 8, effectName: 'Dual-LED Ping-Pong Chase', cycles: 5, speedMs: 150, enabled: true },
      { id: 'lp_3', effectIndex: 16, effectName: 'Knight Rider Larson Scanner', cycles: 5, speedMs: 150, enabled: true },
      { id: 'lp_4', effectIndex: 17, effectName: 'Symmetrical Curtain Ping-Pong', cycles: 5, speedMs: 160, enabled: true },
    ],
  },
  {
    name: '🌊 Fluid Wipes & Stackers (5x)',
    entries: [
      { id: 'ws_1', effectIndex: 0, effectName: 'Progressive Forward Fill & Empty', cycles: 5, speedMs: 150, enabled: true },
      { id: 'ws_2', effectIndex: 4, effectName: 'Dual-Direction Wipe & Clear', cycles: 5, speedMs: 130, enabled: true },
      { id: 'ws_3', effectIndex: 5, effectName: 'Outside-In Converging Wipe', cycles: 5, speedMs: 150, enabled: true },
      { id: 'ws_4', effectIndex: 6, effectName: 'Center-Out Diverging Wipe', cycles: 5, speedMs: 150, enabled: true },
      { id: 'ws_5', effectIndex: 11, effectName: 'Accumulator Drop Stacking', cycles: 3, speedMs: 120, enabled: true },
    ],
  },
]

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
    const customPlaylists = await db
      .collection('playlists')
      .find({ deviceId })
      .sort({ updatedAt: -1 })
      .toArray()

    const formattedCustom = customPlaylists.map((p) => ({
      _id: p._id.toString(),
      deviceId: p.deviceId,
      name: p.name,
      entries: p.entries,
      createdAt: p.createdAt,
      updatedAt: p.updatedAt,
      isCustom: true,
    }))

    // Merge system presets with user's custom saved presets
    const allPresets = [
      ...formattedCustom,
      ...DEFAULT_SYSTEM_PRESETS.map((p, idx) => ({
        _id: `system_${idx}`,
        deviceId,
        name: p.name,
        entries: p.entries,
        createdAt: new Date().toISOString(),
        updatedAt: new Date().toISOString(),
        isCustom: false,
      })),
    ]

    return NextResponse.json({ playlists: allPresets })
  } catch (error: any) {
    console.error('[API Playlist GET Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while retrieving presets' },
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

    const presetId = body._id || body.id

    if (presetId && !presetId.startsWith('system_')) {
      // Update existing preset
      try {
        await db.collection('playlists').updateOne(
          { _id: new ObjectId(presetId), deviceId },
          {
            $set: {
              name: parsed.data.name,
              entries: parsed.data.entries,
              updatedAt: now,
            },
          }
        )
        return NextResponse.json({
          ok: true,
          playlist: {
            _id: presetId,
            deviceId,
            name: parsed.data.name,
            entries: parsed.data.entries,
            updatedAt: now,
            isCustom: true,
          },
        })
      } catch {
        // Fall through to insert if invalid ObjectId
      }
    }

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
        isCustom: true,
      },
    })
  } catch (error: any) {
    console.error('[API Playlist POST Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while saving preset' },
      { status: 500 }
    )
  }
}

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
    const { searchParams } = new URL(request.url)
    const id = searchParams.get('id')

    if (!id || !deviceId) {
      return NextResponse.json({ error: 'Preset ID and deviceId are required' }, { status: 400 })
    }

    if (id.startsWith('system_')) {
      return NextResponse.json({ error: 'Cannot delete built-in system presets' }, { status: 400 })
    }

    const db = await getDb()
    await db.collection('playlists').deleteOne({
      _id: new ObjectId(id),
      deviceId,
    })

    return NextResponse.json({ ok: true, deleted: id })
  } catch (error: any) {
    console.error('[API Playlist DELETE Error]:', error)
    return NextResponse.json(
      { error: 'Internal Server Error while deleting preset' },
      { status: 500 }
    )
  }
}
