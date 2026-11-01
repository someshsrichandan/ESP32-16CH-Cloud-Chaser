#include "diagnostics.h"
#include "config.h"
#include <esp_task_wdt.h>
#include <esp_idf_version.h>

Diagnostics::Diagnostics()
    : _lastPrintTime(0),
      _watchdogInitialized(false) {
}

void Diagnostics::initWatchdog(uint32_t timeout_ms) {
    uint32_t timeoutSec = timeout_ms / 1000;
    if (timeoutSec < 1) {
        timeoutSec = 1;
    }

#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = timeoutSec * 1000,
        .idle_core_mask = 0,
        .trigger_panic = true
    };
    esp_task_wdt_init(&twdt_config);
#else
    esp_task_wdt_init(timeoutSec, true);
#endif

    esp_task_wdt_add(NULL); // Subscribe current task (Arduino loopTask)
    _watchdogInitialized = true;
    Serial.printf("[Diagnostics] Hardware Watchdog configured with %u second timeout.\n", (unsigned int)timeoutSec);
}

void Diagnostics::feedWatchdog() {
    if (_watchdogInitialized) {
        esp_task_wdt_reset();
    }
}

void Diagnostics::loop() {
    unsigned long now = millis();
    if (now - _lastPrintTime >= DIAGNOSTICS_PRINT_INTERVAL_MS) {
        _lastPrintTime = now;
        printStats();
    }
}

void Diagnostics::printStats() {
    Serial.println("================== [DIAGNOSTICS] ==================");
    Serial.printf(" Uptime:        %lu sec (%lu ms)\n", (unsigned long)getUptimeSeconds(), (unsigned long)getUptimeMs());
    Serial.printf(" Free Heap:     %lu bytes\n", (unsigned long)getHeapFree());
    Serial.printf(" Min Free Heap: %lu bytes\n", (unsigned long)getHeapMinFree());
    Serial.printf(" Reset Reason:  %s\n", getRestartReason());
    Serial.println("===================================================");
}

uint32_t Diagnostics::getHeapFree() const {
    return ESP.getFreeHeap();
}

uint32_t Diagnostics::getHeapMinFree() const {
    return ESP.getMinFreeHeap();
}

uint32_t Diagnostics::getUptimeSeconds() const {
    return millis() / 1000;
}

uint32_t Diagnostics::getUptimeMs() const {
    return millis();
}

const char* Diagnostics::getRestartReason() const {
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_POWERON:   return "Power-on event";
        case ESP_RST_EXT:       return "External pin reset";
        case ESP_RST_SW:        return "Software reset";
        case ESP_RST_PANIC:     return "Software panic / exception";
        case ESP_RST_INT_WDT:   return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT:  return "Task watchdog reset";
        case ESP_RST_WDT:       return "Hardware watchdog reset";
        case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
        case ESP_RST_BROWNOUT:  return "Brownout reset (low voltage)";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return "Unknown reset reason";
    }
}
