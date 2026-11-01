#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

/**
 * @enum WiFiState
 * @brief Connection state machine states for WiFiManager.
 */
enum WiFiState {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED
};

/**
 * @class WiFiManager
 * @brief Non-blocking Wi-Fi connection manager with exponential backoff and zero delay() calls.
 */
class WiFiManager {
public:
    WiFiManager();

    /**
     * @brief Start non-blocking Wi-Fi connection attempt with credentials.
     * @param ssid Wi-Fi SSID name.
     * @param password Wi-Fi WPA2 pre-shared key.
     */
    void begin(const char* ssid, const char* password);

    /**
     * @brief Non-blocking state machine tick. Must be called in Arduino loop().
     */
    void loop();

    /**
     * @brief Check whether Wi-Fi is actively connected with valid IP.
     */
    bool isConnected() const;

    /**
     * @brief Populate buffer with current IP address string.
     * @param buffer Destination char array.
     * @param maxLen Buffer size in bytes.
     */
    void getIP(char* buffer, size_t maxLen) const;

    /**
     * @brief Get current Wi-Fi Received Signal Strength Indicator (RSSI) in dBm.
     */
    int getRSSI() const;
    const char* getSSID() const { return _ssid; }

    /**
     * @brief Get current connection state enum.
     */
    WiFiState getState() const;

private:
    char _ssid[CFG_MAX_SSID_LEN];
    char _password[CFG_MAX_PASSWORD_LEN];

    WiFiState _state;
    unsigned long _lastStateChangeTime;
    unsigned long _connectStartTime;
    uint32_t _backoffMs;

    void initiateConnection();
};

extern WiFiManager wifiManager;
