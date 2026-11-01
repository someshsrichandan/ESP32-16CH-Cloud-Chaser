#pragma once
#include <Arduino.h>
#include "config.h"

/**
 * @class GPIOController
 * @brief Manages 16 relay channels with active-low hardware abstraction,
 * bitmask state tracking, and write-minimizing logic.
 */
class GPIOController {
public:
    GPIOController();

    /**
     * @brief Configure all 16 GPIO pins as OUTPUT and assert HIGH (Relay OFF).
     * Must be called at boot before any other system operation.
     */
    void init();

    /**
     * @brief Centralized setter for an individual relay channel.
     * @param channel 0-15 channel index.
     * @param state Logical state: true = Relay ON (LOW), false = Relay OFF (HIGH).
     */
    void setRelay(int channel, bool state);

    /**
     * @brief Set all 16 relays to a uniform logical state.
     * @param state Logical state: true = all ON, false = all OFF.
     */
    void setAllRelays(bool state);

    /**
     * @brief Set all 16 relays simultaneously from a 16-bit bitmask.
     * Bit 0 controls channel 0, Bit 15 controls channel 15.
     * Only GPIO pins whose state actually changes will be toggled.
     * @param bitmask 16-bit mask representing logical states.
     */
    void setRelayPattern(uint16_t bitmask);

    /**
     * @brief Get the current logical state of a specific relay channel.
     * @param channel 0-15 channel index.
     * @return true if Relay is logically ON, false if logically OFF.
     */
    bool getRelayState(int channel) const;

    /**
     * @brief Get the full 16-bit bitmask of current logical relay states.
     * @return 16-bit bitmask where bit i = channel i state.
     */
    uint16_t getRelayBitmask() const;

    /**
     * @brief Populate a boolean array with the states of all 16 relays.
     * @param arr Array of size at least 16 to be filled.
     */
    void getRelayArray(bool arr[RELAY_COUNT]) const;

    /**
     * @brief Check whether any relay state has changed since last cleared.
     * @return true if state has changed.
     */
    bool hasChanged() const;

    /**
     * @brief Clear the state changed flag.
     */
    void clearChanged();

private:
    uint16_t _currentBitmask;  ///< Current logical state of all 16 relays (1 = ON, 0 = OFF)
    bool _stateChanged;        ///< Flag set whenever any relay state changes
};

extern GPIOController gpioController;
