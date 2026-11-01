#pragma once
#include <Arduino.h>

// ─── Firmware Version ──────────────────────────────────────────────
#define FIRMWARE_VERSION    "2.0.0-http"
#define DEVICE_NAME_DEFAULT "ESP32-RelayChaser"

// ─── GPIO Pin Mapping (FIXED - DO NOT CHANGE) ──────────────────────
const int RELAY_PINS[16] = {25, 4, 5, 26, 13, 27, 15, 32, 16, 17, 18, 19, 21, 22, 23, 33};
#define NUM_RELAYS 16

// ─── Active-LOW Logic ──────────────────────────────────────────────
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ─── HTTP API Configuration ────────────────────────────────────────
// These are stored in NVS and loaded at boot. Defaults shown here.
#define DEFAULT_API_BASE_URL      "https://your-app.vercel.app"
#define DEFAULT_API_STATUS_PATH   "/api/esp/status"
#define DEFAULT_API_POLL_PATH     "/api/esp/poll"
#define DEFAULT_API_REGISTER_PATH "/api/esp/register"

// ─── Timing ────────────────────────────────────────────────────────
#define STATUS_PUSH_INTERVAL_MS   2000   // Push status to server every 2s
#define COMMAND_POLL_INTERVAL_MS  1000   // Poll for commands every 1s
#define HEARTBEAT_INTERVAL_MS     5000   // Heartbeat log every 5s
#define HTTP_TIMEOUT_MS           8000   // HTTP request timeout
#define WATCHDOG_TIMEOUT_MS       30000  // Hardware watchdog

// ─── Speed Presets ─────────────────────────────────────────────────
#define SPEED_VERY_SLOW   1000
#define SPEED_SLOW         500
#define SPEED_NORMAL       200
#define SPEED_FAST         100
#define SPEED_VERY_FAST     50
#define SPEED_MIN           20
#define SPEED_MAX         2000

// ─── Compatibility Aliases for Unchanged Subsystem Modules ──────────
#define FIRMWARE_NAME        DEVICE_NAME_DEFAULT
#define RELAY_COUNT          NUM_RELAYS
#define RELAY_ACTIVE_LEVEL   RELAY_ON
#define RELAY_INACTIVE_LEVEL RELAY_OFF

#define SPEED_VERY_SLOW_MS   SPEED_VERY_SLOW
#define SPEED_SLOW_MS        SPEED_SLOW
#define SPEED_NORMAL_MS      SPEED_NORMAL
#define SPEED_FAST_MS        SPEED_FAST
#define SPEED_VERY_FAST_MS   SPEED_VERY_FAST
#define SPEED_MIN_MS         SPEED_MIN
#define SPEED_MAX_MS         SPEED_MAX

#define MAX_SSID_LEN         64
#define MAX_PASSWORD_LEN     64
#define MAX_DEVICE_ID_LEN    32
#define MAX_PLAYLIST_ENTRIES 32
#define DEFAULT_CYCLES_PER_EFFECT 3
#define DIAGNOSTICS_PRINT_INTERVAL_MS 30000
#define WIFI_RECONNECT_INITIAL_MS     1000
#define WIFI_RECONNECT_MAX_MS         30000
