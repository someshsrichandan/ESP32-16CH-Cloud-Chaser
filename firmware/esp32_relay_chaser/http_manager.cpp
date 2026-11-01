#include "http_manager.h"
#include "config_manager.h"
#include "config.h"
#include <WiFiClient.h>

// Use static response buffer to avoid heap fragmentation
static char s_respBuf[1024];

void HttpManager::begin() {
    // Clients created per-request
}

void HttpManager::addAuthHeaders(HTTPClient& http) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Id", configManager.getDeviceId());
    http.addHeader("X-Device-Secret", configManager.getDeviceSecret());
}

bool HttpManager::doPost(const char* path, const char* body, char* responseOut, size_t responseSize) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s", configManager.getApiBaseUrl(), path);

    HTTPClient http;
    bool isHttps = (strncmp(url, "https://", 8) == 0);

    if (isHttps) {
        WiFiClientSecure secClient;
        secClient.setInsecure(); // Skip strict cert chain verification for simplicity
        http.begin(secClient, url);
        http.setTimeout(HTTP_TIMEOUT_MS);
        addAuthHeaders(http);
        _lastCode = http.POST(body);
    } else {
        WiFiClient plainClient;
        http.begin(plainClient, url);
        http.setTimeout(HTTP_TIMEOUT_MS);
        addAuthHeaders(http);
        _lastCode = http.POST(body);
    }

    _lastOk = (_lastCode == 200 || _lastCode == 201);

    if (_lastOk && responseOut && responseSize > 0) {
        String resp = http.getString();
        strncpy(responseOut, resp.c_str(), responseSize - 1);
        responseOut[responseSize - 1] = '\0';
    }

    http.end();
    return _lastOk;
}

bool HttpManager::doGet(const char* path, char* responseOut, size_t responseSize) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s", configManager.getApiBaseUrl(), path);

    HTTPClient http;
    bool isHttps = (strncmp(url, "https://", 8) == 0);

    if (isHttps) {
        WiFiClientSecure secClient;
        secClient.setInsecure();
        http.begin(secClient, url);
        http.setTimeout(HTTP_TIMEOUT_MS);
        addAuthHeaders(http);
        _lastCode = http.GET();
    } else {
        WiFiClient plainClient;
        http.begin(plainClient, url);
        http.setTimeout(HTTP_TIMEOUT_MS);
        addAuthHeaders(http);
        _lastCode = http.GET();
    }

    _lastOk = (_lastCode == 200);

    if (_lastOk && responseOut && responseSize > 0) {
        String resp = http.getString();
        strncpy(responseOut, resp.c_str(), responseSize - 1);
        responseOut[responseSize - 1] = '\0';
    }

    http.end();
    return _lastOk;
}

bool HttpManager::pushStatus(const char* jsonPayload) {
    return doPost(DEFAULT_API_STATUS_PATH, jsonPayload, nullptr, 0);
}

bool HttpManager::pollCommands() {
    char path[128];
    snprintf(path, sizeof(path), "%s", DEFAULT_API_POLL_PATH);

    if (!doGet(path, s_respBuf, sizeof(s_respBuf))) {
        return false;
    }

    // Parse response: { "commands": [{ "cmd": "...", "payload": {...}, "_id": "..." }] }
    StaticJsonDocument<768> doc;
    DeserializationError err = deserializeJson(doc, s_respBuf);
    if (err) return false;

    JsonArray cmds = doc["commands"].as<JsonArray>();
    if (cmds.isNull() || cmds.size() == 0) return false;

    for (JsonObject cmdObj : cmds) {
        const char* cmd = cmdObj["cmd"] | "";
        static char payloadStr[256];
        if (cmdObj.containsKey("payload")) {
            serializeJson(cmdObj["payload"], payloadStr, sizeof(payloadStr));
        } else {
            payloadStr[0] = '{'; payloadStr[1] = '}'; payloadStr[2] = '\0';
        }

        Serial.printf("[CLOUD] Received command from server: %s\n", cmd);

        if (_commandCallback && cmd[0] != '\0') {
            _commandCallback(cmd, payloadStr);
        }
    }

    return true;
}

bool HttpManager::registerDevice(const char* deviceId, const char* deviceSecret, const char* deviceName) {
    Serial.printf("[CLOUD] Attempting outside server connection: %s%s ...\n", 
                  configManager.getApiBaseUrl(), DEFAULT_API_REGISTER_PATH);

    static char body[256];
    snprintf(body, sizeof(body),
        "{\"deviceId\":\"%s\",\"deviceSecret\":\"%s\",\"deviceName\":\"%s\",\"firmware\":\"%s\"}",
        deviceId, deviceSecret, deviceName, FIRMWARE_VERSION);

    bool ok = doPost(DEFAULT_API_REGISTER_PATH, body, nullptr, 0);
    if (ok) {
        Serial.println();
        Serial.println("==================================================");
        Serial.println("  [CLOUD] CONNECTED TO SERVER SUCCESSFULLY!       ");
        Serial.printf ("  [CLOUD] Server URL : %s\n", configManager.getApiBaseUrl());
        Serial.printf ("  [CLOUD] Device ID  : %s\n", deviceId);
        Serial.printf ("  [CLOUD] Device Name: %s\n", deviceName);
        Serial.println("  [CLOUD] Status     : ONLINE & READY (CONNECTED TO OUTSIDE)");
        Serial.println("==================================================");
        Serial.println();
    } else {
        Serial.printf("[CLOUD] Server response: HTTP %d\n", _lastCode);
        if (_lastCode == 404) {
            Serial.println("[CLOUD] Note: HTTP 404 (Not Found). Verify your Vercel deployment URL or local server.");
        } else if (_lastCode < 0) {
            Serial.printf("[CLOUD] Network/TLS connection error (%d). Check server URL / internet reachability.\n", _lastCode);
        }
    }
    return ok;
}

void HttpManager::setCommandCallback(void (*cb)(const char* cmd, const char* payload)) {
    _commandCallback = cb;
}

HttpManager httpManager;
