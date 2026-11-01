# MongoDB Setup Guide

> **v2.0 — MongoDB replaces both the MQTT broker and Redis cache from v1.0.**
> One free database handles everything: device status, command queue, users, and playlists.

---

## Why MongoDB?

In v2.0 the architecture is simplified to just two services:

| Service | Purpose |
|---|---|
| **MongoDB Atlas** | Stores device status, command queue, users, playlists |
| **Vercel** | Hosts the web app and API routes |

No MQTT broker. No Redis. No HiveMQ account. No Upstash account.

---

## Step 1 — Create a Free Atlas Account

1. Go to [mongodb.com/atlas](https://www.mongodb.com/atlas)
2. Click **Try Free**
3. Sign up with Google or email
4. You will be taken to the Atlas dashboard

---

## Step 2 — Create a Free Cluster

1. Click **Build a Database**
2. Select **M0 Free** tier
3. Choose a cloud provider (AWS / Google Cloud / Azure — any is fine)
4. Choose the region **closest to your Vercel deployment region**
   - If your Vercel app is in `us-east-1`, pick AWS N. Virginia
   - If your Vercel app is in `fra1`, pick AWS Frankfurt
5. Set **Cluster Name** to `relay-chaser` (or any name you prefer)
6. Click **Create**

> [!NOTE]
> The M0 free tier gives you 512 MB storage — more than enough for this project.
> The cluster takes about 1–3 minutes to provision.

---

## Step 3 — Create a Database User

1. In the left sidebar click **Database Access**
2. Click **Add New Database User**
3. Set **Authentication Method** to **Password**
4. Enter a username (e.g. `relaychaser`)
5. Click **Autogenerate Secure Password** — **copy this password now**
6. Under **Built-in Role** select **Read and write to any database**
7. Click **Add User**

> [!WARNING]
> Save the password somewhere safe. If it contains special characters like `@`, `#`, `!`, you must URL-encode them in the connection string (e.g. `@` → `%40`). It's safest to use only letters and numbers in the password.

---

## Step 4 — Allow Network Access

> [!IMPORTANT]
> This step is required. Vercel serverless functions run from dynamic IP addresses that change on every request. You must allow all IPs.

1. In the left sidebar click **Network Access**
2. Click **Add IP Address**
3. Click **Allow Access from Anywhere**
4. This sets the allowlist to `0.0.0.0/0`
5. Click **Confirm**

---

## Step 5 — Get Your Connection String

1. In the left sidebar click **Database** (under Deployment)
2. Click **Connect** on your cluster
3. Select **Drivers**
4. Select **Driver: Node.js**, **Version: 5.5 or later**
5. Copy the connection string — it looks like:
   ```
   mongodb+srv://relaychaser:<password>@relay-chaser.abc12.mongodb.net/?retryWrites=true&w=majority&appName=relay-chaser
   ```
6. Replace `<password>` with your actual password

---

## Step 6 — Configure Environment Variables

### Local Development

Edit `web/.env.local`:
```env
MONGODB_URI=mongodb+srv://relaychaser:yourpassword@relay-chaser.abc12.mongodb.net/?retryWrites=true&w=majority
MONGODB_DB=relay_chaser
```

### Vercel Production

In your Vercel project → **Settings** → **Environment Variables**, add:

| Variable | Value |
|---|---|
| `MONGODB_URI` | Your full connection string |
| `MONGODB_DB` | `relay_chaser` |

> [!TIP]
> Set the environment variable in both **Production** and **Preview** environments in Vercel.

---

## Step 7 — Verify Connection

Run the dev server locally:
```bash
cd web
npm run dev
```

Open http://localhost:3000 and log in. Navigate to **Settings → Devices**.
If MongoDB is connected correctly the page loads without errors.

To test from the command line:
```bash
node -e "
const { MongoClient } = require('mongodb');
const client = new MongoClient(process.env.MONGODB_URI);
client.connect().then(() => { console.log('Connected!'); client.close(); });
"
```

---

## Collections Reference

The app **automatically creates** all collections and indexes on first request. You do not need to create them manually.

| Collection | Purpose | Key Index |
|---|---|---|
| `devices` | Registered ESP32 devices | `deviceId` (unique) |
| `device_status` | Latest telemetry per device | `deviceId` (unique) |
| `command_queue` | Pending commands from browser | `deviceId` + TTL 60s |
| `users` | Web app login accounts | `email` (unique) |
| `playlists` | Saved effect playlists | `deviceId` |

### command_queue TTL Index

The `command_queue` collection has a **60-second TTL index** on the `createdAt` field.

This means: if the ESP32 is offline and doesn't pick up a command within 60 seconds, MongoDB automatically deletes the command. This prevents the queue from filling up with stale commands.

---

## MongoDB Atlas Security Best Practices

> [!CAUTION]
> Never commit your `MONGODB_URI` to Git. The `.env.local` file is already in `.gitignore`.

- Use a dedicated database user for this app (not your Atlas admin account)
- Use a strong password (letters + numbers, avoid special chars in password)
- Do not share your connection string publicly
- Rotate credentials if you suspect a leak: Atlas → Database Access → Edit user → Update password → update Vercel env var → redeploy

---

## Common Errors

### `MongoServerError: bad auth: Authentication failed`
- Wrong username or password in URI
- Password contains special characters that need URL encoding
- Fix: re-check password, re-copy URI from Atlas Connect page

### `MongoNetworkTimeoutError: connection timed out`
- Network Access allowlist not set to `0.0.0.0/0`
- Atlas cluster is paused (free tier pauses after 60 days of inactivity)
- Fix: Atlas dashboard → Network Access → add `0.0.0.0/0`
- Fix: Atlas dashboard → Database → Resume cluster

### `MONGODB_URI is not defined`
- `.env.local` file missing or variable name typo
- Fix: ensure `.env.local` exists in `web/` directory with `MONGODB_URI=...`

### Atlas Cluster Paused
Free M0 clusters pause after 60 days without a connection.
- Go to Atlas dashboard → Database → your cluster → click **Resume**
- The cluster takes 2–3 minutes to resume

---

## Next Steps

After MongoDB is set up:
- [Deploy the web app to Vercel](vercel-deployment.md)
- [Flash the ESP32 firmware](firmware-flashing.md)
- [Register your device](device-registration.md)

