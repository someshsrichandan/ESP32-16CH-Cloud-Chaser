// ============================================================
// ESP32 16-Channel Wi-Fi Relay Chaser — HTTP Edition v2.0
// Direct HTTPS REST communication to Vercel API & MongoDB
// ============================================================
#include <Arduino.h>
#include "config.h"
#include "diagnostics.h"
#include "gpio_controller.h"
#include "effect_engine.h"
#include "effect_library.h"
#include "cycle_manager.h"
#include "speed_controller.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "http_manager.h"
#include "status_manager.h"
#include "command_handler.h"

// ─── Subsystem Global Instances ──────────────────────────────
GPIOController   gpioController;
EffectEngine     effectEngine;
SpeedController  speedController;
CycleManager     cycleManager;
WiFiManager      wifiManager;
Diagnostics      diagnostics;

// Timing for HTTP operations
static unsigned long lastStatusPush = 0;
static unsigned long lastCommandPoll = 0;
static bool deviceRegistered = false;

void setup() {
    Serial.begin(115200);
    delay(100);

    // ── 1. GPIO FIRST — all relays OFF before anything else ──
    gpioController.init();

    // ── 2. Diagnostics / Watchdog ────────────────────────────
    diagnostics.initWatchdog(WATCHDOG_TIMEOUT_MS);

    // ── 3. Load configuration from NVS ───────────────────────
    configManager.begin(); // may block for first-boot serial wizard
    configManager.printConfig();

    // ── 4. Speed controller ──────────────────────────────────
    speedController.init(&effectEngine);
    speedController.setCustomSpeed(configManager.getLastSpeed());

    // ── 5. Effect engine ─────────────────────────────────────
    effectEngine.init(&gpioController);
    effectEngine.setSpeed(speedController.getSpeedMs());
    effectEngine.setEffect(configManager.getLastEffect());

    // ── 6. Cycle manager ─────────────────────────────────────
    cycleManager.init(&effectEngine, &speedController);

    // ── 7. Wi-Fi ─────────────────────────────────────────────
    wifiManager.begin(configManager.getWifiSSID(), configManager.getWifiPassword());
    Serial.print("[WiFi] Connecting");
    while (!wifiManager.isConnected()) {
        wifiManager.loop();
        Serial.print(".");
        delay(500);
        diagnostics.feedWatchdog();
    }
    char ipBuf[32];
    wifiManager.getIP(ipBuf, sizeof(ipBuf));
    Serial.printf("\n[WiFi] Connected! IP: %s\n", ipBuf);

    // ── 8. HTTP Manager + Command Handler ────────────────────
    httpManager.begin();
    commandHandler.begin();
    httpManager.setCommandCallback(CommandHandler::handleCommand);

    // ── 9. Register device with server ───────────────────────
    Serial.println("[HTTP] Registering device...");
    deviceRegistered = httpManager.registerDevice(
        configManager.getDeviceId(),
        configManager.getDeviceSecret(),
        configManager.getDeviceName()
    );
    if (deviceRegistered) {
        Serial.println("[HTTP] Device registered OK");
    } else {
        Serial.println("[HTTP] Registration failed (will retry on poll)");
    }

    // ── 10. Status Manager ───────────────────────────────────
    statusManager.begin();
    statusManager.pushStatusNow(); // immediate first push

    Serial.printf("[BOOT] Ready. Device ID: %s\n", configManager.getDeviceId());
}

void loop() {
    unsigned long now = millis();

    // ── Feed watchdog ─────────────────────────────────────────
    diagnostics.feedWatchdog();

    // ── Wi-Fi reconnect ───────────────────────────────────────
    wifiManager.loop();

    // ── Run effect engine (non-blocking) ─────────────────────
    effectEngine.update();
    cycleManager.update();

    // Only do HTTP when Wi-Fi is connected
    if (wifiManager.isConnected()) {

        // ── Poll for commands every 1 second ─────────────────
        if (now - lastCommandPoll >= COMMAND_POLL_INTERVAL_MS) {
            lastCommandPoll = now;
            httpManager.pollCommands();
        }

        // ── Push status every 2 seconds ──────────────────────
        if (now - lastStatusPush >= STATUS_PUSH_INTERVAL_MS) {
            lastStatusPush = now;
            statusManager.loop();
        }
    }

    // ── Diagnostics periodic print ───────────────────────────
    diagnostics.loop();

    // No delay() here — loop runs as fast as possible
}
