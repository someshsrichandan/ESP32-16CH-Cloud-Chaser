#pragma once
#include <Arduino.h>
#include "config.h"

class EffectEngine;
class SpeedController;

/**
 * @struct PlaylistEntry
 * @brief Represents an effect item in an automated playback cycle.
 */
struct PlaylistEntry {
    int effectIndex;
    uint32_t cyclesRequired;
    uint32_t speedMs;
};

/**
 * @class CycleManager
 * @brief Manages playlist sequencing, cycle counting, and zero-dead-time transitions.
 */
class CycleManager {
public:
    CycleManager();

    /**
     * @brief Initialize CycleManager with references to engine and speed controller.
     */
    void init(EffectEngine* engine, SpeedController* speedController);

    /**
     * @brief Update function called in main Arduino loop().
     */
    void update();

    /**
     * @brief Advance to next effect in playlist.
     */
    void nextEffect();

    /**
     * @brief Return to previous effect in playlist.
     */
    void prevEffect();

    /**
     * @brief Load a custom playlist of effect entries.
     * @param entries Array of PlaylistEntry.
     * @param count Number of entries (up to MAX_PLAYLIST_ENTRIES).
     */
    void setPlaylist(const PlaylistEntry entries[], int count);

    /**
     * @brief Load the default playlist incorporating all 18 effects with 5-cycle defaults.
     */
    void setDefaultPlaylist();

    /**
     * @brief Retrieve status information about active and subsequent effects.
     */
    void getCurrent(char* effectName, size_t maxEffectLen,
                    uint32_t& currentCycle, uint32_t& totalCycles,
                    char* nextEffectName, size_t maxNextLen) const;

    /**
     * @brief Enable or disable shuffle mode.
     */
    void setShuffle(bool enable);

    /**
     * @brief Check whether shuffle mode is active.
     */
    bool isShuffle() const;

    /**
     * @brief Enable or disable sequential mode.
     */
    void setSequential(bool enable);

    /**
     * @brief Check whether sequential mode is active.
     */
    bool isSequential() const;

    /**
     * @brief Get currently active effect index.
     */
    int getCurrentEffectIndex() const;

    /**
     * @brief Apply playlist entry with optional timer reset for zero dead time.
     */
    void applyPlaylistEntry(int index, bool resetTimer = false);

    /**
     * @brief Internal handler for engine cycle complete callback.
     */
    void handleCycleComplete(int effectIndex, uint32_t completedCycles);

private:
    EffectEngine* _engine;
    SpeedController* _speedController;

    PlaylistEntry _playlist[MAX_PLAYLIST_ENTRIES];
    int _playlistCount;
    int _currentPlaylistIndex;
    uint32_t _completedCyclesForCurrent;

    bool _shuffleMode;
    bool _sequentialMode;
    volatile bool _cycleCompletePending;

    int calculateNextIndex() const;

    static void cycleCallbackThunk(int effectIndex, uint32_t completedCycles);
    static CycleManager* s_instance;
};
