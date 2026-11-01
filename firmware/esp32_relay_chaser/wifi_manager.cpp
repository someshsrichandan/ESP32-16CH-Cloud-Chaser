#include "wifi_manager.h"
#include <string.h>

WiFiManager::WiFiManager()
    : _state(WIFI_STATE_DISCONNECTED),
      _lastStateChangeTime(0),
      _connectStartTime(0),
      _backoffMs(WIFI_RECONNECT_INITIAL_MS) {
    _ssid[0] = '\0';
    _password[0] = '\0';
}

void WiFiManager::begin(const char* ssid, const char* password) {
    if (ssid != nullptr) {
        snprintf(_ssid, sizeof(_ssid), "%s", ssid);
    } else {
        _ssid[0] = '\0';
    }

    if (password != nullptr) {
        snprintf(_password, sizeof(_password), "%s", password);
    } else {
        _password[0] = '\0';
    }

    _backoffMs = WIFI_RECONNECT_INITIAL_MS;
    _state = WIFI_STATE_DISCONNECTED;
    _lastStateChangeTime = millis();

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    if (_ssid[0] != '\0') {
        initiateConnection();
    }
}

void WiFiManager::loop() {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (_state != WIFI_STATE_CONNECTED) {
            _state = WIFI_STATE_CONNECTED;
            _backoffMs = WIFI_RECONNECT_INITIAL_MS;
            _lastStateChangeTime = millis();

            char ipBuf[32];
            getIP(ipBuf, sizeof(ipBuf));
            Serial.printf("[WiFi] Connected! IP: %s, RSSI: %d dBm\n", ipBuf, getRSSI());
        }
    } else {
        if (_state == WIFI_STATE_CONNECTED) {
            _state = WIFI_STATE_DISCONNECTED;
            _lastStateChangeTime = millis();
            Serial.println("[WiFi] Lost connection! Scheduling reconnect...");
        } else if (_state == WIFI_STATE_CONNECTING) {
            // Connection attempt timed out (15 seconds)
            if (millis() - _connectStartTime > 15000) {
                Serial.println("[WiFi] Connection attempt timed out.");
                WiFi.disconnect();
                _state = WIFI_STATE_DISCONNECTED;
                _lastStateChangeTime = millis();
            }
        } else if (_state == WIFI_STATE_DISCONNECTED) {
            // Reconnection attempt with exponential backoff
            if (_ssid[0] != '\0' && (millis() - _lastStateChangeTime >= _backoffMs)) {
                // Double backoff up to maximum
                _backoffMs = _backoffMs * 2;
                if (_backoffMs > WIFI_RECONNECT_MAX_MS) {
                    _backoffMs = WIFI_RECONNECT_MAX_MS;
                }
                initiateConnection();
            }
        }
    }
}

bool WiFiManager::isConnected() const {
    return (_state == WIFI_STATE_CONNECTED && WiFi.status() == WL_CONNECTED);
}

void WiFiManager::getIP(char* buffer, size_t maxLen) const {
    if (buffer == nullptr || maxLen == 0) return;

    if (isConnected()) {
        IPAddress ip = WiFi.localIP();
        snprintf(buffer, maxLen, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    } else {
        snprintf(buffer, maxLen, "0.0.0.0");
    }
}

int WiFiManager::getRSSI() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

WiFiState WiFiManager::getState() const {
    return _state;
}

void WiFiManager::initiateConnection() {
    Serial.printf("[WiFi] Connecting to \"%s\" (Backoff: %u ms)...\n", _ssid, _backoffMs);
    WiFi.begin(_ssid, _password);
    _connectStartTime = millis();
    _state = WIFI_STATE_CONNECTING;
}
