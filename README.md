# ⚡ ESP32-16CH-Cloud-Chaser

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue.svg)](https://www.espressif.com/)
[![Next.js 16](https://img.shields.io/badge/Frontend-Next.js%2016%20(React%2019)-black.svg)](https://nextjs.org/)
[![MQTT](https://img.shields.io/badge/Protocol-Cloud%20MQTT%20(WSS)-red.svg)](https://www.emqx.com/)
[![Tailwind CSS](https://img.shields.io/badge/Styling-Tailwind%20CSS%20v4-cyan.svg)](https://tailwindcss.com/)

An enterprise-grade, cloud-connected **16-Channel Relay Light Controller & Dynamic Chaser System**.

Engineered for **UCTRONICS 16-Channel 12V Relay Boards**, **Logic Level Shifters (3.3V to 5V)**, and **ESP32**, controlled worldwide through a modern **Next.js 16 Web Dashboard** with **Zero Port Forwarding** and **Zero Re-Flashing** needed to create new effects.

---

## 🌟 Highlights

* 🌐 **Worldwide Cloud Control (Zero Port Forwarding):** Outbound HiveMQ MQTT TLS/WebSockets tunnel connects your phone on 4G/5G directly to your home ESP32 without opening router ports or exposing IP addresses.
* 🔁 **Cycle Repeats Mode (Zero Delay / Zero Stutter):** Configurable repeat cycles (1x, 2x, 3x, 4x, 5x, 10x) per effect. The ESP32 runs each effect for N complete cycles, then transitions to the next effect with zero pause!
* ⚡ **Dual-Core FreeRTOS Isolation:** Core 1 is 100% dedicated to microsecond-precise relay switching. Core 0 handles all WiFi, HTTP, and MQTT in the background, guaranteeing zero jitter or stutter even during network drops.
* 🔋 **Instant Auto-Startup on Power Loss:** On power cut and restoration, the ESP32 latches relays safely, restores saved settings from NVS flash, and immediately begins running the chaser in under 0.2 seconds!
* 🎨 **Dynamic FX Studio (No Re-Flashing Needed):** Design, preview, and push unlimited custom step sequences live from your web browser directly into the ESP32's non-volatile flash memory!
* ⚡ **3-Stage Hardware Isolation:** `ESP32 (3.3V)` ➔ `Logic Level Shifter (3.3V ➔ 5V)` ➔ `16-CH Optocoupler Relay Board (12V Coil)`. Eliminates relay chattering, weak drive currents, and inductive noise.
* ❄️ **Anti-Overheating & Cool-Run Core:** Dynamic CPU clock scaling to 160MHz, WiFi modem sleep, and FreeRTOS idle task sleep (`waiti 0`) keeping the chip cool (~38°C to 42°C).
* 🎆 **40 Built-in Stage Effects:** Cylon Scanner, Knight Rider, Meteor Shower, Wave Fill, Heartbeat, Dominos, Theater Marquee, Raindrop Ripple, Fireworks, and more.
* 🔄 **Continuous Auto-Cycle Mode:** Orderly Sequential cycling or Random Shuffle with live countdown timers.
* 📲 **PWA & Mobile Ready:** Responsive dark cyberpunk interface ready for 1-tap mobile installation or Capacitor native app wrapping.
* 🛡️ **Web OTA Firmware Updates:** Flash future `.bin` updates over-the-air at `http://<device-ip>/update` without USB cables.

---

## ❓ Does My ESP32 Need to Connect to WiFi?

**YES!** For full worldwide remote control:
* **WiFi Connected (Home Router / Mobile Hotspot):** The ESP32 connects outbound to the secure Cloud MQTT broker (`broker.hivemq.com`). This allows you to control all 16 relays from **anywhere in the world** using the Next.js web panel on your phone or laptop.
* **Offline Fallback (Access Point Mode):** Even if your home WiFi router is turned off, the ESP32 broadcasts its own fail-safe hotspot:
  * **SSID:** `ESP32_Relay_Chaser`
  * **Password:** `12345678`
  * **Local URL:** `http://192.168.4.1`

---

## 📐 Complete 3-Stage Hardware Connection Diagram

```
                       ┌────────────────────────┐
                       │  12V DC Power Adapter  │
                       │     (At least 2A)      │
                       └───┬────────────────┬───┘
                           │ (+) 12V        │ (-) GND
                           │                │
                           ▼                ▼
                     ┌──────────┐     ┌──────────┐
                     │ Relay DC+│     │ Relay DC-│ (Relay Board Coil Power)
                     └──────────┘     └────┬─────┘
                                           │
  ┌─────────────────────────┐              │
  │   5V USB Power Supply   │              │
  └───┬─────────────────┬───┘              │
      │ (+) 5V          │ (-) GND          │
      │                 │                  │
      ▼                 ▼                  │
┌───────────┐     ┌───────────┐            │
│ ESP32 VIN │     │ ESP32 GND ├────────────┼───────────┐
└─────┬─────┘     └─────┬─────┘            │           │
      │ 3.3V            │                  │           │
      ▼                 ▼                  ▼           │
┌───────────┐     ┌───────────┐      ┌───────────┐     │
│Shifter LV │     │Shifter GND│      │Shifter HV │     │
│  (3.3V)   │     │  (Ground) │      │   (5V)    │     │
└─────┬─────┘     └─────┬─────┘      └─────┬─────┘     │
      │                 │                  │           │
      │ 16 Channels     │                  │ 16 Channels│
      │ (3.3V Logic)    │                  │ (5V Logic)│
      ▼                 │                  ▼           │
┌───────────┐           │            ┌───────────┐     │
│ESP32 GPIOs│           │            │Relay IN1  │     │
│ (25..33)  │           │            │  through  │     │
└─────┬─────┘           │            │Relay IN16 │     │
      │                 │            └─────┬─────┘     │
      ▼                 │                  │           │
 ┌─────────┐            │                  ▼           ▼
 │ Shifter │            │            ┌───────────┬───────────┐
 │ LV1-LV16│            │            │ Relay VCC │ Relay GND │
 └────┬────┘            │            │   (5V)    │ (Ground)  │
      │                 │            └───────────┴───────────┘
      ▼                 │               (JD1 Jumper REMOVED!)
 ┌─────────┐            │
 │ Shifter ├────────────┘
 │ HV1-HV16│ ────────► [To Relay Board Inputs IN1..IN16]
 └─────────┘
```

---

## 🔌 Complete Pin-to-Pin Wiring Table

### 1. Power & Logic Reference Rails
| Source | Target | Voltage | Purpose |
| :--- | :--- | :--- | :--- |
| **12V Adapter (+)** | **Relay Board DC+** | 12V DC | Relay coil energizing power |
| **12V Adapter (-)** | **Common Ground** | 0V | Ground return |
| **5V Power (+)** | **ESP32 VIN** | 5V DC | Microcontroller main power |
| **5V Power (+)** | **Level Shifter HV** | 5V DC | High Voltage logic rail |
| **5V Power (+)** | **Relay Board VCC** | 5V DC | Optocoupler anode supply (**Remove JD1!**) |
| **ESP32 3V3 Pin** | **Level Shifter LV** | 3.3V DC | Low Voltage logic reference |
| **ESP32 3V3 Pin** | **Level Shifter OE** | 3.3V DC | Output Enable (pull HIGH) |
| **All Grounds** | **ESP32 GND + Shifter GND + Relay GND** | 0V | **Common Ground (Must be tied together)** |

---

### 2. Signal Routing: ESP32 ➔ Level Shifter ➔ Relay Board
Using two 8-channel bi-directional level shifters (e.g. TXS0108E):

| Relay Channel | ESP32 GPIO | Shifter 3.3V (LV) | Shifter 5.0V (HV) | Relay Board Header |
| :---: | :---: | :---: | :---: | :---: |
| **Relay 1** | **GPIO 25** | Shifter 1 - A1 | Shifter 1 - B1 | **IN1** |
| **Relay 2** | **GPIO 4** | Shifter 1 - A2 | Shifter 1 - B2 | **IN2** |
| **Relay 3** | **GPIO 5** | Shifter 1 - A3 | Shifter 1 - B3 | **IN3** |
| **Relay 4** | **GPIO 26** | Shifter 1 - A4 | Shifter 1 - B4 | **IN4** |
| **Relay 5** | **GPIO 13** | Shifter 1 - A5 | Shifter 1 - B5 | **IN5** |
| **Relay 6** | **GPIO 27** | Shifter 1 - A6 | Shifter 1 - B6 | **IN6** |
| **Relay 7** | **GPIO 15** | Shifter 1 - A7 | Shifter 1 - B7 | **IN7** |
| **Relay 8** | **GPIO 32** | Shifter 1 - A8 | Shifter 1 - B8 | **IN8** |
| **Relay 9** | **GPIO 16** | Shifter 2 - A1 | Shifter 2 - B1 | **IN9** |
| **Relay 10** | **GPIO 17** | Shifter 2 - A2 | Shifter 2 - B2 | **IN10** |
| **Relay 11** | **GPIO 18** | Shifter 2 - A3 | Shifter 2 - B3 | **IN11** |
| **Relay 12** | **GPIO 19** | Shifter 2 - A4 | Shifter 2 - B4 | **IN12** |
| **Relay 13** | **GPIO 21** | Shifter 2 - A5 | Shifter 2 - B5 | **IN13** |
| **Relay 14** | **GPIO 22** | Shifter 2 - A6 | Shifter 2 - B6 | **IN14** |
| **Relay 15** | **GPIO 23** | Shifter 2 - A7 | Shifter 2 - B7 | **IN15** |
| **Relay 16** | **GPIO 33** | Shifter 2 - A8 | Shifter 2 - B8 | **IN16** |

---

## 🛠️ Step 1: Flashing the ESP32 (One-Time Flash)

### Prerequisites in Arduino IDE 2.x:
1. **ESP32 Core:** Go to `Tools ➔ Board ➔ Boards Manager...`, search for `esp32` by Espressif Systems and install it.
2. **PubSubClient Library:** Go to `Tools ➔ Manage Libraries...`, search for `PubSubClient` by Nick O'Leary and install it.

### Flashing Steps:
1. Open [`ESP32_16CH_Relay_Chaser/ESP32_16CH_Relay_Chaser.ino`](ESP32_16CH_Relay_Chaser/ESP32_16CH_Relay_Chaser.ino).
2. Connect your ESP32 to your computer via USB.
3. Select your board: **Tools ➔ Board ➔ ESP32 Dev Module**.
4. Select your COM Port: **Tools ➔ Port ➔ COMx** (e.g. `COM7`).
5. Click **Upload (➔)**.
6. Open the **Serial Monitor** at **115200 baud**. You will see:
   ```text
   =======================================================
    ESP32 16-CH RELAY CONTROLLER v10.0 (CLOUD & NEXT.JS PRO)
   =======================================================
   [1] 16 GPIOs Latched OFF.
   [WiFi] AP: ESP32_Relay_Chaser (IP: 192.168.4.1)
   [Device] Device ID: relay16_xxxxxx
   [MQTT] Connecting to broker.hivemq.com:1883...
   =======================================================
   ```
7. Note down your unique **Device ID** (e.g. `relay16_e465b8`).

---

## 📶 Step 2: First-Time WiFi Pairing (How the ESP32 Connects to your Router)

Right after you flash the ESP32, you have **two easy ways** to connect it to your home WiFi:

### Method A: Phone Hotspot Portal (Zero-Code, Recommended)
1. **Power up the ESP32:** After flashing, the ESP32 starts its own WiFi setup network:
   * **WiFi Name (SSID):** `ESP32_Relay_Chaser`
   * **WiFi Password:** `12345678`
2. **Connect your Phone or Laptop:** Go to your phone's WiFi settings and connect to `ESP32_Relay_Chaser`.
3. **Open Setup Portal:** In your phone's browser, open:
   👉 **`http://192.168.4.1`**
4. **Select your Home WiFi:**
   * Scroll down to the **WiFi Setup** section at the bottom.
   * Tap **Scan** to see all available 2.4GHz home WiFi networks.
   * Select your home router SSID, enter your home WiFi password, and click **Save & Connect**.
5. **Connected Forever:** The ESP32 saves your WiFi credentials into internal flash memory (`Preferences` NVS). Even after you power off or unplug the ESP32, it will automatically reconnect to your home router in 3 seconds!

### Method B: Pre-Configure in Arduino IDE Before Uploading
If you prefer not to connect to the hotspot, you can pre-set your home WiFi directly at the top of [`ESP32_16CH_Relay_Chaser.ino`](ESP32_16CH_Relay_Chaser/ESP32_16CH_Relay_Chaser.ino):
```cpp
const char* DEFAULT_HOME_WIFI_SSID = "YourHomeWiFiName";
const char* DEFAULT_HOME_WIFI_PASS = "YourHomeWiFiPassword";
```
When you flash the ESP32, it connects to your home WiFi router immediately on the very first boot!

---

### 🔑 How the Next.js Web Panel Links to your ESP32:
* In the Next.js Web Panel, you **DO NOT** need to enter your WiFi password.
* Instead, your ESP32 has a unique **Device ID** (e.g. `relay16_e465b8`, shown on the Serial Monitor or on the setup page).
* In the Next.js Web Panel, click **Settings (⚙️)**, enter your **Device ID**, and click **Save & Connect**.
* That's it! The Web Panel and your ESP32 link automatically through the Cloud MQTT tunnel with **Zero Port Forwarding**!

---

## 💻 Step 3: Next.js 16 Web Dashboard Setup

### Running Locally:
```bash
# Navigate to web panel directory
cd web-panel

# Install dependencies (if not already installed)
npm install

# Start development server
npm run dev
```
Open **[http://localhost:3000](http://localhost:3000)** in your browser.

### Deploying Free to Vercel (Worldwide Access from your Phone):
1. Run:
   ```bash
   npx vercel
   ```
2. Follow the 1-click prompts.
3. You will receive a production URL (e.g. `https://esp32-chaser.vercel.app`).
4. Open the link on your mobile phone on 4G/5G from anywhere in the world!

---

## 🌐 Step 4: Connecting Web Panel to ESP32 (Cloud MQTT)

1. On the web dashboard, tap the **Settings Gear (⚙️)** in the top right.
2. Choose **Connection Mode: Cloud MQTT**.
3. Enter your ESP32's **Device ID** (printed on boot in Serial Monitor, e.g. `relay16_C8E720`).
4. Ensure **MQTT WebSocket Broker** is set to:
   * `wss://broker.hivemq.com:8884/mqtt` (Default HiveMQ Public WebSocket)
   * *(Note: If you paste `broker.hivemq.com:1883`, the web panel will automatically correct it to the WebSocket endpoint, as web browsers communicate with MQTT over WebSockets!)*
5. Click **Save & Connect**.
6. The connection badge will instantly light up green: **Cloud Active**!

---

## ⚡ Important: Eliminating Voltage Sag & Dim Glow on Relays

* **Hardware Power Supply:**
  * 16 relay coils energized simultaneously draw ~1.3A (at 5V) or ~0.6A (at 12V).
  * **Never** power the relay coils directly from the ESP32's 5V/3.3V pins or a weak 1A phone charger!
  * Connect a **dedicated 12V 2A (or 5V 2A) DC power adapter** directly to the relay board's `DC+` and `DC-` screw terminals.
  * Remove the **JD1 jumper** so the relay coil power is completely isolated from the ESP32 logic power.
* **Firmware Soft-Start Protection:**
  * The firmware includes a built-in **2ms micro-stagger soft-start** on `allOn()` and heavy frame masks. This spreads inductive coil inrush over 32ms, keeping your DC voltage rail completely flat and eliminating dim glowing!

---

## 🎨 Step 5: Creating Dynamic Patterns (No Re-Flashing!)

1. Switch to the **Dynamic FX Studio** tab.
2. Use the **16-Channel Step Sequencer** (Channels 1–16 vs Steps 1–32) or select a preset:
   * *Double Laser Scan*
   * *Accordion In-Out*
   * *Binary Arpeggio*
   * *Waterfall Cascade*
   * *Eurodance Strobe 140BPM*
3. Click **Preview Live** to simulate in browser.
4. Click **Push Pattern to ESP32**: The web app sends the pattern over Cloud MQTT. The ESP32 saves it to internal flash (`Preferences`) and runs it immediately. **Zero USB cable needed!**

---

## 📱 Future Mobile App Expansion
* **PWA (Progressive Web App):** Open the website in Safari (iOS) or Chrome (Android) and tap **"Add to Home Screen"** for a full-screen native experience.
* **Capacitor Mobile App:**
  ```bash
  cd web-panel
  npm install @capacitor/core @capacitor/cli @capacitor/android @capacitor/ios
  npx cap init
  npx cap add android
  ```

---

## 📄 License

MIT License. Designed with ❤️ for makers, IoT builders, and stage lighting enthusiasts.

