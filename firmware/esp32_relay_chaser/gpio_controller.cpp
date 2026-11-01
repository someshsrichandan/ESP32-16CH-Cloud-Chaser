#include "gpio_controller.h"

GPIOController::GPIOController()
    : _currentBitmask(0x0000), _stateChanged(false) {
}

void GPIOController::init() {
    _currentBitmask = 0x0000;
    _stateChanged = false;

    // Boot safety: configure every pin and immediately force inactive level (HIGH = Relay OFF)
    for (int i = 0; i < RELAY_COUNT; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], RELAY_INACTIVE_LEVEL);
    }
}

void GPIOController::setRelay(int channel, bool state) {
    if (channel < 0 || channel >= RELAY_COUNT) {
        return;
    }

    uint16_t mask = (static_cast<uint16_t>(1) << channel);
    bool currentState = (_currentBitmask & mask) != 0;

    // Suppress unnecessary GPIO writes if hardware already matches desired state
    if (currentState != state) {
        digitalWrite(RELAY_PINS[channel], state ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
        if (state) {
            _currentBitmask |= mask;
        } else {
            _currentBitmask &= ~mask;
        }
        _stateChanged = true;
    }
}

void GPIOController::setAllRelays(bool state) {
    setRelayPattern(state ? 0xFFFF : 0x0000);
}

void GPIOController::setRelayPattern(uint16_t bitmask) {
    uint16_t diff = _currentBitmask ^ bitmask;
    if (diff == 0) {
        return; // State unchanged, no GPIO writes required
    }

    // Toggle only the specific pins that changed
    for (int i = 0; i < RELAY_COUNT; i++) {
        uint16_t mask = (static_cast<uint16_t>(1) << i);
        if (diff & mask) {
            bool state = (bitmask & mask) != 0;
            digitalWrite(RELAY_PINS[i], state ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
        }
    }

    _currentBitmask = bitmask;
    _stateChanged = true;
}

bool GPIOController::getRelayState(int channel) const {
    if (channel < 0 || channel >= RELAY_COUNT) {
        return false;
    }
    return (_currentBitmask & (static_cast<uint16_t>(1) << channel)) != 0;
}

uint16_t GPIOController::getRelayBitmask() const {
    return _currentBitmask;
}

void GPIOController::getRelayArray(bool arr[RELAY_COUNT]) const {
    for (int i = 0; i < RELAY_COUNT; i++) {
        arr[i] = (_currentBitmask & (static_cast<uint16_t>(1) << i)) != 0;
    }
}

bool GPIOController::hasChanged() const {
    return _stateChanged;
}

void GPIOController::clearChanged() {
    _stateChanged = false;
}
