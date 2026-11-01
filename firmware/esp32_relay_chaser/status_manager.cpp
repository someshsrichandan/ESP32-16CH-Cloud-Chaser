#include "status_manager.h"
#include "gpio_controller.h"
#include "effect_engine.h"
#include "effect_library.h"
#include "cycle_manager.h"
#include "speed_controller.h"
#include "wifi_manager.h"
#include "diagnostics.h"
#include "http_manager.h"
#include "config_manager.h"
#include "config.h"
#include <ArduinoJson.h>

extern GPIOController gpioController;
extern EffectEngine effectEngine;
extern SpeedController speedController;
extern CycleManager cycleManager;
extern WiFiManager wifiManager;
extern Diagnostics diagnostics;

static char s_statusPayload[768];

void StatusManager::begin() {
    _lastStatusMs = 0;
    _changed = false;
}

void StatusManager::markChanged() {
    _changed = true;
    if (wifiManager.isConnected()) {
        pushStatusNow();
    }
}

void StatusManager::pushStatusNow() {
    _changed = false;
    _lastStatusMs = millis();
    buildAndPush();
}

void StatusManager::loop() {
    unsigned long now = millis();

    // Check if relays changed state asynchronously
    if (gpioController.hasChanged()) {
        gpioController.clearChanged();
        _changed = true;
    }

    if (_changed || (now - _lastStatusMs >= STATUS_PUSH_INTERVAL_MS)) {
        pushStatusNow();
    }
}

void StatusManager::buildAndPush() {
    if (!wifiManager.isConnected()) {
        return;
    }

    StaticJsonDocument<768> doc;

    doc["deviceId"] = configManager.getDeviceId();
    doc["deviceName"] = configManager.getDeviceName();

    uint8_t modeVal = configManager.getLastMode();
    const char* modeStr = "auto";
    if (modeVal == 1) {
        modeStr = "manual";
    } else if (modeVal == 2) {
        modeStr = "stopped";
    }
    doc["mode"] = modeStr;

    // Relay states (16 channels)
    bool relayStates[NUM_RELAYS];
    gpioController.getRelayArray(relayStates);
    JsonArray relays = doc.createNestedArray("relays");
    for (int i = 0; i < NUM_RELAYS; i++) {
        relays.add(relayStates[i] ? 1 : 0);
    }

    // Effect and cycle telemetry
    char effectName[48];
    char nextEffectName[48];
    uint32_t currentCycle = 0;
    uint32_t totalCycles = 0;

    cycleManager.getCurrent(
        effectName, sizeof(effectName),
        currentCycle, totalCycles,
        nextEffectName, sizeof(nextEffectName)
    );

    int effectIdx = effectEngine.getEffectIndex();
    if (effectName[0] == '\0') {
        const EffectDefinition* def = EffectLibrary::getEffect(effectIdx);
        if (def != nullptr && def->name != nullptr) {
            strncpy(effectName, def->name, sizeof(effectName) - 1);
            effectName[sizeof(effectName) - 1] = '\0';
        } else {
            snprintf(effectName, sizeof(effectName), "Effect-%d", effectIdx);
        }
    }

    doc["effect"] = effectName;
    doc["effectIndex"] = effectIdx;
    doc["cycle"] = currentCycle;
    doc["totalCycles"] = totalCycles;
    doc["nextEffect"] = nextEffectName;
    doc["speed"] = speedController.getSpeedMs();
    doc["uptime"] = diagnostics.getUptimeSeconds();
    doc["rssi"] = wifiManager.getRSSI();
    doc["heap"] = diagnostics.getHeapFree();
    doc["minHeap"] = diagnostics.getHeapMinFree();

    char ipStr[32];
    wifiManager.getIP(ipStr, sizeof(ipStr));
    doc["ip"] = ipStr;
    doc["firmware"] = FIRMWARE_VERSION;
    doc["ts"] = millis() / 1000;

    size_t len = serializeJson(doc, s_statusPayload, sizeof(s_statusPayload));
    if (len > 0) {
        httpManager.pushStatus(s_statusPayload);
    }
}

StatusManager statusManager;
