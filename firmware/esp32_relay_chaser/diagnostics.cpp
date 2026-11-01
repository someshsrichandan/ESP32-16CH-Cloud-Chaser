#include "diagnostics.h"

Diagnostics::Diagnostics()
    : _lastPrintTime(0),
      _watchdogInitialized(false) {
}

void Diagnostics::initWatchdog(uint32_t timeout_ms) {
    uint32_t timeoutSec = timeout_ms / 1000;
    if (timeoutSec < 1) {
        timeoutSec = 1;
    }

    esp_task_wdt_init(timeoutSec, true);
    esp_task_wdt_add(NULL); // Subscribe current task (Arduino loopTask)
    _watchdogInitialized = true;
    Serial.printf("[Diagnostics] Hardware Watchdog configured with %u second timeout.\n", timeoutSec);
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
    Serial.printf(" Uptime:        %u sec (%u ms)\n", getUptimeSeconds(), getUptimeMs());
    Serial.printf(" Free Heap:     %u bytes\n", getHeapFree());
    Serial.printf(" Min Free Heap: %u bytes\n", getHeapMinFree());
    Serial.printf(" Reset Reason:  %s\n", getRestartReason());
    Serial.println("===================================================");
}

uint32_t Diagnostics::getHeapFree() const {
    return ESP.getFreeHeap();
}

uint32_t Diagnostics::getHeapMinFree() const {
    return ESP.getMinFreeHeap();
}

uint32_t Diagnostics::getUptimeMs() const {
    return millis();
}

uint32_t Diagnostics::getUptimeSeconds() const {
    return millis() / 1000;
}

const char* Diagnostics::getRestartReason() const {
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_POWERON:   return "POWERON";
        case ESP_RST_EXT:       return "EXTERNAL_PIN";
        case ESP_RST_SW:        return "SOFTWARE_RESET";
        case ESP_RST_PANIC:     return "PANIC_CRASH";
        case ESP_RST_INT_WDT:   return "INTERRUPT_WATCHDOG";
        case ESP_RST_TASK_WDT:  return "TASK_WATCHDOG";
        case ESP_RST_WDT:       return "OTHER_WATCHDOG";
        case ESP_RST_DEEPSLEEP: return "DEEP_SLEEP";
        case ESP_RST_BROWNOUT:  return "BROWNOUT_LOW_VOLTAGE";
        case ESP_RST_SDIO:      return "SDIO_RESET";
        default:                return "UNKNOWN";
    }
}
