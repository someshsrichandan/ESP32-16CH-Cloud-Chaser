#pragma once
#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "config.h"

/**
 * @class Diagnostics
 * @brief System monitoring, hardware task watchdog feeding, memory analytics, and periodic logging.
 */
class Diagnostics {
public:
    Diagnostics();

    /**
     * @brief Initialize hardware task watchdog timer on ESP32 loopTask.
     * @param timeout_ms Watchdog timeout duration in milliseconds.
     */
    void initWatchdog(uint32_t timeout_ms = WATCHDOG_TIMEOUT_MS);

    /**
     * @brief Feed/reset the hardware watchdog timer. Call every loop() iteration.
     */
    void feedWatchdog();

    /**
     * @brief Non-blocking loop tick that periodically prints diagnostic stats to Serial.
     */
    void loop();

    /**
     * @brief Print system diagnostic statistics immediately to Serial.
     */
    void printStats();

    /**
     * @brief Current free heap in bytes.
     */
    uint32_t getHeapFree() const;

    /**
     * @brief Lowest free heap recorded since boot in bytes.
     */
    uint32_t getHeapMinFree() const;

    /**
     * @brief System uptime in milliseconds.
     */
    uint32_t getUptimeMs() const;

    /**
     * @brief System uptime in seconds.
     */
    uint32_t getUptimeSeconds() const;

    /**
     * @brief Human-readable string identifying the boot/restart reason.
     */
    const char* getRestartReason() const;

private:
    unsigned long _lastPrintTime;
    bool _watchdogInitialized;
};
