#include "config_manager.h"
#include "config.h"
#include <esp_system.h>
#include <esp_mac.h>
#include <string.h>

#define NVS_NAMESPACE "rchaser"

void ConfigManager::begin() {
    _prefs.begin(NVS_NAMESPACE, false);
    loadConfig();

    if (_deviceId[0] == '\0') {
        generateDeviceId();
        saveConfig();
    }

    if (_deviceSecret[0] == '\0') {
        generateSecret();
        saveConfig();
    }
}

void ConfigManager::generateDeviceId() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(_deviceId, sizeof(_deviceId), "esp32_%02x%02x%02x%02x", mac[2], mac[3], mac[4], mac[5]);
}

void ConfigManager::generateSecret() {
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    uint32_t r3 = esp_random();
    uint32_t r4 = esp_random();
    snprintf(_deviceSecret, sizeof(_deviceSecret), "%08lx%08lx%08lx%08lx", 
             (unsigned long)r1, (unsigned long)r2, (unsigned long)r3, (unsigned long)r4);
}

void ConfigManager::loadConfig() {
    if (!_prefs.begin(NVS_NAMESPACE, true)) {
        resetToDefaults();
        return;
    }

    if (_prefs.isKey("devId")) {
        _prefs.getString("devId", _deviceId, sizeof(_deviceId));
    } else {
        _deviceId[0] = '\0';
    }

    if (_prefs.isKey("devSec")) {
        _prefs.getString("devSec", _deviceSecret, sizeof(_deviceSecret));
    } else {
        _deviceSecret[0] = '\0';
    }

    if (_prefs.isKey("devName")) {
        _prefs.getString("devName", _deviceName, sizeof(_deviceName));
    } else {
        strncpy(_deviceName, DEVICE_NAME_DEFAULT, sizeof(_deviceName) - 1);
        _deviceName[sizeof(_deviceName) - 1] = '\0';
    }

    if (_prefs.isKey("ssid")) {
        _prefs.getString("ssid", _wifiSSID, sizeof(_wifiSSID));
    } else {
        // Fallback to default in config.h
        strncpy(_wifiSSID, DEFAULT_WIFI_SSID, sizeof(_wifiSSID) - 1);
        _wifiSSID[sizeof(_wifiSSID) - 1] = '\0';
    }

    if (_prefs.isKey("pass")) {
        _prefs.getString("pass", _wifiPassword, sizeof(_wifiPassword));
    } else {
        // Fallback to default in config.h
        strncpy(_wifiPassword, DEFAULT_WIFI_PASSWORD, sizeof(_wifiPassword) - 1);
        _wifiPassword[sizeof(_wifiPassword) - 1] = '\0';
    }

    if (_prefs.isKey("apiUrl")) {
        _prefs.getString("apiUrl", _apiBaseUrl, sizeof(_apiBaseUrl));
    } else {
        strncpy(_apiBaseUrl, DEFAULT_API_BASE_URL, sizeof(_apiBaseUrl) - 1);
        _apiBaseUrl[sizeof(_apiBaseUrl) - 1] = '\0';
    }

    _lastEffect = _prefs.getInt("lastEff", 0);
    _lastSpeed  = _prefs.getInt("lastSpd", SPEED_NORMAL);
    _lastMode   = _prefs.getUChar("lastMode", 0);

    _prefs.end();
}

void ConfigManager::saveConfig() {
    if (!_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[Config] Error opening NVS for writing!");
        return;
    }

    _prefs.putString("devId", _deviceId);
    _prefs.putString("devSec", _deviceSecret);
    _prefs.putString("devName", _deviceName);
    _prefs.putString("ssid", _wifiSSID);
    _prefs.putString("pass", _wifiPassword);
    _prefs.putString("apiUrl", _apiBaseUrl);
    _prefs.putInt("lastEff", _lastEffect);
    _prefs.putInt("lastSpd", _lastSpeed);
    _prefs.putUChar("lastMode", _lastMode);

    _prefs.end();
}

void ConfigManager::resetToDefaults() {
    if (_prefs.begin(NVS_NAMESPACE, false)) {
        _prefs.clear();
        _prefs.end();
    }

    strncpy(_deviceName, DEVICE_NAME_DEFAULT, sizeof(_deviceName) - 1);
    _deviceName[sizeof(_deviceName) - 1] = '\0';
    strncpy(_wifiSSID, DEFAULT_WIFI_SSID, sizeof(_wifiSSID) - 1);
    _wifiSSID[sizeof(_wifiSSID) - 1] = '\0';
    strncpy(_wifiPassword, DEFAULT_WIFI_PASSWORD, sizeof(_wifiPassword) - 1);
    _wifiPassword[sizeof(_wifiPassword) - 1] = '\0';
    strncpy(_apiBaseUrl, DEFAULT_API_BASE_URL, sizeof(_apiBaseUrl) - 1);
    _apiBaseUrl[sizeof(_apiBaseUrl) - 1] = '\0';
    _lastEffect = 0;
    _lastSpeed  = SPEED_NORMAL;
    _lastMode   = 0;

    generateDeviceId();
    generateSecret();
    saveConfig();
}

void ConfigManager::printConfig() {
    Serial.println("================ [CONFIGURATION] ================");
    Serial.printf(" Device ID:   %s\n", _deviceId);
    Serial.printf(" Device Name: %s\n", _deviceName);
    Serial.printf(" Wi-Fi SSID:  %s\n", _wifiSSID[0] != '\0' ? _wifiSSID : "(unconfigured)");
    Serial.printf(" API Base:    %s\n", _apiBaseUrl);
    Serial.printf(" Last Effect: %d\n", _lastEffect);
    Serial.printf(" Last Speed:  %d ms\n", _lastSpeed);
    Serial.printf(" Last Mode:   %u\n", _lastMode);
    Serial.println("=================================================");
}

void ConfigManager::setDeviceName(const char* name) {
    if (name != nullptr) {
        strncpy(_deviceName, name, sizeof(_deviceName) - 1);
        _deviceName[sizeof(_deviceName) - 1] = '\0';
    }
}

void ConfigManager::setWifiSSID(const char* ssid) {
    if (ssid != nullptr) {
        strncpy(_wifiSSID, ssid, sizeof(_wifiSSID) - 1);
        _wifiSSID[sizeof(_wifiSSID) - 1] = '\0';
    }
}

void ConfigManager::setWifiPassword(const char* password) {
    if (password != nullptr) {
        strncpy(_wifiPassword, password, sizeof(_wifiPassword) - 1);
        _wifiPassword[sizeof(_wifiPassword) - 1] = '\0';
    }
}

void ConfigManager::setApiBaseUrl(const char* url) {
    if (url != nullptr) {
        strncpy(_apiBaseUrl, url, sizeof(_apiBaseUrl) - 1);
        _apiBaseUrl[sizeof(_apiBaseUrl) - 1] = '\0';
    }
}

void ConfigManager::setDeviceSecret(const char* secret) {
    if (secret != nullptr) {
        strncpy(_deviceSecret, secret, sizeof(_deviceSecret) - 1);
        _deviceSecret[sizeof(_deviceSecret) - 1] = '\0';
    }
}

void ConfigManager::setLastEffect(int idx) {
    _lastEffect = idx;
}

void ConfigManager::setLastSpeed(int ms) {
    _lastSpeed = ms;
}

void ConfigManager::setLastMode(uint8_t mode) {
    _lastMode = mode;
}

ConfigManager configManager;
