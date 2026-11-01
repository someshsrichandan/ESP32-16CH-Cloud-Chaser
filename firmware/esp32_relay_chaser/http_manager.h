#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class HttpManager {
public:
    void begin();

    // Called by status_manager — pushes device status to Vercel
    bool pushStatus(const char* jsonPayload);

    // Called every loop — polls for pending commands from Vercel
    // Returns true if commands were received
    // Calls the command callback for each command
    bool pollCommands();

    // Register device with server (called once on first boot)
    bool registerDevice(const char* deviceId, const char* deviceSecret, const char* deviceName);

    // Set callback for received commands
    void setCommandCallback(void (*cb)(const char* cmd, const char* payload));

    bool isLastRequestOk() const { return _lastOk; }
    int  getLastHttpCode() const { return _lastCode; }

private:
    bool _lastOk = false;
    int  _lastCode = 0;
    void (*_commandCallback)(const char* cmd, const char* payload) = nullptr;

    bool doPost(const char* path, const char* body, char* responseOut, size_t responseSize);
    bool doGet(const char* path, char* responseOut, size_t responseSize);
    void addAuthHeaders(HTTPClient& http);
};

extern HttpManager httpManager;
