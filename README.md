# ESP32 × 16-Channel Relay Controller PRO (v8.5)

A high-performance, standalone 16-channel relay chaser and lighting sequencer for the **ESP32** and **UCTRONICS 16-Channel Relay Module**. Features an embedded cyberpunk dark-mode web controller hosted directly on the ESP32, 25 non-blocking chase effects, 5 curated presets, dual WiFi mode (SoftAP + Home WiFi Station), and glitch-free active-LOW hardware switching.

No cloud brokers, no external servers, and zero delay timers — runs completely standalone or on your local WiFi network.

---

## ⚡ Key Highlights

- **Embedded Cyberpunk Web Panel**: Beautiful, dark-mode web GUI served directly from ESP32 PROGMEM flash memory on port 80.
- **Dual WiFi Mode (`WIFI_AP_STA`)**:
  - **Direct Access Point**: Connect to `ESP32_Relay` (Password: `12345678`) and open `http://192.168.4.1`.
  - **Local Home WiFi**: Connects to your router (`loki`) in the background so you can access the panel from any browser on your home network.
- **25 Non-Blocking Chase Effects**: Left/Right chaser, bounce, center-out, zip-unzip, binary counter, sparkle, heartbeat, cylon eye, fireworks, meteor, strobe, and stack build — all powered by non-blocking `millis()` state machines.
- **5 Curated Lighting Presets**:
  1. 🌟 **Grand Showcase** — Auto-cycles all 25 effects
  2. ⚡ **High Energy & Strobes** — Rapid flashes, sparkles & heartbeats
  3. 🌊 **Waves & Smooth Chasers** — Cylon scanners, fluid chases & bounces
  4. 🌧️ **Atmospheric & Fireworks** — Cascades, meteors, raindrops & explosions
  5. 🎭 **Matrix & Dual Bounce** — Alternating grids, counter-crossing dots & binary
- **Glitch-Free Active-LOW Safe Boot**: 3-step GPIO initialization (`INPUT_PULLUP` → latch `HIGH` → `OUTPUT`) prevents false relay clicks on boot.
- **Unified State Machine**: Smooth transitions between **Auto Run**, **Single FX Loop**, **Manual Relay Override**, and **Master OFF**.
- **Hardware Watchdog**: 30-second hardware watchdog protects against hangs.

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

## 🚀 How to Flash & Run

### Option 1: VS Code with PlatformIO (Recommended)
1. Plug the ESP32 into your computer via USB.
2. In VS Code Terminal, run:
   ```powershell
   platformio run --target upload
   ```
3. Open Serial Monitor at **115200 baud** to see connection details.

### Option 2: Arduino IDE
1. Open [`ESP32_16CH_Relay/ESP32_16CH_Relay.ino`](ESP32_16CH_Relay/ESP32_16CH_Relay.ino).
2. Select Board: **ESP32 Dev Module**.
3. Select your ESP32 COM Port.
4. Click **Upload**.

---

## 🌐 Web Panel Usage

### Connecting:
1. **Via Direct Hotspot (No Router Required)**:
   - Connect phone or PC to WiFi: `ESP32_Relay`
   - Password: `12345678`
   - Open browser: **`http://192.168.4.1`**
2. **Via Local Home WiFi**:
   - The ESP32 auto-connects to your WiFi (`loki`).
   - Open browser: **`http://<station-ip>`** (displayed in Serial Monitor and on the top banner).

### Controls Available:
- **Lighting Presets**: 1-click preset switching (Grand Showcase, High Energy, Waves, Fireworks, Matrix).
- **Master Controls**: All ON, All OFF, Pause / Resume, Auto ON/OFF, Next FX, Prev FX.
- **Relay Patterns**: Odd (1,3,5...), Even (2,4,6...), Invert Matrix, Manual Override.
- **Sliders**: Speed (20ms – 500ms) and Auto Cycle Time (2s – 30s).
- **Interactive 16-Channel Matrix**: Touch any channel (CH1 – CH16) to instantly toggle that relay.

---

## 📡 REST JSON API

The ESP32 responds with full CORS headers, allowing external automation, Home Assistant, or Python scripts to control it:

| Method | Endpoint | Query / Body | Description |
|---|---|---|---|
| `GET` | `/state` | None | Returns full JSON state (relays, effect, speed, mode, IPs) |
| `GET` | `/preset` | `?id=0..4` | Switch to preset playlist (0=Showcase, 1=Energy, 2=Waves, etc.) |
| `GET` | `/effect` | `?id=0..24` | Jump directly to effect ID (0 to 24) |
| `GET` | `/relay` | `?ch=0..15&v=0\|1` | Directly toggle relay channel (0-15) ON (1) or OFF (0) |
| `GET` | `/speed` | `?v=20..500` | Set pattern step speed in milliseconds |
| `GET` | `/cycle` | `?v=2000..30000` | Set auto-cycle duration in milliseconds |
| `GET` | `/cmd` | `?a=all_on` | Energize all 16 relays |
| `GET` | `/cmd` | `?a=all_off` | De-energize all 16 relays (Standby) |
| `GET` | `/cmd` | `?a=toggle_run` | Toggle Pause / Resume |
| `GET` | `/cmd` | `?a=toggle_auto` | Toggle Auto cycling vs Single loop |
| `GET` | `/cmd` | `?a=toggle_manual`| Toggle Manual relay override mode |
| `GET` | `/cmd` | `?a=next` / `?a=prev` | Advance or go back in effect playlist |
| `GET` | `/cmd` | `?a=odd` / `?a=even` / `?a=invert` | Relay pattern tools |

---

## 📁 Repository Structure

```
ESP32-16CH-Cloud-Chaser/
├── ESP32_16CH_Relay/
│   ├── ESP32_16CH_Relay.ino     ← Main Arduino sketch (25 effects, presets, state machine)
│   └── web_page.h               ← Embedded HTML/CSS/JS cyberpunk web panel in PROGMEM
├── platformio.ini               ← PlatformIO build configuration
├── min_spiffs.csv               ← 3MB flash partition table
└── README.md                    ← You are here
```
