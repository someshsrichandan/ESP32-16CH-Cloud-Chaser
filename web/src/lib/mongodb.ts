import { MongoClient, Db } from 'mongodb'

const DB_NAME = process.env.MONGODB_DB || 'relay_chaser'

let client: MongoClient
let db: Db

declare global {
  // eslint-disable-next-line no-var
  var _mongoClient: MongoClient | undefined
  // eslint-disable-next-line no-var
  var _mongoDb: Db | undefined
}

export async function getDb(): Promise<Db> {
  const uri = process.env.MONGODB_URI
  if (!uri) {
    throw new Error('MONGODB_URI environment variable is not defined')
  }

  if (process.env.NODE_ENV === 'development') {
    if (!global._mongoClient) {
      global._mongoClient = new MongoClient(uri)
      await global._mongoClient.connect()
      global._mongoDb = global._mongoClient.db(DB_NAME)
    }
    return global._mongoDb!
  }

  if (!client) {
    client = new MongoClient(uri)
    await client.connect()
    db = client.db(DB_NAME)
  }
  return db
}

// Helper: ensure indexes exist (call on first startup)
export async function ensureIndexes() {
  const database = await getDb()
  await database.collection('devices').createIndex({ deviceId: 1 }, { unique: true })
  await database.collection('device_status').createIndex({ deviceId: 1 }, { unique: true })
  await database.collection('command_queue').createIndex({ deviceId: 1 })
  await database.collection('command_queue').createIndex({ createdAt: 1 }, { expireAfterSeconds: 60 })
  await database.collection('users').createIndex({ email: 1 }, { unique: true })
  await database.collection('playlists').createIndex({ deviceId: 1 })
}
