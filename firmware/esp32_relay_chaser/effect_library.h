#pragma once
#include <Arduino.h>
#include "config.h"

class EffectEngine;
class GPIOController;

/**
 * @brief Function pointer for an effect step algorithm.
 * Called on every non-blocking step interval.
 * @param engine Pointer to EffectEngine containing current step/position/direction state.
 * @param gpio Pointer to GPIOController for updating relay outputs.
 * @return true if this step marks the completion of one full cycle; false otherwise.
 */
typedef bool (*EffectStepFunc)(EffectEngine* engine, GPIOController* gpio);

/**
 * @struct EffectDefinition
 * @brief Metadata and function pointer for a relay effect.
 */
struct EffectDefinition {
    int id;
    const char* name;
    const char* description;
    EffectStepFunc stepFunc;
};

#define EFFECT_COUNT 30

/**
 * @class EffectLibrary
 * @brief Registry and implementations for all 30 relay chaser patterns.
 */
class EffectLibrary {
public:
    /**
     * @brief Get total number of registered effects (30).
     */
    static int getEffectCount();

    /**
     * @brief Retrieve effect definition by index (0-29).
     * @param index Effect index.
     * @return Pointer to EffectDefinition, or nullptr if out of bounds.
     */
    static const EffectDefinition* getEffect(int index);

    /**
     * @brief Find effect index by case-insensitive name.
     * @param name Name string to search.
     * @return Effect index (0-29) if found, -1 if not found.
     */
    static int findEffectByName(const char* name);

    /**
     * @brief Execute one step of the specified effect.
     * @param index Effect index.
     * @param engine Pointer to EffectEngine.
     * @param gpio Pointer to GPIOController.
     * @return true if effect completed a full cycle.
     */
    static bool runEffectStep(int index, EffectEngine* engine, GPIOController* gpio);

    // ========================================================================
    // 30 Individual Effect Step Functions
    // ========================================================================
    static bool effectSequentialForward(EffectEngine* engine, GPIOController* gpio);
    static bool effectSequentialReverse(EffectEngine* engine, GPIOController* gpio);
    static bool effectLeftToRight(EffectEngine* engine, GPIOController* gpio);
    static bool effectRightToLeft(EffectEngine* engine, GPIOController* gpio);
    static bool effectCenterOut(EffectEngine* engine, GPIOController* gpio);
    static bool effectOutsideIn(EffectEngine* engine, GPIOController* gpio);
    static bool effectAlternateChannelFlash(EffectEngine* engine, GPIOController* gpio);
    static bool effectOddEvenAlternating(EffectEngine* engine, GPIOController* gpio);
    static bool effectDoubleRunningChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectTripleRunningChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectFourChannelPattern(EffectEngine* engine, GPIOController* gpio);
    static bool effectPingPongChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectKnightRider(EffectEngine* engine, GPIOController* gpio);
    static bool effectCometChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectBouncingLight(EffectEngine* engine, GPIOController* gpio);
    static bool effectWaterfall(EffectEngine* engine, GPIOController* gpio);
    static bool effectWave(EffectEngine* engine, GPIOController* gpio);
    static bool effectFillAndEmpty(EffectEngine* engine, GPIOController* gpio);
    static bool effectRandomChannelChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectRandomGroupFlash(EffectEngine* engine, GPIOController* gpio);
    static bool effectBinaryCounter(EffectEngine* engine, GPIOController* gpio);
    static bool effectProgressiveFill(EffectEngine* engine, GPIOController* gpio);
    static bool effectProgressiveEmpty(EffectEngine* engine, GPIOController* gpio);
    static bool effectSplitHalfChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectSymmetricalMirrorChase(EffectEngine* engine, GPIOController* gpio);
    static bool effectFastStrobe(EffectEngine* engine, GPIOController* gpio);
    static bool effectSlowStrobe(EffectEngine* engine, GPIOController* gpio);
    static bool effectCrossPattern(EffectEngine* engine, GPIOController* gpio);
    static bool effectSpiralSequence(EffectEngine* engine, GPIOController* gpio);
    static bool effectCombinationChase(EffectEngine* engine, GPIOController* gpio);

private:
    static const EffectDefinition effects[EFFECT_COUNT];
};
