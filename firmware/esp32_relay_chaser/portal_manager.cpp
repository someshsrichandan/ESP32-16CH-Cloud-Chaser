#include "portal_manager.h"
#include "config.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include "gpio_controller.h"
#include "http_manager.h"
#include <WiFi.h>

PortalManager portalManager;

PortalManager::PortalManager()
    : _server(80),
      _lastScanTime(0),
      _scanResultsCache("") {
}

void PortalManager::begin() {
    // Configure Access Point on 192.168.4.1
    WiFi.softAPConfig(AP_PORTAL_IP, AP_PORTAL_GW, AP_PORTAL_SUBNET);
    
    char apName[32];
    snprintf(apName, sizeof(apName), "%s-%s", AP_SSID_DEFAULT, configManager.getDeviceId() + 6);
    
    if (strlen(AP_PASSWORD_DEFAULT) > 0) {
        WiFi.softAP(apName, AP_PASSWORD_DEFAULT);
    } else {
        WiFi.softAP(apName);
    }
    
    Serial.printf("[Portal] SoftAP started: %s (IP: 192.168.4.1)\n", apName);

    // Captive portal DNS server (port 53)
    _dnsServer.start(53, "*", AP_PORTAL_IP);

    setupRoutes();
    _server.begin();
    Serial.println("[Portal] Web configuration portal active at http://192.168.4.1");
}

void PortalManager::loop() {
    _dnsServer.processNextRequest();
    _server.handleClient();
}

void PortalManager::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/save", HTTP_POST, [this]() { handleSave(); });
    _server.on("/scan", HTTP_GET, [this]() { handleScan(); });
    _server.on("/relay", HTTP_POST, [this]() { handleRelayAction(); });

    // Captive Portal common redirect routes
    _server.on("/generate_204", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/ncsi.txt", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/connecttest.txt", HTTP_GET, [this]() { handleRoot(); });
    _server.onNotFound([this]() { handleNotFound(); });
}

void PortalManager::handleRoot() {
    char ipBuf[32];
    wifiManager.getIP(ipBuf, sizeof(ipBuf));
    bool connected = wifiManager.isConnected();

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "<title>ESP32 Relay Chaser Setup</title>";
    html += "<style>";
    html += "body{font-family:system-ui,-apple-system,sans-serif;background:#0d1117;color:#c9d1d9;margin:0;padding:16px;}";
    html += ".card{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:20px;max-width:440px;margin:10px auto;}";
    html += "h1{font-size:18px;margin:0 0 16px;color:#58a6ff;text-align:center;}";
    html += "h2{font-size:14px;color:#8b949e;border-bottom:1px solid #21262d;padding-bottom:6px;margin:16px 0 12px;}";
    html += ".info{background:#0d1117;border:1px solid #21262d;border-radius:6px;padding:10px;margin-bottom:14px;font-size:12px;font-family:monospace;}";
    html += ".info div{margin:4px 0;}";
    html += ".badge{display:inline-block;padding:2px 8px;border-radius:12px;font-size:11px;font-weight:bold;}";
    html += ".badge.on{background:#238636;color:#fff;} .badge.off{background:#da3633;color:#fff;}";
    html += "label{display:block;font-size:12px;margin:10px 0 4px;color:#8b949e;}";
    html += "input[type='text'],input[type='password']{width:100%;box-sizing:border-box;background:#0d1117;border:1px solid #30363d;color:#c9d1d9;padding:10px;border-radius:6px;font-size:14px;}";
    html += "input[type='text']:focus,input[type='password']:focus{border-color:#58a6ff;outline:none;}";
    html += "button{width:100%;background:#238636;color:#fff;border:none;padding:12px;border-radius:6px;font-size:14px;font-weight:bold;cursor:pointer;margin-top:16px;}";
    html += "button:hover{background:#2ea043;}";
    html += ".btn-sec{background:#21262d;border:1px solid #30363d;color:#c9d1d9;margin-top:8px;}";
    html += ".btn-sec:hover{background:#30363d;}";
    html += ".btn-row{display:flex;gap:8px;margin-top:8px;}";
    html += ".btn-row button{margin-top:0;}";
    html += ".btn-off{background:#da3633;} .btn-off:hover{background:#b62324;}";
    html += "</style></head><body>";

    html += "<div class='card'>";
    html += "<h1>ESP32 16-CH Relay Setup</h1>";

    html += "<div class='info'>";
    html += "<div>Device ID: <b style='color:#58a6ff;'>" + String(configManager.getDeviceId()) + "</b></div>";
    html += "<div>Wi-Fi Status: ";
    if (connected) {
        html += "<span class='badge on'>CONNECTED</span></div>";
        html += "<div>Connected SSID: <b>" + String(configManager.getWifiSSID()) + "</b></div>";
        html += "<div>Station IP: <b>" + String(ipBuf) + "</b></div>";
        html += "<div>Signal (RSSI): <b>" + String(wifiManager.getRSSI()) + " dBm</b></div>";
    } else {
        html += "<span class='badge off'>DISCONNECTED</span></div>";
        html += "<div>SoftAP IP: <b>192.168.4.1</b></div>";
    }
    html += "<div>Cloud Server: <b>" + String(configManager.getApiBaseUrl()) + "</b></div>";
    html += "</div>";

    // Wi-Fi Config Form
    html += "<form method='POST' action='/save'>";
    html += "<h2>Wi-Fi Network Setup</h2>";
    html += "<label>Wi-Fi Network Name (SSID):</label>";
    html += "<input type='text' id='ssid' name='ssid' value='" + String(configManager.getWifiSSID()) + "' placeholder='Enter your 2.4GHz Wi-Fi name' required>";

    html += "<label>Wi-Fi Password:</label>";
    html += "<input type='password' name='password' value='" + String(configManager.getWifiPassword()) + "' placeholder='Enter Wi-Fi password'>";

    html += "<h2>Cloud Controller (Vercel)</h2>";
    html += "<label>Vercel Web App URL:</label>";
    html += "<input type='text' name='api_url' value='" + String(configManager.getApiBaseUrl()) + "' placeholder='https://your-app.vercel.app' required>";

    html += "<button type='submit'>Save & Connect</button>";
    html += "</form>";

    // Hardware Relay Test Controls (Local)
    html += "<h2>Hardware Relay Test</h2>";
    html += "<div class='btn-row'>";
    html += "<form method='POST' action='/relay?act=on' style='flex:1;'><button type='submit'>All Relays ON</button></form>";
    html += "<form method='POST' action='/relay?act=off' style='flex:1;'><button type='submit' class='btn-off'>All Relays OFF</button></form>";
    html += "</div>";

    html += "<button type='button' class='btn-sec' onclick='location.reload()'>Refresh Status</button>";
    html += "</div>";
    html += "</body></html>";

    _server.send(200, "text/html", html);
}

void PortalManager::handleSave() {
    if (_server.hasArg("ssid")) {
        String ssid = _server.arg("ssid");
        String password = _server.arg("password");
        String apiUrl = _server.arg("api_url");

        ssid.trim();
        password.trim();
        apiUrl.trim();

        if (apiUrl.endsWith("/")) {
            apiUrl.remove(apiUrl.length() - 1);
        }

        configManager.setWifiSSID(ssid.c_str());
        configManager.setWifiPassword(password.c_str());
        if (apiUrl.length() > 0) {
            configManager.setApiBaseUrl(apiUrl.c_str());
        }
        configManager.saveConfig();

        Serial.printf("[Portal] New configuration saved: SSID='%s', API='%s'\n", ssid.c_str(), apiUrl.c_str());

        // Respond with success page before reconnecting
        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
        html += "<title>Settings Saved</title>";
        html += "<style>body{font-family:system-ui,sans-serif;background:#0d1117;color:#c9d1d9;padding:20px;text-align:center;}";
        html += ".card{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:24px;max-width:400px;margin:40px auto;}";
        html += "h1{color:#3fb950;font-size:20px;margin-bottom:12px;}";
        html += "p{font-size:13px;color:#8b949e;line-height:1.5;}";
        html += "a{display:inline-block;margin-top:16px;color:#58a6ff;text-decoration:none;border:1px solid #30363d;padding:10px 16px;border-radius:6px;}";
        html += "</style></head><body>";
        html += "<div class='card'>";
        html += "<h1>Settings Saved!</h1>";
        html += "<p>ESP32 is now attempting to connect to <b>" + ssid + "</b>.</p>";
        html += "<p>After connecting, the ESP32 will register with: <br><code style='color:#58a6ff;'>" + apiUrl + "</code></p>";
        html += "<p>Device ID: <b>" + String(configManager.getDeviceId()) + "</b></p>";
        html += "<a href='/'>Back to Status Page</a>";
        html += "</div></body></html>";

        _server.send(200, "text/html", html);

        // Reconnect Wi-Fi with new credentials
        wifiManager.begin(ssid.c_str(), password.c_str());
    } else {
        _server.send(400, "text/plain", "Missing SSID parameter");
    }
}

void PortalManager::handleRelayAction() {
    String act = _server.arg("act");
    if (act == "on") {
        gpioController.setAllRelays(true);
    } else {
        gpioController.setAllRelays(false);
    }
    _server.sendHeader("Location", "/");
    _server.send(303);
}

void PortalManager::handleScan() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    _server.send(200, "application/json", json);
}

void PortalManager::handleNotFound() {
    // Redirect unknown queries to root captive portal
    _server.sendHeader("Location", "/");
    _server.send(302, "text/plain", "");
}

