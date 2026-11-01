# Device Registration Guide

> **v2.0 — After flashing, register your ESP32 in the web app using its Device ID.**

---

## Overview

The registration process is simple:

1. Flash ESP32 → get Device ID from Serial Monitor
2. Log into web app → register the Device ID
3. ESP32 appears online on Dashboard within 10 seconds

---

## Step 1 — Get Your Device ID

Flash the firmware and open Serial Monitor (see [firmware-flashing.md](firmware-flashing.md)).

On boot, the device prints its ID:
```
[BOOT] Ready. Device ID: esp32_a1b2c3d4
```

The Device ID format is `esp32_` followed by 8 hex characters derived from your ESP32's MAC address. **It never changes** as long as you don't erase the full flash.

> [!TIP]
> The Device ID is also printed every time the ESP32 boots. If you miss it, just power-cycle the board and check Serial Monitor.

---

## Step 2 — Log Into the Web App

Open your Vercel URL (e.g. `https://your-app.vercel.app`).

Log in with your admin credentials:
- **Email:** value of `ADMIN_EMAIL` environment variable
- **Password:** the plain-text password you used to generate `ADMIN_PASSWORD_HASH`

Default for local dev: `admin@relaychaser.local` / `admin`

---

## Step 3 — Register the Device

1. Navigate to **Settings** in the sidebar
2. Find the **Devices** section
3. Click **Register New Device**
4. Enter:
   - **Device ID:** `esp32_a1b2c3d4` (from Serial Monitor)
   - **Device Name:** anything descriptive, e.g. `Living Room Chaser`
5. Click **Register**

The device record is created in MongoDB. The ESP32 will complete full registration on its next boot or re-registration HTTP call.

---

## Step 4 — Verify Device is Online

Navigate to the **Dashboard**.

Within **10 seconds** of the ESP32 pushing its first status, the device will show:
- 🟢 **Online** indicator (green dot)
- Device Name
- IP address
- Wi-Fi signal strength
- Current mode (Manual / Auto / Stopped)
- 16-relay status grid

If the device doesn't appear online after 30 seconds, see [troubleshooting.md](troubleshooting.md).

---

## Step 5 — Test All 16 Channels

Go to **Manual Control** in the sidebar.

You will see a 4×4 grid of 16 relay buttons.

Test each relay:
1. Click **Channel 1** button → relay 1 should click ON
2. The button glows cyan when ON
3. Click again → relay clicks OFF
4. Repeat for all 16 channels
5. Try **Master ON** → all 16 relays click ON simultaneously
6. Try **Master OFF** → all 16 off

If a relay does not respond, check:
- The Serial Monitor for `[CMD] SET_RELAY` messages
- The wiring for that channel (see [hardware-wiring.md](hardware-wiring.md))

---

## Step 6 — Test Automatic Effects

1. Go to **Effects** in the sidebar
2. Click **Set Effect** on "Knight Rider"
3. Go back to Dashboard and click **Switch to Auto Mode**
4. The 16 relays should chase back and forth in the classic Knight Rider pattern
5. Try other effects: Waterfall, Sequential Forward, Strobe

---

## Step 7 — Test Playlist Cycling

1. Go to **Playlist** in the sidebar
2. Click **Add Effect** → select "Sequential Forward Chase" → set cycles to **2**
3. Click **Add Effect** → select "Knight Rider" → set cycles to **3**
4. Click **Add Effect** → select "Waterfall" → set cycles to **2**
5. Click **Start Playlist**

Expected behavior:
- Sequential Forward Chase runs **2 complete passes** across all 16 relays
- Then Knight Rider runs **3 complete back-and-forth passes**
- Then Waterfall runs **2 complete fill-and-drain cycles**
- Then loops back to Sequential Forward Chase

The Dashboard shows current effect name, cycle X of Y, and next effect.

---

## Multiple Devices

To add a second ESP32:
1. Flash the firmware on the second device
2. Note its Device ID (different MAC = different ID)
3. Register it in Settings with a different name
4. Both devices appear in the device list
5. Switch between them using the device selector in the sidebar

---

## Device ID Explained

The Device ID is derived from the last 4 bytes of the ESP32's MAC address:
```
MAC:       AA:BB:CC:DD:E1:F2
Device ID: esp32_dde1f2  (last 3 bytes → 6 hex chars)
           OR
           esp32_aabbccdd  (4 bytes → 8 hex chars)
```

This means:
- The Device ID is permanent and tied to the hardware
- Re-flashing firmware does NOT change the Device ID
- Full flash erase may regenerate the device secret (see troubleshooting)

---

## Device Secret Explained

Each ESP32 generates a **32-character hex secret** on first boot using the ESP32's hardware random number generator. This secret is:

- Stored in NVS flash on the ESP32
- Also stored in the MongoDB `devices` collection during registration
- Sent as the `X-Device-Secret` header on every HTTP request
- Never displayed in the web UI
- Used to authenticate all ESP32 → Vercel communications

If the secret is regenerated (after full flash erase), you must delete and re-register the device in the web app.

---

## Next Steps

- [Troubleshooting](troubleshooting.md)
- [Hardware Wiring](hardware-wiring.md)
