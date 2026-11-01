#include "command_handler.h"
#include "gpio_controller.h"
#include "effect_engine.h"
#include "effect_library.h"
#include "cycle_manager.h"
#include "speed_controller.h"
#include "status_manager.h"
#include "config_manager.h"
#include "config.h"
#include <ArduinoJson.h>
#include <string.h>

extern GPIOController gpioController;
extern EffectEngine effectEngine;
extern SpeedController speedController;
extern CycleManager cycleManager;
extern StatusManager statusManager;

void CommandHandler::begin() {
    // Registered in httpManager during setup()
}

void CommandHandler::handleCommand(const char* cmd, const char* payloadJson) {
    if (cmd == nullptr || cmd[0] == '\0') {
        return;
    }

    Serial.printf("[CMD] Handling command: %s\n", cmd);

    if (strcasecmp(cmd, "SET_RELAY") == 0) {
        cmdSetRelay(payloadJson);
    } else if (strcasecmp(cmd, "MASTER_ON") == 0) {
        cmdMasterOn();
    } else if (strcasecmp(cmd, "MASTER_OFF") == 0) {
        cmdMasterOff();
    } else if (strcasecmp(cmd, "EMERGENCY_STOP") == 0) {
        cmdEmergencyStop();
    } else if (strcasecmp(cmd, "SET_MODE") == 0) {
        cmdSetMode(payloadJson);
    } else if (strcasecmp(cmd, "SET_EFFECT") == 0) {
        cmdSetEffect(payloadJson);
    } else if (strcasecmp(cmd, "SET_SPEED") == 0) {
        cmdSetSpeed(payloadJson);
    } else if (strcasecmp(cmd, "PAUSE") == 0) {
        cmdPause();
    } else if (strcasecmp(cmd, "RESUME") == 0) {
        cmdResume();
    } else if (strcasecmp(cmd, "STOP") == 0) {
        cmdStop();
    } else if (strcasecmp(cmd, "NEXT_EFFECT") == 0) {
        cmdNextEffect();
    } else if (strcasecmp(cmd, "PREV_EFFECT") == 0) {
        cmdPrevEffect();
    } else if (strcasecmp(cmd, "SET_PLAYLIST") == 0) {
        cmdSetPlaylist(payloadJson);
    } else if (strcasecmp(cmd, "REQUEST_STATE") == 0) {
        statusManager.pushStatusNow();
    } else {
        Serial.printf("[CMD] Unknown command ignored: %s\n", cmd);
    }
}

void CommandHandler::cmdSetRelay(const char* payloadJson) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payloadJson);
    if (!err) {
        if (doc.containsKey("channel")) {
            int ch = doc["channel"];
            bool state = doc["state"] | false;
            if (ch >= 0 && ch < NUM_RELAYS) {
                gpioController.setRelay(ch, state);
            }
        }
        if (doc.containsKey("bitmask")) {
            uint16_t mask = doc["bitmask"];
            gpioController.setRelayPattern(mask);
        }
        if (doc["invert"] | false) {
            uint16_t cur = gpioController.getRelayBitmask();
            gpioController.setRelayPattern(~cur);
        }
    }
    configManager.setLastMode(1); // manual mode
    statusManager.markChanged();
}

void CommandHandler::cmdMasterOn() {
    effectEngine.stop();
    gpioController.setAllRelays(true);
    configManager.setLastMode(1); // manual mode
    statusManager.markChanged();
}

void CommandHandler::cmdMasterOff() {
    effectEngine.stop();
    gpioController.setAllRelays(false);
    configManager.setLastMode(1); // manual mode
    statusManager.markChanged();
}

void CommandHandler::cmdEmergencyStop() {
    effectEngine.stop();
    gpioController.setAllRelays(false);
    configManager.setLastMode(2); // stopped mode
    statusManager.markChanged();
}

void CommandHandler::cmdSetMode(const char* payloadJson) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payloadJson);
    if (!err) {
        const char* mode = doc["mode"] | "";
        if (strcasecmp(mode, "manual") == 0) {
            effectEngine.stop();
            configManager.setLastMode(1);
        } else if (strcasecmp(mode, "auto") == 0) {
            effectEngine.start();
            configManager.setLastMode(0);
        } else if (strcasecmp(mode, "stopped") == 0) {
            effectEngine.stop();
            gpioController.setAllRelays(false);
            configManager.setLastMode(2);
        }
        configManager.saveConfig();
        statusManager.markChanged();
    }
}

void CommandHandler::cmdSetEffect(const char* payloadJson) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payloadJson);
    if (!err) {
        int effIdx = -1;
        if (doc.containsKey("effectIndex")) {
            effIdx = doc["effectIndex"];
        } else if (doc.containsKey("idx")) {
            effIdx = doc["idx"];
        } else if (doc.containsKey("effect")) {
            const char* name = doc["effect"];
            effIdx = EffectLibrary::findEffectByName(name);
        } else if (doc.containsKey("name")) {
            const char* name = doc["name"];
            effIdx = EffectLibrary::findEffectByName(name);
        }

        if (effIdx >= 0 && effIdx < EffectLibrary::getEffectCount()) {
            effectEngine.setEffect(effIdx);
            effectEngine.start();
            configManager.setLastEffect(effIdx);
            configManager.setLastMode(0); // auto
            configManager.saveConfig();
            statusManager.markChanged();
        }
    }
}

void CommandHandler::cmdSetSpeed(const char* payloadJson) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payloadJson);
    if (!err) {
        if (doc.containsKey("speed")) {
            int spd = doc["speed"];
            speedController.setCustomSpeed(spd);
            effectEngine.setSpeed(speedController.getSpeedMs());
            configManager.setLastSpeed(spd);
            configManager.saveConfig();
        } else if (doc.containsKey("ms")) {
            int spd = doc["ms"];
            speedController.setCustomSpeed(spd);
            effectEngine.setSpeed(speedController.getSpeedMs());
            configManager.setLastSpeed(spd);
            configManager.saveConfig();
        } else if (doc.containsKey("preset")) {
            const char* p = doc["preset"];
            speedController.setPreset(SpeedController::parsePreset(p));
            effectEngine.setSpeed(speedController.getSpeedMs());
            configManager.setLastSpeed(speedController.getSpeedMs());
            configManager.saveConfig();
        }
        statusManager.markChanged();
    }
}

void CommandHandler::cmdPause() {
    effectEngine.pause();
    statusManager.markChanged();
}

void CommandHandler::cmdResume() {
    effectEngine.resume();
    configManager.setLastMode(0); // auto
    statusManager.markChanged();
}

void CommandHandler::cmdStop() {
    effectEngine.stop();
    gpioController.setAllRelays(false);
    configManager.setLastMode(2); // stopped
    statusManager.markChanged();
}

void CommandHandler::cmdNextEffect() {
    cycleManager.nextEffect();
    configManager.setLastEffect(cycleManager.getCurrentEffectIndex());
    statusManager.markChanged();
}

void CommandHandler::cmdPrevEffect() {
    cycleManager.prevEffect();
    configManager.setLastEffect(cycleManager.getCurrentEffectIndex());
    statusManager.markChanged();
}

void CommandHandler::cmdSetPlaylist(const char* payloadJson) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payloadJson);
    if (!err) {
        JsonArray plArr;
        if (doc.containsKey("playlist") && doc["playlist"].is<JsonArray>()) {
            plArr = doc["playlist"].as<JsonArray>();
        } else if (doc.is<JsonArray>()) {
            plArr = doc.as<JsonArray>();
        } else if (doc.containsKey("entries") && doc["entries"].is<JsonArray>()) {
            plArr = doc["entries"].as<JsonArray>();
        }

        if (!plArr.isNull()) {
            PlaylistEntry entries[MAX_PLAYLIST_ENTRIES];
            int count = 0;
            for (JsonObject item : plArr) {
                if (count >= MAX_PLAYLIST_ENTRIES) {
                    break;
                }
                entries[count].effectIndex = item["effectIndex"] | 0;
                entries[count].cyclesRequired = item["cycles"] | (item["cyclesRequired"] | DEFAULT_CYCLES_PER_EFFECT);
                entries[count].speedMs = item["speed"] | (item["speedMs"] | SPEED_NORMAL_MS);
                count++;
            }
            if (count > 0) {
                cycleManager.setPlaylist(entries, count);
            }
        }
        statusManager.markChanged();
    }
}

CommandHandler commandHandler;
