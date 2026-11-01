#pragma once
#include <Arduino.h>

// Central command handler — called by httpManager when a command arrives
// Dispatches incoming commands to hardware modules
class CommandHandler {
public:
    void begin();
    // This is registered as the callback in httpManager.setCommandCallback()
    static void handleCommand(const char* cmd, const char* payloadJson);

private:
    static void cmdSetRelay(const char* payloadJson);
    static void cmdMasterOn();
    static void cmdMasterOff();
    static void cmdEmergencyStop();
    static void cmdSetMode(const char* payloadJson);
    static void cmdSetEffect(const char* payloadJson);
    static void cmdSetSpeed(const char* payloadJson);
    static void cmdPause();
    static void cmdResume();
    static void cmdStop();
    static void cmdNextEffect();
    static void cmdPrevEffect();
    static void cmdSetPlaylist(const char* payloadJson);
};

extern CommandHandler commandHandler;
