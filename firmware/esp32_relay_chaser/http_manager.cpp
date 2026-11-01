#include "http_manager.h"
#include "config_manager.h"
#include "config.h"

// Use static response buffer to avoid heap fragmentation
static char s_respBuf[1024];

void HttpManager::begin() {
    // Nothing to init — WiFiClientSecure is created per-request
}

void HttpManager::addAuthHeaders(HTTPClient& http) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Id", configManager.getDeviceId());
    http.addHeader("X-Device-Secret", configManager.getDeviceSecret());
}

bool HttpManager::doPost(const char* path, const char* body, char* responseOut, size_t responseSize) {
    WiFiClientSecure client;
    client.setInsecure(); // Accept any cert (Vercel uses valid certs, but skip verification for simplicity)

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url), "%s%s", configManager.getApiBaseUrl(), path);

    http.begin(client, url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    addAuthHeaders(http);

    int code = http.POST(body);
    _lastCode = code;
    _lastOk = (code == 200 || code == 201);

    if (_lastOk && responseOut && responseSize > 0) {
        String resp = http.getString();
        strncpy(responseOut, resp.c_str(), responseSize - 1);
        responseOut[responseSize - 1] = '\0';
    }

    http.end();
    return _lastOk;
}

bool HttpManager::doGet(const char* path, char* responseOut, size_t responseSize) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url), "%s%s", configManager.getApiBaseUrl(), path);

    http.begin(client, url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    addAuthHeaders(http);

    int code = http.GET();
    _lastCode = code;
    _lastOk = (code == 200);

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
    // Build poll URL with deviceId as query param
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
        // Serialize payload back to string for the callback
        static char payloadStr[256];
        if (cmdObj.containsKey("payload")) {
            serializeJson(cmdObj["payload"], payloadStr, sizeof(payloadStr));
        } else {
            payloadStr[0] = '{'; payloadStr[1] = '}'; payloadStr[2] = '\0';
        }

        if (_commandCallback && cmd[0] != '\0') {
            _commandCallback(cmd, payloadStr);
        }
    }

    return true;
}

bool HttpManager::registerDevice(const char* deviceId, const char* deviceSecret, const char* deviceName) {
    static char body[256];
    snprintf(body, sizeof(body),
        "{\"deviceId\":\"%s\",\"deviceSecret\":\"%s\",\"deviceName\":\"%s\",\"firmware\":\"%s\"}",
        deviceId, deviceSecret, deviceName, FIRMWARE_VERSION);
    return doPost(DEFAULT_API_REGISTER_PATH, body, nullptr, 0);
}

void HttpManager::setCommandCallback(void (*cb)(const char* cmd, const char* payload)) {
    _commandCallback = cb;
}

HttpManager httpManager;
