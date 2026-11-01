// ============================================================
// ESP32 16-Channel Wi-Fi Relay Chaser — HTTP Edition v2.0
// Direct HTTPS REST communication to Vercel API & MongoDB
// Includes Local Web Configuration Portal (SoftAP 192.168.4.1)
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
#include "portal_manager.h"

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
    configManager.begin();
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

    // ── 7. Local Web Configuration Portal (SoftAP 192.168.4.1)
    portalManager.begin();

    // ── 8. Connect to Station Wi-Fi ──────────────────────────
    wifiManager.begin(configManager.getWifiSSID(), configManager.getWifiPassword());
    
    // Quick non-blocking connection wait window (up to 5 seconds)
    Serial.print("[WiFi] Connecting to '");
    Serial.print(configManager.getWifiSSID());
    Serial.println("'...");

    unsigned long wifiWaitStart = millis();
    while (!wifiManager.isConnected() && (millis() - wifiWaitStart < 5000)) {
        wifiManager.loop();
        portalManager.loop();
        delay(200);
        diagnostics.feedWatchdog();
    }

    if (wifiManager.isConnected()) {
        char ipBuf[32];
        wifiManager.getIP(ipBuf, sizeof(ipBuf));
        Serial.printf("[WiFi] Connected! Station IP: %s\n", ipBuf);
    } else {
        Serial.println("[WiFi] Not connected yet. Connect your phone to 'ESP32-Relay-Setup' at 192.168.4.1 to configure!");
    }

    // ── 9. HTTP Manager + Command Handler ────────────────────
    httpManager.begin();
    commandHandler.begin();
    httpManager.setCommandCallback(CommandHandler::handleCommand);

    // ── 10. Status Manager ───────────────────────────────────
    statusManager.begin();

    Serial.printf("[BOOT] Ready. Device ID: %s\n", configManager.getDeviceId());
}

void loop() {
    unsigned long now = millis();

    // ── Feed watchdog ─────────────────────────────────────────
    diagnostics.feedWatchdog();

    // ── Local Web Portal (192.168.4.1) ───────────────────────
    portalManager.loop();

    // ── Wi-Fi reconnect state machine ─────────────────────────
    wifiManager.loop();

    // ── Run effect engine (non-blocking) ─────────────────────
    effectEngine.update();
    cycleManager.update();

    // Only do cloud HTTP when Wi-Fi is connected
    if (wifiManager.isConnected()) {
        static unsigned long lastRegisterAttempt = 0;

        // Register device on connection (retry every 5 seconds if not yet registered)
        if (!deviceRegistered) {
            if (now - lastRegisterAttempt >= 5000) {
                lastRegisterAttempt = now;
                deviceRegistered = httpManager.registerDevice(
                    configManager.getDeviceId(),
                    configManager.getDeviceSecret(),
                    configManager.getDeviceName()
                );
                if (deviceRegistered) {
                    statusManager.pushStatusNow();
                }
            }
        } else {
            // ── Poll for commands every 1 second ─────────────────
            if (now - lastCommandPoll >= COMMAND_POLL_INTERVAL_MS) {
                lastCommandPoll = now;
                httpManager.pollCommands();
            }

            // ── Push status (pushes immediately if command changed state, or every 2s) ──
            statusManager.loop();
        }
    } else {
        // Reset registration state when Wi-Fi disconnects
        deviceRegistered = false;
    }

    // ── Diagnostics periodic print ───────────────────────────
    diagnostics.loop();

    // No delay() here — loop runs as fast as possible
}
