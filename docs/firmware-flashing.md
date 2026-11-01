# Firmware Flashing Guide

> **v2.0 — HTTP Edition. No MQTT. ESP32 connects directly to Vercel over HTTPS.**

---

## Prerequisites

| Tool | Download |
|---|---|
| VS Code | [code.visualstudio.com](https://code.visualstudio.com) |
| PlatformIO Extension | VS Code Extensions → search "PlatformIO IDE" |
| USB Cable | Data cable (not charge-only) |
| USB Driver | CH340: [wch-ic.com](http://www.wch-ic.com/downloads/CH341SER_EXE.html) · CP2102: [silabs.com](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |

---

## Step 1 — Open Project in VS Code

1. Open VS Code
2. **File → Open Folder**
3. Select: `c:\Users\somes\Desktop\esp-32\test\firmware\esp32_relay_chaser\`
4. PlatformIO will detect the project automatically
5. Wait for PlatformIO to finish indexing (bottom status bar shows progress)

---

## Step 2 — Understanding the Firmware Structure

| File | Purpose |
|---|---|
| `esp32_relay_chaser.ino` | Main entry — `setup()` and `loop()` |
| `config.h` | GPIO map, timing constants, speed presets, API paths |
| `gpio_controller.*` | All relay writes — centralized, Active-LOW |
| `effect_engine.*` | Non-blocking `millis()`-based effect runner |
| `effect_library.*` | All 30 chasing effects |
| `cycle_manager.*` | Playlist sequencer — cycles per effect |
| `speed_controller.*` | Speed presets (20ms–2000ms) |
| `wifi_manager.*` | Wi-Fi + exponential backoff reconnect |
| `http_manager.*` | **NEW v2.0** — HTTPS GET/POST to Vercel |
| `command_handler.*` | **NEW v2.0** — Dispatches 13 command types |
| `config_manager.*` | NVS persistence — Wi-Fi, API URL, device secret |
| `status_manager.*` | Builds and pushes JSON telemetry |
| `diagnostics.*` | Watchdog, heap monitor, uptime |
| `platformio.ini` | Build config — only `ArduinoJson` lib needed |

### platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    bblanchon/ArduinoJson @ ^6.21.3
monitor_speed = 115200
```

> [!NOTE]
> `HTTPClient` and `WiFiClientSecure` are **built into the ESP32 Arduino SDK** — no extra libraries needed for HTTP communication.

---

## Step 3 — Connect ESP32

1. Connect ESP32 to your PC via USB
2. In VS Code bottom bar, PlatformIO shows the detected COM port
3. If no port detected: install the USB driver for your board's chip (CH340 or CP2102)

---

## Step 4 — Build & Upload

1. In VS Code, click the **PlatformIO icon** in the left sidebar (alien head icon)
2. Under **esp32dev** → click **Upload**
3. Or press the → (right arrow) button in the bottom toolbar

PlatformIO will:
- Download ESP32 platform tools if first time (~200 MB, one-time)
- Compile all firmware files
- Flash to the ESP32

Upload takes about 30–60 seconds.

---

## Step 5 — First Boot Serial Wizard

After uploading, open **Serial Monitor**:
- PlatformIO sidebar → **Monitor** (or Ctrl+Alt+S)
- Baud rate: **115200**

On first boot (blank NVS), the wizard runs automatically:

```
ESP32 Relay Chaser v2.0 - HTTP Edition
========================================
[GPIO] All 16 relays initialized HIGH (OFF)
[CONFIG] First boot detected. Running setup wizard...

Enter WiFi SSID: █
```

Type your Wi-Fi network name and press Enter:
```
Enter WiFi SSID: YourNetwork2.4GHz
Enter WiFi Password: █
```

Type your Wi-Fi password and press Enter:
```
Enter WiFi Password: YourPassword
Enter API Base URL (e.g. https://your-app.vercel.app): █
```

Type your full Vercel URL (no trailing slash):
```
Enter API Base URL: https://esp32-relay-chaser.vercel.app
[CONFIG] Saving to NVS...
[CONFIG] Device ID:     esp32_a1b2c3d4
[CONFIG] Device Secret: 9f3ab21e...32chars...
[CONFIG] Setup complete!

[WiFi] Connecting to YourNetwork2.4GHz ...
[WiFi] Connected! IP: 192.168.1.105
[HTTP] Registering device with server...
[HTTP] Device registered OK (201)
[BOOT] Ready. Device ID: esp32_a1b2c3d4
[HTTP] Status pushed OK (200)
[HTTP] Poll OK — 0 commands pending
```

> [!IMPORTANT]
> **Copy the Device ID** (e.g. `esp32_a1b2c3d4`). You need it to register the device in the web app. It is also printed on every boot after the first.

---

## Step 6 — Normal Boot Output

On every subsequent boot:

```
ESP32 Relay Chaser v2.0 - HTTP Edition
========================================
[GPIO] All 16 relays initialized HIGH (OFF)
[CONFIG] Loaded from NVS: esp32_a1b2c3d4
[WiFi] Connecting to YourNetwork2.4GHz .
[WiFi] Connected! IP: 192.168.1.105
[HTTP] Re-registering device...
[HTTP] Device re-registered OK (200)
[BOOT] Ready. Device ID: esp32_a1b2c3d4
[HTTP] Status pushed OK (200)
[HTTP] Poll OK — 0 commands pending
[DIAG] Heap: 234512 bytes free
```

---

## Serial Log Prefix Reference

| Prefix | Meaning |
|---|---|
| `[GPIO]` | Relay initialization and writes |
| `[BOOT]` | Startup sequence milestones |
| `[CONFIG]` | NVS load/save and setup wizard |
| `[WiFi]` | Wi-Fi connection status |
| `[HTTP]` | HTTP request results |
| `[CMD]` | Received and dispatched commands |
| `[EFFECT]` | Effect engine events (start, cycle complete, switch) |
| `[DIAG]` | Heap, uptime, watchdog diagnostics |

---

## Re-configuring Credentials

If you need to change Wi-Fi network or Vercel URL after first boot:

**Option A — Erase NVS via Serial Monitor:**
Send the text `RESET_CONFIG` in Serial Monitor. The device will erase its NVS and reboot into the wizard.

**Option B — Full chip erase:**
PlatformIO sidebar → **Erase Flash** (under Platform). Then re-upload firmware. This runs the wizard again.

---

## GPIO Safety — Boot-Strapping Pins

> [!WARNING]
> GPIO 4, GPIO 5, and GPIO 15 have boot-strapping behavior on ESP32. These pins determine the boot mode. The relay board pulling them LOW during boot could cause issues.

The firmware sets ALL relays `HIGH` (OFF) as the **very first operation** in `setup()`, before Wi-Fi or HTTP initialization. However for safety-critical loads, add **10kΩ pull-up resistors** on the relay IN pins to 3.3V. See [hardware-wiring.md](hardware-wiring.md) for details.

---

## Offline Operation

> [!NOTE]
> If Wi-Fi or internet is lost, the ESP32 continues running the current automatic effect locally. HTTP requests are skipped when offline. When connectivity is restored, the device resumes pushing status and polling commands automatically.

---

## OTA Updates (Future)

OTA firmware update support is planned. For now, update firmware by:
1. Editing files in `firmware/esp32_relay_chaser/`
2. Re-uploading via PlatformIO USB

---

## Next Steps

- [Register your device in the web app](device-registration.md)
- [Troubleshooting](troubleshooting.md)
