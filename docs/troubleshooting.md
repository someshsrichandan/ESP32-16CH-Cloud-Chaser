# Troubleshooting Guide

> **v2.0 — Direct HTTP + MongoDB architecture.**
> This guide covers HTTP communication issues. There is no MQTT in this project.

---

## 1. ESP32 Won't Connect to Wi-Fi

**Symptom:** Serial Monitor shows `[WiFi] Connecting..........` forever.

**Causes & Fixes:**

| Cause | Fix |
|---|---|
| Wrong SSID or password | Erase NVS and re-run the setup wizard |
| 5GHz network | ESP32 only supports **2.4GHz**. Use 2.4GHz band |
| Router too far away | Move ESP32 closer to router |
| MAC filter on router | Add ESP32 MAC to router's allowlist |

**How to re-run the setup wizard:**
1. Open Serial Monitor
2. Send the text `RESET_CONFIG`
3. Device reboots and prompts for new credentials

**Check the SSID band:**
- In your router settings, ensure the 2.4GHz network has a different name from 5GHz
- Connect ESP32 to the 2.4GHz SSID specifically

---

## 2. ESP32 Can't Reach Vercel API

**Symptom:** Serial shows:
```
[HTTP] Status push FAILED (code: -1)
[HTTP] Poll FAILED (code: -1)
```

**Causes & Fixes:**

| Code | Meaning | Fix |
|---|---|---|
| `-1` | Connection failed | Check API base URL, ensure HTTPS prefix |
| `401` | Bad device secret | Delete and re-register device in web app |
| `404` | Wrong endpoint path | Check Vercel deployment is live |
| `500` | Server error | Check Vercel function logs |

**Check your API URL:**
- Must start with `https://` (not `http://`)
- No trailing slash: `https://your-app.vercel.app` ✅ not `https://your-app.vercel.app/` ❌
- Verify the URL works: open `https://your-app.vercel.app/api/esp/register` in browser — should return `{"error":"Unauthorized"}` (401), which means the API is running

**Verify Vercel deployment:**
- Go to Vercel dashboard → your project → latest deployment shows ✅

---

## 3. Device Shows Offline in Dashboard

**Symptom:** Dashboard shows 🔴 Offline even though ESP32 is running.

**How "online" is determined:**
A device is online if its `lastSeenAt` in MongoDB is **less than 10 seconds ago**.

**Fixes:**

1. Check ESP32 Serial Monitor for HTTP errors (step 2 above)
2. Check that `MONGODB_URI` is correctly set in Vercel env vars
3. Open Vercel → Deployments → Functions → `/api/esp/status` → check logs for errors
4. Verify MongoDB Network Access allows `0.0.0.0/0` (see [mongodb-setup.md](mongodb-setup.md))
5. Check if Atlas cluster is paused: Atlas dashboard → Resume cluster

---

## 4. Commands Not Reaching ESP32

**Symptom:** Clicking buttons in the web app has no effect. Serial Monitor shows no `[CMD]` messages.

**How commands work:**
Browser → POST `/api/device/{id}/command` → stored in MongoDB → ESP32 picks up on next GET `/api/esp/poll`

**Diagnose:**

1. Check Serial Monitor — ESP32 polls every 1 second:
   ```
   [HTTP] Poll OK — 1 commands pending
   [CMD] Dispatching: MASTER_ON
   ```
   If you see `0 commands` constantly, the command isn't reaching MongoDB.

2. Check device is registered with matching Device ID:
   - Web app Settings → device list shows your `esp32_xxxx`
   - Serial Monitor Device ID matches what's registered

3. Commands expire after **60 seconds** (TTL index). If ESP32 is offline, commands are lost after 60s.

4. Verify web app session: log out and log back in (session may have expired).

---

## 5. Relays Activating During Boot

**Symptom:** Some relays click ON when ESP32 is powered on or reset.

**Why this happens:**
GPIO 4, 5, and 15 have boot-strapping behavior. The relay board pulling these LOW can affect the boot mode.

**Software fix (already implemented):**
The firmware sets all relay pins `HIGH` (OFF) as the very **first instruction** in `setup()`, before any other initialization.

**Hardware fix (recommended for safety-critical loads):**
Add **10kΩ pull-up resistors** from each relay IN pin to 3.3V. This keeps the relay OFF during the brief period before the ESP32 GPIO is configured.

See [hardware-wiring.md](hardware-wiring.md) for the pull-up resistor wiring diagram.

---

## 6. Wrong Relay Activating

**Symptom:** Clicking "Relay 3" in the web app activates a different physical relay.

**Check the GPIO pin mapping:**

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

Verify your physical wiring matches this mapping exactly. Check `config.h`:
```cpp
const int RELAY_PINS[16] = {25, 4, 5, 26, 13, 27, 15, 32, 16, 17, 18, 19, 21, 22, 23, 33};
```

**Active-LOW logic:** The relay board uses Active-LOW. `LOW` = Relay energized (ON). `HIGH` = Relay de-energized (OFF). This is handled by the firmware — do not change it.

---

## 7. Web App Login Fails

**Symptom:** Login page shows "Invalid credentials" with correct email/password.

**Fixes:**

1. **Wrong password hash:** Regenerate:
   ```bash
   node -e "const b=require('bcryptjs'); console.log(b.hashSync('YourPassword', 10))"
   ```
   Paste the output into Vercel env var `ADMIN_PASSWORD_HASH`. Redeploy.

2. **NEXTAUTH_SECRET too short:** Must be at least 32 characters. Regenerate:
   ```bash
   openssl rand -base64 32
   ```

3. **NEXTAUTH_URL mismatch:** The value must exactly match your Vercel URL (no trailing slash, exact protocol):
   - ✅ `https://my-app.vercel.app`
   - ❌ `https://my-app.vercel.app/`
   - ❌ `http://my-app.vercel.app`

4. **After changing env vars:** Always **Redeploy** in Vercel dashboard (Deployments → ... → Redeploy).

---

## 8. MongoDB Connection Errors

**Symptom:** Vercel function logs show `MongoNetworkTimeoutError` or `Authentication failed`.

**Fixes:**

| Error | Cause | Fix |
|---|---|---|
| `Authentication failed` | Wrong username/password in URI | Re-copy URI from Atlas → Connect |
| `Connection timeout` | Network Access not set | Add `0.0.0.0/0` in Atlas Network Access |
| `MONGODB_URI not defined` | Env var missing | Add to Vercel env vars → Redeploy |
| `Cluster paused` | Free tier inactivity | Atlas dashboard → Resume cluster |

**Password special characters:** If your Atlas password contains `@`, `!`, `#`, `$`, etc., they must be URL-encoded in the URI:
- `@` → `%40`
- `!` → `%21`
- `#` → `%23`
- `$` → `%24`

Example: password `pass@word!` → URI: `...user:pass%40word%21@cluster...`

---

## 9. SSE Stream Disconnecting

**Symptom:** Dashboard status stops updating, then resumes every 10 seconds.

**Why this happens:**
Vercel Hobby plan limits serverless functions to **10 seconds**. The SSE stream reconnects automatically.

**This is expected behavior** — the `use-device-status` hook handles reconnection silently. You may see a brief "reconnecting" indicator.

**Fix (if it bothers you):**
- Upgrade to **Vercel Pro** for 60-second function timeout
- Or the hook falls back to polling mode automatically

---

## 10. Effects Not Cycling Correctly

**Symptom:** Effect switches before completing the configured number of cycles, or never switches.

**Expected behavior:**
The `CycleManager` counts complete effect cycles. It only advances to the next playlist entry **after** `N` full cycles are complete. A cycle = one complete pass of the pattern.

**Fixes:**
1. Verify cycle count is set to `> 0` in the playlist
2. Check Serial Monitor for `[EFFECT] Cycle complete` messages
3. Use the **Next Effect** button to manually advance if stuck
4. Check that effect speed is not set to 0ms (causes effect engine to stall)

---

## 11. High Heap Usage / Watchdog Reset

**Symptom:** ESP32 restarts randomly. Serial shows `Guru Meditation Error` or `Watchdog timeout`.

**Check heap in Serial Monitor:**
```
[DIAG] Free heap: 187432 bytes | Min heap: 156284 bytes
```

If free heap drops below **20,000 bytes**, there may be a memory issue.

**Fixes:**

| Symptom | Fix |
|---|---|
| Watchdog reset | HTTP request stuck — increase `HTTP_TIMEOUT_MS` in `config.h` |
| Heap shrinking | Avoid very rapid speed settings (< 20ms) |
| Random restarts | Check power supply — ESP32 needs stable 3.3V/500mA |
| Stack overflow | Rebuild firmware with `CORE_DEBUG_LEVEL=3` to see full trace |

The firmware uses `StaticJsonDocument` and char arrays to avoid heap fragmentation. If issues persist, check `http_manager.cpp` buffer sizes.

---

## 12. Build Errors (`npm run build` fails)

**Symptom:** Vercel deployment fails with build errors.

**Fixes:**

1. **TypeScript errors:**
   ```bash
   cd web && npx tsc --noEmit
   ```
   Fix any errors shown.

2. **Missing env vars at build time:**
   Some Next.js pages try to access env vars during build. Ensure `MONGODB_URI` and `NEXTAUTH_SECRET` are set in Vercel.

3. **Module not found:**
   ```bash
   cd web && npm install
   ```
   Then commit `package-lock.json`.

4. **Import of removed modules (`mqtt`, `redis`):**
   These are removed in v2.0. If you see import errors for these, check for any file still importing them. They should have empty export `{}` stubs.

---

## 13. Device ID Changed After Re-flash

**Symptom:** After re-flashing, the device has a different Device ID and can't authenticate.

**Why:**
- **Normal re-flash** (Upload in PlatformIO): Device ID stays the same (NVS preserved)
- **Full chip erase** (Erase Flash in PlatformIO): NVS is wiped, device secret regenerates

**Fix after full chip erase:**
1. Boot the ESP32 and run through the setup wizard again
2. Note the new Device ID (same as before — MAC-based)
3. In web app Settings → delete the old device record
4. Register the device again with the same Device ID
5. The new device secret will be stored in MongoDB during re-registration

> [!NOTE]
> The Device ID itself (based on MAC address) does NOT change on erase. Only the device secret regenerates.

---

## Quick Diagnostic Checklist

If something is wrong, work through this list:

- [ ] ESP32 Serial Monitor shows `[BOOT] Ready. Device ID: esp32_xxxx`
- [ ] Serial shows `[HTTP] Status pushed OK (200)` every 2 seconds
- [ ] Serial shows `[HTTP] Poll OK` every 1 second
- [ ] Web app Dashboard shows device as 🟢 Online
- [ ] Device ID in web app Settings matches Serial Monitor output
- [ ] MongoDB Atlas cluster is running (not paused)
- [ ] MongoDB Network Access has `0.0.0.0/0` allowlisted
- [ ] Vercel env vars: `MONGODB_URI`, `MONGODB_DB`, `NEXTAUTH_SECRET`, `NEXTAUTH_URL` all set
- [ ] All relays initialized OFF on boot (no unexpected clicking sounds)

---

## Getting More Help

Check Vercel function logs:
- Vercel dashboard → Project → Deployments → click latest → Functions tab
- Click on `/api/esp/status` or `/api/device/[deviceId]/command` to see live logs

Check ESP32 Serial Monitor with increased debug level:
- In `platformio.ini` set `build_flags = -DCORE_DEBUG_LEVEL=3`
- Re-upload and check Serial Monitor for verbose output
