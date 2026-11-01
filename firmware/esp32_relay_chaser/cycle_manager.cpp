#include "cycle_manager.h"
#include "effect_engine.h"
#include "speed_controller.h"
#include "effect_library.h"
#include <string.h>

CycleManager* CycleManager::s_instance = nullptr;

CycleManager::CycleManager()
    : _engine(nullptr),
      _speedController(nullptr),
      _playlistCount(0),
      _currentPlaylistIndex(0),
      _completedCyclesForCurrent(0),
      _shuffleMode(false),
      _sequentialMode(true),
      _cycleCompletePending(false) {
}

void CycleManager::init(EffectEngine* engine, SpeedController* speedController) {
    _engine = engine;
    _speedController = speedController;
    s_instance = this;

    setDefaultPlaylist();

    if (_engine != nullptr) {
        _engine->setCycleCompleteCallback(cycleCallbackThunk);
    }
}

void CycleManager::update() {
    // Synchronous execution handled directly in handleCycleComplete
}

void CycleManager::nextEffect() {
    if (_playlistCount <= 0) return;
    int nextIdx = calculateNextIndex();
    applyPlaylistEntry(nextIdx, true);
    if (_engine != nullptr) {
        _engine->triggerImmediateStep();
    }
}

void CycleManager::jumpToEffect(int effectIndex) {
    if (_playlistCount <= 0) {
        setDefaultPlaylist();
    }
    // Search if this effect exists in current playlist
    for (int i = 0; i < _playlistCount; i++) {
        if (_playlist[i].effectIndex == effectIndex) {
            _currentPlaylistIndex = i;
            _completedCyclesForCurrent = 0;
            _cycleCompletePending = false;
            applyPlaylistEntry(i, true);
            if (_engine != nullptr) {
                _engine->triggerImmediateStep();
            }
            return;
        }
    }
    // If not found in current playlist, build a single-entry playlist for it
    PlaylistEntry singleEntry;
    singleEntry.effectIndex = effectIndex;
    singleEntry.cyclesRequired = DEFAULT_CYCLES_PER_EFFECT;
    singleEntry.speedMs = EffectLibrary::getDefaultSpeed(effectIndex);
    setPlaylist(&singleEntry, 1);
}

void CycleManager::prevEffect() {
    if (_playlistCount <= 0) return;
    int prevIdx = (_currentPlaylistIndex - 1 + _playlistCount) % _playlistCount;
    applyPlaylistEntry(prevIdx, true);
    if (_engine != nullptr) {
        _engine->triggerImmediateStep();
    }
}

void CycleManager::setPlaylist(const PlaylistEntry entries[], int count) {
    if (entries == nullptr || count <= 0) {
        setDefaultPlaylist();
        return;
    }

    if (count > MAX_PLAYLIST_ENTRIES) {
        count = MAX_PLAYLIST_ENTRIES;
    }

    for (int i = 0; i < count; i++) {
        _playlist[i] = entries[i];
        if (_playlist[i].cyclesRequired == 0) {
            _playlist[i].cyclesRequired = DEFAULT_CYCLES_PER_EFFECT;
        }
        if (_playlist[i].speedMs < SPEED_MIN_MS || _playlist[i].speedMs > SPEED_MAX_MS) {
            _playlist[i].speedMs = EffectLibrary::getDefaultSpeed(_playlist[i].effectIndex);
        }
    }
    _playlistCount = count;
    _currentPlaylistIndex = 0;
    _completedCyclesForCurrent = 0;
    _cycleCompletePending = false;

    applyPlaylistEntry(0, true);
    if (_engine != nullptr) {
        _engine->triggerImmediateStep();
    }
}

void CycleManager::setDefaultPlaylist() {
    _playlistCount = EffectLibrary::getEffectCount();
    if (_playlistCount > MAX_PLAYLIST_ENTRIES) {
        _playlistCount = MAX_PLAYLIST_ENTRIES;
    }

    for (int i = 0; i < _playlistCount; i++) {
        _playlist[i].effectIndex = i;
        _playlist[i].cyclesRequired = DEFAULT_CYCLES_PER_EFFECT;
        _playlist[i].speedMs = EffectLibrary::getDefaultSpeed(i);
    }

    _currentPlaylistIndex = 0;
    _completedCyclesForCurrent = 0;
    _cycleCompletePending = false;
}

void CycleManager::getCurrent(char* effectName, size_t maxEffectLen,
                              uint32_t& currentCycle, uint32_t& totalCycles,
                              char* nextEffectName, size_t maxNextLen) const {
    if (effectName != nullptr && maxEffectLen > 0) {
        if (_playlistCount > 0 && _currentPlaylistIndex < _playlistCount) {
            int currentEffIdx = _playlist[_currentPlaylistIndex].effectIndex;
            const EffectDefinition* def = EffectLibrary::getEffect(currentEffIdx);
            if (def != nullptr && def->name != nullptr) {
                snprintf(effectName, maxEffectLen, "%s", def->name);
            } else {
                snprintf(effectName, maxEffectLen, "Effect %d", currentEffIdx);
            }
        } else {
            snprintf(effectName, maxEffectLen, "None");
        }
    }

    if (_playlistCount > 0 && _currentPlaylistIndex < _playlistCount) {
        currentCycle = _completedCyclesForCurrent + 1;
        totalCycles = _playlist[_currentPlaylistIndex].cyclesRequired;
    } else {
        currentCycle = 0;
        totalCycles = 0;
    }

    if (nextEffectName != nullptr && maxNextLen > 0) {
        if (_playlistCount > 0) {
            int nextIdx = calculateNextIndex();
            int nextEffIdx = _playlist[nextIdx].effectIndex;
            const EffectDefinition* nextDef = EffectLibrary::getEffect(nextEffIdx);
            if (nextDef != nullptr && nextDef->name != nullptr) {
                snprintf(nextEffectName, maxNextLen, "%s", nextDef->name);
            } else {
                snprintf(nextEffectName, maxNextLen, "Effect %d", nextEffIdx);
            }
        } else {
            snprintf(nextEffectName, maxNextLen, "None");
        }
    }
}

void CycleManager::setShuffle(bool enable) {
    _shuffleMode = enable;
    if (_shuffleMode) {
        _sequentialMode = false;
    }
}

bool CycleManager::isShuffle() const {
    return _shuffleMode;
}

void CycleManager::setSequential(bool enable) {
    _sequentialMode = enable;
    if (_sequentialMode) {
        _shuffleMode = false;
    }
}

bool CycleManager::isSequential() const {
    return _sequentialMode;
}

int CycleManager::getCurrentEffectIndex() const {
    if (_playlistCount > 0 && _currentPlaylistIndex < _playlistCount) {
        return _playlist[_currentPlaylistIndex].effectIndex;
    }
    return 0;
}

void CycleManager::handleCycleComplete(int effectIndex, uint32_t completedCycles) {
    (void)effectIndex;
    (void)completedCycles;
    _completedCyclesForCurrent++;

    if (_playlistCount > 0 && _currentPlaylistIndex < _playlistCount) {
        uint32_t req = _playlist[_currentPlaylistIndex].cyclesRequired;
        if (req == 0) {
            req = DEFAULT_CYCLES_PER_EFFECT;
        }

        // When exact cycle count is reached, switch immediately to next effect with ZERO dead time!
        if (_completedCyclesForCurrent >= req) {
            int nextIdx = calculateNextIndex();
            applyPlaylistEntry(nextIdx, false); // false = seamless transition on timer rhythm
        }
    }
}

void CycleManager::applyPlaylistEntry(int index, bool resetTimer) {
    if (_playlistCount <= 0) return;
    if (index < 0 || index >= _playlistCount) {
        index = 0;
    }

    _currentPlaylistIndex = index;
    _completedCyclesForCurrent = 0;
    _cycleCompletePending = false;

    int newEffectIdx = _playlist[index].effectIndex;
    uint32_t newSpeed = _playlist[index].speedMs;

    if (_engine != nullptr) {
        _engine->setEffect(newEffectIdx, resetTimer);
        if (newSpeed > 0) {
            _engine->setSpeed(newSpeed);
        }
        _engine->start();
    }
    if (_speedController != nullptr && newSpeed > 0) {
        _speedController->setCustomSpeed(newSpeed);
    }
}

int CycleManager::calculateNextIndex() const {
    if (_playlistCount <= 1) {
        return 0;
    }

    if (_shuffleMode) {
        int next = _currentPlaylistIndex;
        for (int attempts = 0; attempts < 10; attempts++) {
            next = esp_random() % _playlistCount;
            if (next != _currentPlaylistIndex) break;
        }
        return next;
    }

    return (_currentPlaylistIndex + 1) % _playlistCount;
}

void CycleManager::cycleCallbackThunk(int effectIndex, uint32_t completedCycles) {
    if (s_instance != nullptr) {
        s_instance->handleCycleComplete(effectIndex, completedCycles);
    }
}
