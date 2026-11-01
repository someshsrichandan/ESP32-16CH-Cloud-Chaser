#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>

class PortalManager {
public:
    PortalManager();
    void begin();
    void loop();

private:
    WebServer _server;
    DNSServer _dnsServer;
    unsigned long _lastScanTime;
    String _scanResultsCache;

    void setupRoutes();
    void handleRoot();
    void handleSave();
    void handleScan();
    void handleRelayAction();
    void handleNotFound();
};

extern PortalManager portalManager;

