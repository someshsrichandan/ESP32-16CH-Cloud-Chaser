#pragma once
#include <Arduino.h>

class StatusManager {
public:
    void begin();
    void loop();                    // call every loop()
    void markChanged();             // forces immediate push on next loop
    void pushStatusNow();           // force immediate HTTP POST

private:
    unsigned long _lastStatusMs = 0;
    bool _changed = false;
    void buildAndPush();
    // builds JSON from all module states and calls httpManager.pushStatus()
};

extern StatusManager statusManager;
