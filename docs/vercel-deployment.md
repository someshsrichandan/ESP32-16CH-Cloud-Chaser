# Vercel Deployment Guide

> **v2.0 — No MQTT, No Redis, No Upstash. Only MongoDB Atlas required.**

---

## Prerequisites

Before deploying, have these ready:

- [ ] GitHub account
- [ ] Vercel account (free at [vercel.com](https://vercel.com))
- [ ] MongoDB Atlas connection string (see [mongodb-setup.md](mongodb-setup.md))
- [ ] `bcryptjs` installed locally for generating password hash (optional)

---

## Step 1 — Push to GitHub

From the project root (`c:\Users\somes\Desktop\esp-32\test`):

```bash
git init
git add .
git commit -m "ESP32 Relay Chaser v2.0 - HTTP + MongoDB"

# Create a repo on github.com first, then:
git remote add origin https://github.com/YOUR_USERNAME/esp32-relay-chaser.git
git push -u origin main
```

> [!TIP]
> The `web/.env.local` file is in `.gitignore` — your secrets will NOT be pushed to GitHub.

---

## Step 2 — Import Project on Vercel

1. Go to [vercel.com](https://vercel.com) → **Add New Project**
2. Click **Import** next to your GitHub repository
3. Under **Framework Preset** — Vercel should auto-detect **Next.js**
4. Set **Root Directory** to **`web`** ← important
5. Do NOT click Deploy yet — configure environment variables first

---

## Step 3 — Set Environment Variables

In the Vercel import screen, click **Environment Variables** and add all of the following:

| Variable | Description | Example Value |
|---|---|---|
| `MONGODB_URI` | Full MongoDB Atlas connection string | `mongodb+srv://user:pass@cluster.mongodb.net/...` |
| `MONGODB_DB` | Database name | `relay_chaser` |
| `NEXTAUTH_SECRET` | Random secret (min 32 chars) | See generation command below |
| `NEXTAUTH_URL` | Your Vercel app URL | `https://your-app.vercel.app` |
| `ADMIN_EMAIL` | Admin login email | `admin@relaychaser.local` |
| `ADMIN_PASSWORD_HASH` | Bcrypt hash of admin password | See generation command below |
| `NEXT_PUBLIC_APP_URL` | Same as NEXTAUTH_URL | `https://your-app.vercel.app` |

### Generate NEXTAUTH_SECRET

```bash
# On Mac/Linux:
openssl rand -base64 32

# On Windows PowerShell:
[Convert]::ToBase64String([Security.Cryptography.RandomNumberGenerator]::GetBytes(32))
```

### Generate ADMIN_PASSWORD_HASH

```bash
# In the web/ directory (bcryptjs is already installed):
node -e "const b=require('bcryptjs'); console.log(b.hashSync('YourChosenPassword', 10))"
```

Copy the output (starts with `$2b$10$...`) and paste as the value.

> [!IMPORTANT]
> Set these variables for **both Production and Preview** environments in Vercel. Use the checkbox when adding each variable.

---

## Step 4 — Deploy

Click **Deploy**. Vercel will:
1. Install npm dependencies
2. Run `next build`
3. Deploy to edge network

This takes about 2–3 minutes. When done, you'll see a URL like `https://esp32-relay-chaser-abc.vercel.app`.

---

## Step 5 — Update NEXTAUTH_URL

After deployment, copy the final Vercel URL.

Go to **Vercel → Project → Settings → Environment Variables**:
- Update `NEXTAUTH_URL` to your exact Vercel URL
- Update `NEXT_PUBLIC_APP_URL` to the same URL
- Click **Save** and **Redeploy** (top right → Deployments → ... → Redeploy)

---

## Step 6 — Test the Web App

1. Open your Vercel URL
2. You should see the login page
3. Log in with:
   - **Email:** value of `ADMIN_EMAIL`
   - **Password:** the plain-text password you hashed above
4. The Dashboard should load — device will show **Offline** until ESP32 connects

---

## Step 7 — Register Your Device

After flashing the ESP32 and getting its Device ID from Serial Monitor:

1. In the web app → **Settings** page
2. Find the **Register Device** section
3. Enter the **Device ID** (e.g. `esp32_a1b2c3d4`)
4. Enter a **Device Name** (e.g. `Living Room Chaser`)
5. Click **Register**

Once the ESP32 boots and starts pushing status, the Dashboard will show it as **Online** within 10 seconds.

---

## Custom Domain (Optional)

1. In Vercel → Project → **Settings → Domains**
2. Click **Add Domain**
3. Enter your domain (e.g. `relay.yourdomain.com`)
4. Add the DNS record shown (CNAME → `cname.vercel-dns.com`)
5. Wait for DNS propagation (up to 48h, usually minutes)
6. Update `NEXTAUTH_URL` and `NEXT_PUBLIC_APP_URL` to the new domain → Redeploy

---

## Redeploying After Code Changes

```bash
git add .
git commit -m "your changes"
git push
```

Vercel automatically deploys on every push to `main`. No manual action needed.

---

## Viewing Logs

- Go to Vercel → Project → **Deployments** → click latest → **Functions**
- Click any API route to see its runtime logs
- This is useful for debugging MongoDB connection issues

---

## SSE Timeout Note

> [!NOTE]
> Vercel Hobby plan limits serverless functions to **10 seconds**. The SSE stream (`/api/device/{id}/stream`) will disconnect every ~10 seconds. The `use-device-status` hook automatically reconnects — this is handled gracefully and you'll see continuous updates. Upgrade to **Vercel Pro** for persistent connections if needed.

---

## Environment Variables Quick Reference

```env
MONGODB_URI=mongodb+srv://username:password@cluster.mongodb.net/?retryWrites=true&w=majority
MONGODB_DB=relay_chaser
NEXTAUTH_SECRET=REPLACE_WITH_32_CHAR_RANDOM_STRING
NEXTAUTH_URL=https://your-app.vercel.app
ADMIN_EMAIL=admin@relaychaser.local
ADMIN_PASSWORD_HASH=$2b$10$...bcrypt-hash...
NEXT_PUBLIC_APP_URL=https://your-app.vercel.app
```

> [!CAUTION]
> Never commit real credentials to Git. Always use Vercel's Environment Variables panel for production secrets.

---

## Next Steps

- [Flash the ESP32 firmware](firmware-flashing.md)
- [Register your device](device-registration.md)
- [Troubleshooting](troubleshooting.md)
