#include "speed_controller.h"
#include "effect_engine.h"
#include <string.h>

SpeedController::SpeedController()
    : _engine(nullptr),
      _currentPreset(PRESET_NORMAL),
      _speedMs(SPEED_NORMAL_MS) {
}

void SpeedController::init(EffectEngine* engine) {
    _engine = engine;
    _currentPreset = PRESET_NORMAL;
    _speedMs = SPEED_NORMAL_MS;
    applySpeed();
}

void SpeedController::setPreset(SpeedPreset preset) {
    _currentPreset = preset;
    switch (preset) {
        case PRESET_VERY_SLOW:
            _speedMs = SPEED_VERY_SLOW_MS;
            break;
        case PRESET_SLOW:
            _speedMs = SPEED_SLOW_MS;
            break;
        case PRESET_NORMAL:
            _speedMs = SPEED_NORMAL_MS;
            break;
        case PRESET_FAST:
            _speedMs = SPEED_FAST_MS;
            break;
        case PRESET_VERY_FAST:
            _speedMs = SPEED_VERY_FAST_MS;
            break;
        case PRESET_CUSTOM:
        default:
            // Custom maintains current _speedMs value
            break;
    }
    applySpeed();
}

void SpeedController::setCustomSpeed(int ms) {
    if (ms < static_cast<int>(SPEED_MIN_MS)) {
        ms = SPEED_MIN_MS;
    } else if (ms > static_cast<int>(SPEED_MAX_MS)) {
        ms = SPEED_MAX_MS;
    }
    _speedMs = static_cast<uint32_t>(ms);

    // Identify if it matches an exact preset
    if (_speedMs == SPEED_VERY_SLOW_MS) {
        _currentPreset = PRESET_VERY_SLOW;
    } else if (_speedMs == SPEED_SLOW_MS) {
        _currentPreset = PRESET_SLOW;
    } else if (_speedMs == SPEED_NORMAL_MS) {
        _currentPreset = PRESET_NORMAL;
    } else if (_speedMs == SPEED_FAST_MS) {
        _currentPreset = PRESET_FAST;
    } else if (_speedMs == SPEED_VERY_FAST_MS) {
        _currentPreset = PRESET_VERY_FAST;
    } else {
        _currentPreset = PRESET_CUSTOM;
    }

    applySpeed();
}

uint32_t SpeedController::getSpeedMs() const {
    return _speedMs;
}

SpeedPreset SpeedController::getPreset() const {
    return _currentPreset;
}

const char* SpeedController::getPresetName() const {
    switch (_currentPreset) {
        case PRESET_VERY_SLOW: return "VERY_SLOW";
        case PRESET_SLOW:      return "SLOW";
        case PRESET_NORMAL:    return "NORMAL";
        case PRESET_FAST:      return "FAST";
        case PRESET_VERY_FAST: return "VERY_FAST";
        case PRESET_CUSTOM:    return "CUSTOM";
        default:               return "UNKNOWN";
    }
}

SpeedPreset SpeedController::parsePreset(const char* str) {
    if (str == nullptr) return PRESET_NORMAL;
    if (strcasecmp(str, "VERY_SLOW") == 0) return PRESET_VERY_SLOW;
    if (strcasecmp(str, "SLOW") == 0)      return PRESET_SLOW;
    if (strcasecmp(str, "NORMAL") == 0)    return PRESET_NORMAL;
    if (strcasecmp(str, "FAST") == 0)      return PRESET_FAST;
    if (strcasecmp(str, "VERY_FAST") == 0) return PRESET_VERY_FAST;
    return PRESET_CUSTOM;
}

void SpeedController::applySpeed() {
    if (_engine != nullptr) {
        _engine->setSpeed(_speedMs);
    }
}
