#pragma once
#include <Arduino.h>
#include <Preferences.h>

class ConfigManager {
public:
    void begin();
    void loadConfig();
    void saveConfig();
    void resetToDefaults();
    void generateDeviceId();        // from MAC address

    // Getters
    const char* getDeviceId()       const { return _deviceId; }
    const char* getDeviceSecret()   const { return _deviceSecret; }
    const char* getDeviceName()     const { return _deviceName; }
    const char* getWifiSSID()       const { return _wifiSSID; }
    const char* getWifiPassword()   const { return _wifiPassword; }
    const char* getApiBaseUrl()     const { return _apiBaseUrl; }
    int         getLastEffect()     const { return _lastEffect; }
    int         getLastSpeed()      const { return _lastSpeed; }
    uint8_t     getLastMode()       const { return _lastMode; }

    // Setters (call saveConfig() after)
    void setDeviceName(const char* name);
    void setWifiSSID(const char* ssid);
    void setWifiPassword(const char* password);
    void setApiBaseUrl(const char* url);
    void setDeviceSecret(const char* secret);
    void setLastEffect(int idx);
    void setLastSpeed(int ms);
    void setLastMode(uint8_t mode);

    void printConfig(); // debug print to Serial

private:
    Preferences _prefs;
    char _deviceId[32];
    char _deviceSecret[64];
    char _deviceName[32];
    char _wifiSSID[64];
    char _wifiPassword[64];
    char _apiBaseUrl[128];
    int  _lastEffect = 0;
    int  _lastSpeed  = 200;
    uint8_t _lastMode = 0;

    void generateSecret();
};

extern ConfigManager configManager;
