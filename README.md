# ESP32 16-Channel Smart Wi-Fi Relay Chaser Controller

**v2.0 — Direct HTTP + MongoDB (No MQTT, No Redis, No Broker)**

A professional 16-channel relay chaser controller built on ESP32 with a modern Next.js web dashboard hosted on Vercel. Control 16 relays remotely from anywhere — no port forwarding, no static IP, no router configuration required.

---

## How It Works

```
ESP32 (your hardware)
  │
  ├─ POST /api/esp/status   every 2s  ──► Vercel API ──► MongoDB
  └─ GET  /api/esp/poll     every 1s  ◄── Vercel API ◄── MongoDB
                                                │
                          Your Browser ◄── SSE Stream ◄──┘
                          (Dashboard, Manual, Effects, Playlist)
```

The ESP32 makes direct HTTPS requests to your Vercel app. No MQTT broker, no Redis — just MongoDB as the single database.

---

## Project Structure

```
esp32-relay-chaser/
├── README.md                        ← You are here
├── firmware/
│   └── esp32_relay_chaser/          ← ESP32 Arduino firmware (PlatformIO)
│       ├── esp32_relay_chaser.ino   ← Main entry point
│       ├── config.h                 ← GPIO map, timing, speed presets
│       ├── gpio_controller.*        ← Centralized relay control (Active-LOW)
│       ├── effect_engine.*          ← Non-blocking millis() effect runner
│       ├── effect_library.*         ← All 30 chasing effects
│       ├── cycle_manager.*          ← Playlist & cycle sequencer
│       ├── speed_controller.*       ← Speed presets & custom ms
│       ├── wifi_manager.*           ← Wi-Fi + auto-reconnect
│       ├── http_manager.*           ← HTTPS GET/POST to Vercel API  ← NEW v2.0
│       ├── command_handler.*        ← Dispatches 13 command types   ← NEW v2.0
│       ├── config_manager.*         ← NVS persistence (Wi-Fi, API URL, secret)
│       ├── status_manager.*         ← Builds & pushes JSON telemetry
│       ├── diagnostics.*            ← Watchdog, heap monitor, uptime
│       └── platformio.ini           ← Build config (ArduinoJson only)
│
├── web/                             ← Next.js 14 web application
│   ├── src/
│   │   ├── app/
│   │   │   ├── page.tsx             ← Dashboard
│   │   │   ├── manual/page.tsx      ← 4×4 relay grid manual control
│   │   │   ├── effects/page.tsx     ← 30-effect library with preview
│   │   │   ├── playlist/page.tsx    ← Drag-and-drop playlist manager
│   │   │   ├── settings/page.tsx    ← Device settings
│   │   │   ├── diagnostics/page.tsx ← System health monitor
│   │   │   └── api/
│   │   │       ├── esp/             ← ESP32-facing endpoints
│   │   │       │   ├── register/    ← ESP32 self-registers on boot
│   │   │       │   ├── status/      ← ESP32 pushes telemetry
│   │   │       │   └── poll/        ← ESP32 picks up commands
│   │   │       └── device/          ← Browser-facing endpoints
│   │   │           └── [deviceId]/
│   │   │               ├── command/ ← Queue a command
│   │   │               ├── status/  ← Get device status
│   │   │               ├── stream/  ← SSE real-time stream
│   │   │               └── playlist/← Save/load playlists
│   │   ├── components/              ← All UI components
│   │   ├── hooks/                   ← use-device-status, use-device-command
│   │   ├── lib/
│   │   │   ├── mongodb.ts           ← MongoDB singleton connection
│   │   │   ├── device-auth.ts       ← Authenticates ESP32 requests
│   │   │   ├── auth.ts              ← NextAuth config
│   │   │   ├── effects-data.ts      ← 30 effect definitions
│   │   │   └── utils.ts             ← Helpers
│   │   └── types/                   ← TypeScript interfaces
│   ├── .env.example                 ← Environment variable template
│   └── package.json
│
└── docs/
    ├── hardware-wiring.md           ← GPIO wiring, power, safety
    ├── mongodb-setup.md             ← MongoDB Atlas free-tier setup
    ├── firmware-flashing.md         ← PlatformIO flash guide
    ├── vercel-deployment.md         ← Vercel deploy + env vars
    ├── device-registration.md       ← First-time device setup
    └── troubleshooting.md           ← Common issues & fixes
```

---

## Quick Start

### 1. Set Up MongoDB (5 minutes)
Create a **free MongoDB Atlas** cluster at [mongodb.com/atlas](https://www.mongodb.com/atlas).
→ See [docs/mongodb-setup.md](docs/mongodb-setup.md)

### 2. Deploy to Vercel
Push this repo to GitHub, import on Vercel, set environment variables.
→ See [docs/vercel-deployment.md](docs/vercel-deployment.md)

### 3. Flash ESP32
Open `firmware/esp32_relay_chaser/` in VS Code with PlatformIO. Click Upload.
Enter Wi-Fi credentials and your Vercel URL in the first-boot serial wizard.
→ See [docs/firmware-flashing.md](docs/firmware-flashing.md)

### 4. Register Your Device
Copy the Device ID from Serial Monitor. Log into the web app. Register the device.
→ See [docs/device-registration.md](docs/device-registration.md)

---

## Environment Variables

```env
MONGODB_URI=mongodb+srv://user:pass@cluster.mongodb.net/?retryWrites=true&w=majority
MONGODB_DB=relay_chaser
NEXTAUTH_SECRET=your-32-char-random-secret
NEXTAUTH_URL=https://your-app.vercel.app
ADMIN_EMAIL=admin@relaychaser.local
ADMIN_PASSWORD_HASH=$2b$10$...bcrypt-hash...
NEXT_PUBLIC_APP_URL=https://your-app.vercel.app
```

> **No MQTT variables. No Redis variables. No broker accounts needed.**

---

## Hardware

| Relay | GPIO | | Relay | GPIO |
|---|---|---|---|---|
| 1 | 25 | | 9 | 16 |
| 2 | 4  | | 10 | 17 |
| 3 | 5  | | 11 | 18 |
| 4 | 26 | | 12 | 19 |
| 5 | 13 | | 13 | 21 |
| 6 | 27 | | 14 | 22 |
| 7 | 15 | | 15 | 23 |
| 8 | 32 | | 16 | 33 |

**Active-LOW logic**: `LOW` = Relay ON, `HIGH` = Relay OFF.
→ See full wiring guide: [docs/hardware-wiring.md](docs/hardware-wiring.md)

---

## Features

### ESP32 Firmware
- 30 professional chasing effects (Sequential, Knight Rider, Waterfall, Strobe, Spiral, and more)
- Non-blocking effect engine (`millis()`-based, zero `delay()` in loop)
- Cycle manager: run each effect N times before switching
- Drag-and-drop playlist with configurable speeds
- Speed control: 20ms – 2000ms (5 presets + custom)
- Auto-reconnect Wi-Fi with exponential backoff
- NVS flash persistence (survives power cycle)
- Hardware watchdog protection
- Offline operation: effects continue running without internet

### Web Dashboard
- Live connection status and relay grid
- Manual 4×4 relay control grid
- 30-effect library with animated CSS previews
- Drag-and-drop playlist manager
- Real-time SSE updates (no page refresh needed)
- Dark professional UI (Tailwind CSS + shadcn/ui)
- Mobile responsive

---

## API Reference (ESP32 ↔ Vercel)

| Method | Endpoint | Auth | Purpose |
|---|---|---|---|
| POST | `/api/esp/register` | Device headers | ESP32 self-registers at boot |
| POST | `/api/esp/status` | Device headers | ESP32 pushes telemetry every 2s |
| GET | `/api/esp/poll` | Device headers | ESP32 picks up pending commands |
| POST | `/api/device/{id}/command` | Session | Browser queues a command |
| GET | `/api/device/{id}/status` | Session | Browser reads device state |
| GET | `/api/device/{id}/stream` | Session | Browser SSE real-time stream |

**ESP32 authentication headers:**
```
X-Device-Id: esp32_a1b2c3d4
X-Device-Secret: abc123...32hexchars
```

---

## Local Development

```bash
cd web
npm install
# Edit .env.local — add your MONGODB_URI
npm run dev
```

Open http://localhost:3000 — Login: `admin@relaychaser.local` / `admin`

---

## Tech Stack

| Layer | Technology |
|---|---|
| ESP32 Firmware | Arduino C++, ArduinoJson, HTTPClient (built-in), NVS Preferences |
| Web Frontend | Next.js 14, TypeScript, Tailwind CSS, shadcn/ui, Framer Motion |
| Web Backend | Next.js API Routes (Vercel Serverless) |
| Database | MongoDB Atlas (free M0 tier) |
| Auth | NextAuth.js (credentials + JWT) |
| Real-time | Server-Sent Events (SSE) |
| Deployment | Vercel |

---

## Troubleshooting

→ See [docs/troubleshooting.md](docs/troubleshooting.md) for common issues.

---

## License

MIT

