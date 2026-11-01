#pragma once
#include <Arduino.h>
#include "config.h"
#include "patterns_data.h"

class EffectEngine;
class GPIOController;

/**
 * @struct EffectDefinition
 * @brief Metadata and parameters for a 16-channel chase pattern.
 */
struct EffectDefinition {
    int id;
    const char* name;
    const char* description;
    uint16_t defaultSpeedMs;
    uint16_t stepCount;
};

/**
 * @class EffectLibrary
 * @brief Registry and step executor for all 18 native Arduino LED chase patterns.
 */
class EffectLibrary {
public:
    /**
     * @brief Get total number of registered patterns (18).
     */
    static int getEffectCount();

    /**
     * @brief Retrieve effect definition by index (0-17).
     * @param index Effect index.
     * @return Pointer to EffectDefinition, or nullptr if out of bounds.
     */
    static const EffectDefinition* getEffect(int index);

    /**
     * @brief Find effect index by case-insensitive name.
     * @param name Name string to search.
     * @return Effect index (0-17) if found, -1 if not found.
     */
    static int findEffectByName(const char* name);

    /**
     * @brief Execute one step of the specified effect.
     * @param index Effect index (0-17).
     * @param engine Pointer to EffectEngine.
     * @param gpio Pointer to GPIOController.
     * @return true if this step marks the completion of one full cycle; false otherwise.
     */
    static bool runEffectStep(int index, EffectEngine* engine, GPIOController* gpio);

    /**
     * @brief Get default step interval in ms for an effect.
     */
    static uint16_t getDefaultSpeed(int index);
};
