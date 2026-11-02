# ESP32 × 16-Channel Cloud Chaser PRO (v8.5)

A professional, enterprise-grade dual-control 16-channel relay sequencer and lighting automation platform for the **ESP32** and **UCTRONICS 16-Channel Relay Module**. 

Features a **unified dual-control architecture**:
1. **Local Cyberpunk Web Controller**: Hosted directly on the ESP32 in PROGMEM flash on port 80 (accessible via SoftAP `192.168.4.1` or Local LAN).
2. **Cloud Web Application**: Built with **Next.js 16 (Turbopack)**, **React 19**, **TypeScript**, **Tailwind CSS**, and **MongoDB Atlas**, deployed to Vercel ([esp-32-16-ch-cloud-chaser.vercel.app](https://esp-32-16-ch-cloud-chaser.vercel.app)).

Control all 16 relays, 25 non-blocking chase effects, 5 curated presets, and live timing from either the local physical network or anywhere in the world with **seamless two-way real-time state synchronization**.

---

## ⚡ Key Highlights

- **Dual-Control Architecture (Local + Cloud Everywhere)**:
  - **Local Hotspot (SoftAP)**: Connect to `ESP32_Relay` (Password: `12345678`) → `http://192.168.4.1`.
  - **Local Home Network (Station)**: Auto-connects to your router (`loki`) → `http://192.168.1.37`.
  - **Remote Cloud Controller**: Hosted on Vercel backed by MongoDB Atlas → control from any browser or smartphone worldwide.
- **Next.js 16 & React 19 Web App (`web/`)**:
  - Latest **Next.js 16.4.3 with Turbopack** and **React 19.3.1**.
  - Serverless HTTPS REST APIs (`/api/esp/status`, `/api/esp/poll`, `/api/device/[deviceId]/command`).
  - Real-time Server-Sent Events (SSE) telemetry stream for instantaneous UI updates.
  - Interactive 4×4 manual relay matrix, velocity sliders, preset selectors, and diagnostics.
- **25 Fully Non-Blocking Lighting Effects**:
  - 100% non-blocking timing powered by hardware `millis()` state machine — zero `delay()` calls.
  - Left/Right chaser, bounce, center-out, zip-unzip, binary counter, sparkle, heartbeat, cylon eye, fireworks, meteor, strobe, cross-fade, and stack build.
- **5 Curated Lighting Presets**:
  1. 🌟 **Grand Showcase** — Sequentially cycles through all 25 native lighting effects.
  2. ⚡ **High Energy & Strobes** — Double dot, sparkles, heartbeat pulses, theater and strobe flashes.
  3. 🌊 **Waves & Smooth Chasers** — Smooth progressive sweeps, cylon scanner, and 5-channel Knight Rider.
  4. 🌧️ **Atmospheric & Fireworks** — Cascading waterfalls, meteor showers, raindrops, and fireworks.
  5. 🎭 **Matrix & Dual Bounce** — Dual converging dots, binary counter, cross-fades, and alternating odd/even.
- **Glitch-Free Active-LOW Safe Boot**:
  - 3-step GPIO initialization (`INPUT_PULLUP` → latch `HIGH` → `OUTPUT`) guarantees zero relay chattering or spurious clicks during power-up or reboot.
- **Hardware Watchdog**:
  - 30-second hardware watchdog timer (`esp_task_wdt`) prevents firmware hangs.

---

## 🔌 Hardware Wiring Diagram

```
 [12V DC Power Adapter]
     (+) ────────────────────────► DC+ on Relay Board (Coil Power)
     (-) ──────────────┬─────────► DC- on Relay Board
                       │
 [ESP32 Dev Module]    └─────────► GND on Relay Board (Common Ground)
     GND ────────────────────────► GND on Relay Board Signal Header
     3.3V ───────────────────────► VCC on Relay Board Signal Header
       (IMPORTANT: Remove JD1 jumper on the relay board first!)

     GPIO 25 ────────────────────► IN1  (Relay 1)
     GPIO  4 ────────────────────► IN2  (Relay 2)
     GPIO  5 ────────────────────► IN3  (Relay 3)
     GPIO 26 ────────────────────► IN4  (Relay 4)
     GPIO 13 ────────────────────► IN5  (Relay 5)
     GPIO 27 ────────────────────► IN6  (Relay 6)
     GPIO 15 ────────────────────► IN7  (Relay 7)
     GPIO 32 ────────────────────► IN8  (Relay 8)
     GPIO 16 ────────────────────► IN9  (Relay 9)
     GPIO 17 ────────────────────► IN10 (Relay 10)
     GPIO 18 ────────────────────► IN11 (Relay 11)
     GPIO 19 ────────────────────► IN12 (Relay 12)
     GPIO 21 ────────────────────► IN13 (Relay 13)
     GPIO 22 ────────────────────► IN14 (Relay 14)
     GPIO 23 ────────────────────► IN15 (Relay 15)
     GPIO 33 ────────────────────► IN16 (Relay 16)
```

> ⚠️ **CRITICAL POWER INSTRUCTIONS**:
> 1. Power relay coils with **12V DC** on `DC+` and `DC-`.
> 2. **Remove the JD1 jumper** to isolate 12V coil power from 3.3V ESP32 logic.
> 3. Connect ESP32 `3.3V` to `VCC` on the relay board input header.
> 4. Power the ESP32 via its micro-USB / Type-C port. Never feed 12V into the ESP32.

---

## 📁 Repository Structure

```
ESP32-16CH-Cloud-Chaser/
├── ESP32_16CH_Relay/
│   ├── ESP32_16CH_Relay.ino     ← Main Arduino sketch (25 effects, dual WiFi, cloud HTTP client)
│   └── web_page.h               ← Embedded HTML/CSS/JS cyberpunk web panel in PROGMEM
├── web/                         ← Next.js 16 Web Application (Turbopack + React 19 + Tailwind)
│   ├── src/
│   │   ├── app/                 ← App Router pages (/manual, /effects, /playlist, /settings)
│   │   │   └── api/             ← REST & SSE endpoints (/api/esp/*, /api/device/*)
│   │   ├── components/          ← UI components (RelayGrid, SpeedSlider, PresetCards)
│   │   ├── context/             ← DeviceProvider (live hardware pairing & status state)
│   │   └── lib/                 ← MongoDB Atlas connection & effect definitions
│   ├── package.json             ← Next.js 16.4, React 19.3, Lucide, Tailwind, MongoDB
│   └── next.config.mjs          ← Next.js configuration
├── platformio.ini               ← PlatformIO build configuration
├── min_spiffs.csv               ← 3MB flash partition table
└── README.md                    ← Complete documentation
```

---

## 🚀 How to Flash Firmware

### Using PlatformIO (Recommended)
1. Plug the ESP32 into your PC via USB.
2. Build and upload with one command:
   ```bash
   platformio run --target upload
   ```
3. Open Serial Monitor at **115200 baud** to see Wi-Fi status and cloud synchronization:
   ```bash
   platformio device monitor
   ```

### Using Arduino IDE
1. Open [`ESP32_16CH_Relay/ESP32_16CH_Relay.ino`](ESP32_16CH_Relay/ESP32_16CH_Relay.ino).
2. Install `ArduinoJson` (v6.x) via Library Manager.
3. Select Board: **ESP32 Dev Module**, Partition Scheme: **Minimal SPIFFS (Large APPS with OTA)**.
4. Select your COM Port and click **Upload**.

---

## 💻 Running the Web Application

### Local Development
```bash
cd web
npm install
npm run dev
```
Open [http://localhost:3000](http://localhost:3000) in your browser.

### Production Build
```bash
cd web
npm run build
npm start
```

### Environment Variables (`web/.env.local`)
```env
NEXTAUTH_SECRET=your-secure-random-secret
NEXTAUTH_URL=http://localhost:3000
MONGODB_URI=mongodb+srv://user:pass@cluster.mongodb.net/?retryWrites=true&w=majority
MONGODB_DB=relay_chaser
ADMIN_EMAIL=admin@relaychaser.local
ADMIN_PASSWORD_HASH=$2b$10$...
NEXT_PUBLIC_APP_URL=http://localhost:3000
```

---

## 🌐 Web Panel Controls & APIs

### 1. Local Panel Endpoints (Port 80)
The embedded web server responds directly on `http://192.168.4.1` and `http://192.168.1.37`:

| Method | Endpoint | Query / Body | Description |
|---|---|---|---|
| `GET` | `/state` | None | Full JSON state (relays, effect, speed, mode, IPs) |
| `GET` | `/preset` | `?id=0..4` | Switch to preset playlist (0=Showcase, 1=Energy, etc.) |
| `GET` | `/effect` | `?id=0..24` | Jump directly to effect ID (0 to 24) |
| `GET` | `/relay` | `?ch=0..15&v=0\|1` | Toggle individual relay ON (1) or OFF (0) |
| `GET` | `/speed` | `?v=20..500` | Set pattern speed in milliseconds |
| `GET` | `/cycle` | `?v=2000..30000` | Set auto-cycle duration in milliseconds |
| `GET` | `/cmd` | `?a=all_on` | Master ON: Energize all 16 relays |
| `GET` | `/cmd` | `?a=all_off` | Master OFF: De-energize all 16 relays |
| `GET` | `/cmd` | `?a=toggle_run` | Toggle Pause / Resume |
| `GET` | `/cmd` | `?a=toggle_auto` | Toggle Auto cycling vs Single loop |
| `GET` | `/cmd` | `?a=toggle_manual`| Toggle Manual relay override mode |
| `GET` | `/cmd` | `?a=next` / `?a=prev` | Advance or go back in effect playlist |
| `GET` | `/cmd` | `?a=odd` / `?a=even` / `?a=invert` | Relay pattern tools |

### 2. Cloud Serverless APIs (Next.js + MongoDB)
Endpoints hosted on [esp-32-16-ch-cloud-chaser.vercel.app](https://esp-32-16-ch-cloud-chaser.vercel.app):

- `POST /api/esp/status` — Pushed by ESP32 every 2.5s (or on state change) to update MongoDB.
- `GET /api/esp/poll` — Polled by ESP32 every 1.2s to retrieve and execute queued cloud commands.
- `POST /api/device/[deviceId]/command` — Queues remote commands (`SET_PRESET`, `SET_RELAY`, `MASTER_ON`, `MASTER_OFF`, `SET_SPEED`, `SET_EFFECT`).
- `GET /api/device/[deviceId]/stream` — Live Server-Sent Events (SSE) telemetry stream for the web UI.
