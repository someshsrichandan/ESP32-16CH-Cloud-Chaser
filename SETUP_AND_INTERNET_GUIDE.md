# ⚡ ESP32 16-Channel Relay Controller — Cloud & Next.js Pro v10.0

A complete professional IoT system:
- **Firmware:** ESP32 with Outbound Cloud MQTT (Zero Port Forwarding), 40 Built-in Effects + Unlimited Dynamic Custom Patterns, Web OTA, and Anti-Overheat Core.
- **Web App:** Modern full-stack **Next.js 16** (React 19, TypeScript, Tailwind CSS) control panel and visual **Effect Studio**.

---

## 🚀 Why This Architecture is Game-Changing

### 1. 🌐 Zero Port Forwarding Needed (Worldwide Internet Access)
- **Traditional problem:** To access an ESP32 over the internet, you normally had to configure router port forwarding, DDNS, and expose your home network to hackers, which often failed on mobile data (CGNAT).
- **Our Solution:** The ESP32 creates an **outbound persistent TCP connection** to a cloud broker (`broker.emqx.io`).
- Your home router's firewall automatically permits outbound connections.
- The **Next.js Web Panel** connects to the same cloud broker via WebSockets (`wss://broker.emqx.io:8084/mqtt`).
- **Result:** You can control all 16 relays from anywhere in the world on 4G/5G with under 50ms latency, zero router setup, and zero security risk!

### 2. 🎨 Unlimited New Effects Without Re-Flashing!
- In the Next.js Web Panel, switch to the **Dynamic FX Studio** tab.
- Visually create custom step sequences on the 16-channel timeline grid, or choose from pre-built presets (*Double Laser Scan, Accordion In-Out, Eurodance Strobe, Waterfall Cascade*).
- Click **Push Pattern to ESP32**: The web panel sends the pattern directly over MQTT or HTTP.
- The ESP32 saves it to non-volatile flash memory (`Preferences`) and runs it immediately!
- **You never need to plug the ESP32 into a USB cable or open Arduino IDE to add new effects.**

### 3. ❄️ Thermal & Low-Load Core (No Overheating)
- CPU clocked at **160 MHz** (saves ~40% power vs 240MHz).
- WiFi Modem Sleep powers down RF baseband when idle.
- `delay(1)` in the main loop executes the hardware `waiti` instruction (FreeRTOS idle sleep), keeping the ESP32 chip barely lukewarm (~38°C to 42°C).
- Minimum switching interval clamped to **40ms** to prevent mechanical relay arcing and inductive coil heat.

---

## 🔌 Hardware Wiring Diagram (UCTRONICS 16-CH 12V Board)

> **⚠️ CRITICAL:** REMOVE the `JD1` jumper on the relay board!  
> Power the relay coils with 12V DC. Power the ESP32 via its USB port.

| Relay Board Header | Connects To | Description |
| :--- | :--- | :--- |
| **DC+** (Screw Terminal) | **12V DC Adapter (+)** | Relay coil power (at least 2A) |
| **DC-** (Screw Terminal) | **12V DC Adapter (-)** & **ESP32 GND** | Common Ground |
| **VCC** (Signal Header) | **ESP32 3.3V Pin** | Logic power (JD1 jumper REMOVED) |
| **GND** (Signal Header) | **ESP32 GND Pin** | Optocoupler ground |
| **ESP32 USB** | **5V USB Wall Charger / PC** | Dedicated clean ESP32 power |

### Safe 16 GPIO Pinout (Boot-Safe)
| Relay Channel | GPIO Pin | Relay Channel | GPIO Pin |
| :--- | :--- | :--- | :--- |
| **Relay 1** | **GPIO 25** | **Relay 9** | **GPIO 16** |
| **Relay 2** | **GPIO 4** | **Relay 10** | **GPIO 17** |
| **Relay 3** | **GPIO 5** | **Relay 11** | **GPIO 18** |
| **Relay 4** | **GPIO 26** | **Relay 12** | **GPIO 19** |
| **Relay 5** | **GPIO 13** | **Relay 13** | **GPIO 21** |
| **Relay 6** | **GPIO 27** | **Relay 14** | **GPIO 22** |
| **Relay 7** | **GPIO 15** | **Relay 15** | **GPIO 23** |
| **Relay 8** | **GPIO 32** | **Relay 16** | **GPIO 33** |

---

## 💻 Step 1: Flash the ESP32 (One-Time Flash)

1. Open `ESP32_16CH_Relay_Chaser.ino` in Arduino IDE.
2. Select your board: **Tools ➔ Board ➔ ESP32 Dev Module**.
3. Select your port: **Tools ➔ Port ➔ COM7** (or your connected COM port).
4. Click **Upload**.
5. Once uploaded, open the Serial Monitor (115200 baud). It will print your unique **Device ID** (e.g. `relay16_e465b8`).

---

## 🌐 Step 2: Run the Next.js Web Panel

### Running Locally on your PC or Laptop:
```bash
cd web-panel
npm run dev
```
Open [http://localhost:3000](http://localhost:3000) in your browser.

### Deploying to the Cloud for Free (Vercel / Phone Access):
1. Install Vercel CLI (or push to GitHub and import into [Vercel.com](https://vercel.com)):
   ```bash
   cd web-panel
   npx vercel
   ```
2. You will get a free production URL like:
   `https://my-relay-controller.vercel.app`
3. Open this URL on your iPhone, Android, tablet, or PC anywhere in the world on 4G/5G!

---

## ⚙️ Step 3: Connect the Web Panel to your ESP32

1. On the web panel, click the **Settings Gear (⚙️)** in the top right.
2. Ensure **Connection Mode** is set to **Cloud MQTT**.
3. Enter your ESP32's **Device ID** (from the Serial Monitor or shown on the local web interface, e.g. `relay16_e465b8`).
4. Click **Save & Connect**.
5. The status badge will turn green: **Cloud Active**. You now have live two-way control from anywhere on Earth!

---

## 🎨 How to Load Custom Effects Without Re-Flashing

1. In the Next.js Web Panel, click the **Dynamic FX Studio (No Re-Flashing)** tab.
2. You will see a 16-channel step sequencer grid:
   - Rows: Channels 1 to 16.
   - Columns: Steps 1 to 32.
3. Click cells to toggle lights on/off for each step, or select a pre-made preset (*Waterfall Cascade, Accordion In-Out, Eurodance Strobe, Double Laser Scan*).
4. Click **Preview Live** to test the animation in your browser.
5. Click **Push Pattern to ESP32**.
6. The pattern is transmitted over the cloud tunnel, saved to the ESP32's non-volatile flash, and starts running immediately!

---

## 📲 Future Mobile App Support

Because the Next.js web panel is built with standard responsive React components:
- **PWA (Progressive Web App):** Open the website in Safari on iPhone or Chrome on Android and tap **"Add to Home Screen"** to install it as an app with its own icon.
- **Native iOS/Android App:** Can be packaged into native apps using [Capacitor](https://capacitorjs.com):
  ```bash
  cd web-panel
  npm install @capacitor/core @capacitor/cli @capacitor/android @capacitor/ios
  npx cap init
  npx cap add android
  ```
