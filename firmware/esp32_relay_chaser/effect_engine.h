#pragma once
#include <Arduino.h>
#include "config.h"

class GPIOController;

/**
 * @brief Callback invoked whenever an effect finishes one full cycle.
 * @param effectIndex Index of the effect that just completed.
 * @param completedCycles Total consecutive cycles completed for this effect.
 */
typedef void (*CycleCompleteCallback)(int effectIndex, uint32_t completedCycles);

/**
 * @class EffectEngine
 * @brief Non-blocking, millis()-based runtime engine that drives relay chase effects.
 */
class EffectEngine {
public:
    EffectEngine();

    /**
     * @brief Initialize engine with GPIO hardware controller reference.
     * @param gpio Pointer to initialized GPIOController instance.
     */
    void init(GPIOController* gpio);

    /**
     * @brief Main non-blocking update tick. Must be called in Arduino loop().
     * Advances the active effect frame when millis() exceeds speed interval.
     */
    void update();

    /**
     * @brief Set and switch to a new effect index.
     * @param effectIndex 0 to EFFECT_COUNT-1 index into effect library.
     * @param resetTimer If true, resets _lastStepTime to millis(); if false, maintains cadence.
     */
    void setEffect(int effectIndex, bool resetTimer = true);

    /**
     * @brief Immediately apply current frame to relays without waiting for next tick.
     */
    void triggerImmediateStep();

    /**
     * @brief Update the step execution interval in milliseconds.
     * @param speedMs Interval in milliseconds (clamped to valid bounds).
     */
    void setSpeed(uint32_t speedMs);

    /**
     * @brief Retrieve the current speed interval in milliseconds.
     * @return Interval in milliseconds.
     */
    uint32_t getSpeed() const;

    /**
     * @brief Pause effect execution without turning off relays or resetting positions.
     */
    void pause();

    /**
     * @brief Resume effect execution from paused state.
     */
    void resume();

    /**
     * @brief Stop effect execution and turn OFF all relays immediately.
     */
    void stop();

    /**
     * @brief Start or restart effect execution.
     */
    void start();

    /**
     * @brief Check whether engine is actively running (not stopped).
     */
    bool isRunning() const;

    /**
     * @brief Check whether engine is currently paused.
     */
    bool isPaused() const;

    /**
     * @brief Get the currently active effect index.
     */
    int getEffectIndex() const;

    /**
     * @brief Get completed cycle count for the active effect.
     */
    uint32_t getCycleCount() const;

    /**
     * @brief Reset cycle count for active effect back to zero.
     */
    void resetCycle();

    /**
     * @brief Register callback for cycle completion notifications.
     */
    void setCycleCompleteCallback(CycleCompleteCallback cb);

    // ========================================================================
    // Internal State Accessors for Effect Implementations
    // ========================================================================
    uint32_t getStep() const;
    void setStep(uint32_t step);

    int getPosition() const;
    void setPosition(int pos);

    int getDirection() const;
    void setDirection(int dir);

    uint32_t getCustomState() const;
    void setCustomState(uint32_t val);

    GPIOController* getGPIO();

private:
    GPIOController* _gpio;
    int _currentEffectIndex;
    uint32_t _speedMs;
    unsigned long _lastStepTime;
    bool _running;
    bool _paused;
    uint32_t _cycleCount;

    // Internal state variables for effect algorithms
    uint32_t _stepCounter;
    int _position;
    int _direction;
    uint32_t _customState;

    CycleCompleteCallback _onCycleComplete;

    void resetInternalState();
};
