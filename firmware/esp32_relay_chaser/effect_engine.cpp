#include "effect_engine.h"
#include "gpio_controller.h"
#include "effect_library.h"

EffectEngine::EffectEngine()
    : _gpio(nullptr),
      _currentEffectIndex(0),
      _speedMs(SPEED_NORMAL_MS),
      _lastStepTime(0),
      _running(false),
      _paused(false),
      _cycleCount(0),
      _stepCounter(0),
      _position(0),
      _direction(1),
      _customState(0),
      _onCycleComplete(nullptr) {
}

void EffectEngine::init(GPIOController* gpio) {
    _gpio = gpio;
    _currentEffectIndex = 0;
    _speedMs = SPEED_NORMAL_MS;
    _lastStepTime = millis();
    _running = true;
    _paused = false;
    _cycleCount = 0;
    resetInternalState();
}

void EffectEngine::update() {
    if (!_running || _paused || _gpio == nullptr) {
        return;
    }

    unsigned long now = millis();
    if (now - _lastStepTime >= _speedMs) {
        _lastStepTime = now;
        bool cycleDone = EffectLibrary::runEffectStep(_currentEffectIndex, this, _gpio);
        _stepCounter++;
        if (cycleDone) {
            _cycleCount++;
            if (_onCycleComplete != nullptr) {
                _onCycleComplete(_currentEffectIndex, _cycleCount);
            }
        }
    }
}

void EffectEngine::setEffect(int effectIndex) {
    if (effectIndex < 0 || effectIndex >= EffectLibrary::getEffectCount()) {
        return;
    }
    _currentEffectIndex = effectIndex;
    _cycleCount = 0;
    _lastStepTime = millis();
    resetInternalState();
}

void EffectEngine::setSpeed(uint32_t speedMs) {
    if (speedMs < SPEED_MIN_MS) {
        speedMs = SPEED_MIN_MS;
    } else if (speedMs > SPEED_MAX_MS) {
        speedMs = SPEED_MAX_MS;
    }
    _speedMs = speedMs;
}

uint32_t EffectEngine::getSpeed() const {
    return _speedMs;
}

void EffectEngine::pause() {
    _paused = true;
}

void EffectEngine::resume() {
    _paused = false;
    _lastStepTime = millis();
}

void EffectEngine::stop() {
    _running = false;
    _paused = false;
    if (_gpio != nullptr) {
        _gpio->setAllRelays(false);
    }
}

void EffectEngine::start() {
    _running = true;
    _paused = false;
    _lastStepTime = millis();
}

bool EffectEngine::isRunning() const {
    return _running;
}

bool EffectEngine::isPaused() const {
    return _paused;
}

int EffectEngine::getEffectIndex() const {
    return _currentEffectIndex;
}

uint32_t EffectEngine::getCycleCount() const {
    return _cycleCount;
}

void EffectEngine::resetCycle() {
    _cycleCount = 0;
}

void EffectEngine::setCycleCompleteCallback(CycleCompleteCallback cb) {
    _onCycleComplete = cb;
}

uint32_t EffectEngine::getStep() const {
    return _stepCounter;
}

void EffectEngine::setStep(uint32_t step) {
    _stepCounter = step;
}

int EffectEngine::getPosition() const {
    return _position;
}

void EffectEngine::setPosition(int pos) {
    _position = pos;
}

int EffectEngine::getDirection() const {
    return _direction;
}

void EffectEngine::setDirection(int dir) {
    _direction = dir;
}

uint32_t EffectEngine::getCustomState() const {
    return _customState;
}

void EffectEngine::setCustomState(uint32_t val) {
    _customState = val;
}

GPIOController* EffectEngine::getGPIO() {
    return _gpio;
}

void EffectEngine::resetInternalState() {
    _stepCounter = 0;
    _position = 0;
    _direction = 1;
    _customState = 0;
}
