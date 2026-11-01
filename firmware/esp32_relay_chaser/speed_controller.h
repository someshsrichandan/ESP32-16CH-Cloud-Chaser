#pragma once
#include <Arduino.h>
#include "config.h"

class EffectEngine;

/**
 * @enum SpeedPreset
 * @brief Discrete speed preset tiers for relay chase effects.
 */
enum SpeedPreset {
    PRESET_VERY_SLOW = 0,
    PRESET_SLOW,
    PRESET_NORMAL,
    PRESET_FAST,
    PRESET_VERY_FAST,
    PRESET_CUSTOM
};

/**
 * @class SpeedController
 * @brief Manages effect timing, presets, custom millisecond speeds,
 * and synchronizes immediately with EffectEngine.
 */
class SpeedController {
public:
    SpeedController();

    /**
     * @brief Initialize SpeedController with pointer to EffectEngine.
     * @param engine Pointer to EffectEngine instance.
     */
    void init(EffectEngine* engine);

    /**
     * @brief Set speed from a discrete preset.
     * @param preset SpeedPreset enum value.
     */
    void setPreset(SpeedPreset preset);

    /**
     * @brief Set custom speed in milliseconds (clamped between SPEED_MIN_MS and SPEED_MAX_MS).
     * @param ms Interval in milliseconds.
     */
    void setCustomSpeed(int ms);

    /**
     * @brief Retrieve current speed in milliseconds.
     */
    uint32_t getSpeedMs() const;

    /**
     * @brief Retrieve current preset enum value.
     */
    SpeedPreset getPreset() const;

    /**
     * @brief Get human-readable name of current preset.
     */
    const char* getPresetName() const;

    /**
     * @brief Parse preset enum from string (e.g. "FAST", "NORMAL", "VERY_SLOW").
     * @param str Case-insensitive string.
     * @return SpeedPreset enum value.
     */
    static SpeedPreset parsePreset(const char* str);

private:
    EffectEngine* _engine;
    SpeedPreset _currentPreset;
    uint32_t _speedMs;

    void applySpeed();
};
