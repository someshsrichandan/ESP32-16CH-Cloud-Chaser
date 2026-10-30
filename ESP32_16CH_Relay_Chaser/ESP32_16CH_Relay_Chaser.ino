/*
  ══════════════════════════════════════════════════════════════════════════════
   ESP32 × UCTRONICS 16-CHANNEL RELAY CONTROLLER — v10.0 CLOUD & NEXT.JS PRO
   • Outbound Cloud MQTT (Zero Port Forwarding Needed!)
   • Dynamic Pattern & Custom Effect Engine (Zero Re-Flashing Needed!)
   • Web-based OTA Firmware Update (/update)
   • 40 Built-in Effects + Unlimited Dynamic Custom Patterns
   • Anti-Overheat Core (160MHz, Modem Sleep, FreeRTOS Idle Yield)
   • Dual WiFi (Home Router + Fallback Access Point + mDNS)
  ══════════════════════════════════════════════════════════════════════════════
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <PubSubClient.h>
#include "webpage.h"

// ── Default Network & Device Configuration ──────────────────
const char* AP_SSID_DEFAULT     = "ESP32_Relay_Chaser";
const char* AP_PASSWORD_DEFAULT = "12345678";
const char* MDNS_NAME           = "relay16";

// Optional: Pre-set your home WiFi here (or leave empty "" to configure via hotspot at 192.168.4.1)
const char* DEFAULT_HOME_WIFI_SSID = "";
const char* DEFAULT_HOME_WIFI_PASS = "";

// High-Speed Reliable Cloud MQTT Broker (Outbound connection = NO PORT FORWARDING)
const char* MQTT_BROKER_DEFAULT = "broker.hivemq.com";
const int   MQTT_PORT_DEFAULT   = 1883;

// ── 16 GPIO Pin Assignments (Boot-safe, active-LOW) ─────────
const int RELAY_PINS[16] = {
  25,  // Relay 1
   4,  // Relay 2
   5,  // Relay 3
  26,  // Relay 4
  13,  // Relay 5
  27,  // Relay 6
  15,  // Relay 7
  32,  // Relay 8
  16,  // Relay 9
  17,  // Relay 10
  18,  // Relay 11
  19,  // Relay 12
  21,  // Relay 13
  22,  // Relay 14
  23,  // Relay 15
  33   // Relay 16
};

#define RELAY_ON          LOW
#define RELAY_OFF         HIGH
#define NUM_RELAYS        16
#define TOTAL_BUILTIN_FX  40
#define EFFECT_CUSTOM     40
#define TOTAL_EFFECTS     41
#define MAX_CUSTOM_FRAMES 128

// ── State Variables ─────────────────────────────────────────
bool relayState[NUM_RELAYS];
int  currentEffect   = 0;
int  effectSpeed     = 120;     // Clamped 40ms to 1000ms
bool effectRunning   = true;
bool autoMode        = true;    // Auto-startup default ON!
bool autoShuffle     = false;
bool manualMode      = false;
int  autoCycleTime   = 8000;    // ms (Timed mode fallback)

// Cycle repeat management (Repeats each effect N times before auto-advance)
int  autoCycleRepeats = 3;      // Target full cycles per effect (1..50, default 3)
int  completedCycles  = 0;      // Cycles finished for active effect
int  currentCycleStep = 0;      // Step within current cycle
int  autoCycleMode    = 0;      // 0 = Repeat Count Mode (e.g. 3 cycles), 1 = Timed Seconds Mode
volatile bool flagStatePublish = false; // Non-blocking asynchronous broadcast flag

unsigned long autoTimer     = 0;
unsigned long effectTimer   = 0;
unsigned long wifiTimer     = 0;
unsigned long mqttRetryTimer= 0;
unsigned long statePubTimer = 0;
unsigned long wdTimer       = 0;

// Animation step tracking
int effectStep  = 0;
int effectDir   = 1;
int effSub      = 0;
int effA        = 0;
int effB        = 8;
int effDirA     = 1;
int effDirB     = -1;
int effTop      = NUM_RELAYS - 1;
int rippleCenter= 8;

// Dynamic Custom Pattern Storage (No flashing needed!)
uint16_t customPatternFrames[MAX_CUSTOM_FRAMES];
uint8_t  customPatternCount = 0;
char     customPatternName[32] = "Custom Pattern";

// WiFi & Storage
Preferences prefs;
String sta_ssid = "";
String sta_pass = "";
String device_id = "";
String mqtt_broker = "";
int    mqtt_port = MQTT_PORT_DEFAULT;
bool   sta_connecting = false;
unsigned long sta_connect_start = 0;

// MQTT & WebServer
WiFiClient   espClient;
PubSubClient mqttClient(espClient);
WebServer    server(80);

// Forward Declarations
void initRelayPins();
void setRelay(int ch, bool on);
void setRelayMask(uint16_t mask);
void allOff();
void allOn();
void enterManual();
void exitManual();
void changeEffect(int newEff);
void loadCustomPattern(const char* name, int speed, const uint16_t* frames, uint8_t count, bool saveNVS);
void initWiFi();
void checkWiFi();
void initMQTT();
void checkMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishStateMQTT();
void runEffectStep();
void handleAutoMode();
void advanceAutoEffect();
int  getEffectCycleLength(int eff);
void networkTask(void* pvParameters);
float getChipTemp();
String buildStateJSON();
void addCORSHeaders();
void sendState();
void setupWebServer();

// ── 40 Built-in Effect Names ────────────────────────────────
const char* const EFFECT_NAMES[TOTAL_EFFECTS] = {
  "Left Chaser",        // 0
  "Right Chaser",       // 1
  "Ping Pong",          // 2
  "Center Out",         // 3
  "Edges In",           // 4
  "Zip + Unzip",        // 5
  "Dual Snake",         // 6
  "Binary Counter",     // 7
  "Sparkle / Stardust", // 8
  "Twinkle Sky",        // 9
  "Wave Fill",          // 10
  "Heartbeat Pulse",    // 11
  "Cylon Scanner",      // 12
  "Alternating Odd/Ev", // 13
  "Falling Dominos",    // 14
  "Theater Marquee",    // 15
  "Raindrop Ripple",    // 16
  "Fireworks Burst",    // 17
  "Knight Rider",       // 18
  "Meteor Shower",      // 19
  "Double Bounce",      // 20
  "Cross Over",         // 21
  "Strobe Burst",       // 22
  "Stack Build",        // 23
  "Reverse Stack",      // 24
  "Quad Runner",        // 25
  "In-Out Breathe",     // 26
  "Segment Stride",     // 27
  "Police Strobe",      // 28
  "Curtain Reveal",     // 29
  "Cyber Glitch",       // 30
  "Ripple Reflect",     // 31
  "Marching Pairs",     // 32
  "Converge & Cross",   // 33
  "4-Phase Wave",       // 34
  "Harbor Radar",       // 35
  "Fill & Drain",       // 36
  "Random Walk",        // 37
  "DNA Helix",          // 38
  "All Breather",       // 39
  "Dynamic Pattern"     // 40 (Custom user-defined pattern)
};

// ══════════════════════════════════════════════════════════════
//  GPIO SAFE INITIALIZATION
// ══════════════════════════════════════════════════════════════
void initRelayPins() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], INPUT_PULLUP);
    relayState[i] = false;
  }
  delayMicroseconds(250);
  for (int i = 0; i < NUM_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }
}

// ══════════════════════════════════════════════════════════════
//  RELAY CONTROL PRIMITIVES
// ══════════════════════════════════════════════════════════════
void setRelay(int ch, bool on) {
  if (ch < 0 || ch >= NUM_RELAYS) return;
  relayState[ch] = on;
  digitalWrite(RELAY_PINS[ch], on ? RELAY_ON : RELAY_OFF);
}

void setRelayMask(uint16_t mask) {
  for (int i = 0; i < NUM_RELAYS; i++) {
    bool on = (mask >> i) & 1;
    if (on && !relayState[i]) {
      setRelay(i, true);
      delayMicroseconds(1000); // 1ms soft-start per energized coil to eliminate DC rail voltage sag
    } else {
      setRelay(i, on);
    }
  }
}

void allOff() {
  for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, false);
}

void allOn() {
  // Stagger coil energizing by 2ms (32ms total) to eliminate 1.3A inrush voltage collapse & dim glow
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, true);
    delayMicroseconds(2000);
  }
}

void enterManual() {
  manualMode = true;
  effectRunning = false;
}

void exitManual() {
  manualMode = false;
  effectRunning = true;
  effectStep = 0;
  effectDir = 1;
  allOff();
}

int getEffectCycleLength(int eff) {
  switch (eff) {
    case 0:  return 16;  // Left Chaser: 16 steps
    case 1:  return 16;  // Right Chaser: 16 steps
    case 2:  return 30;  // Ping Pong: 30 steps
    case 3:  return 8;   // Center Out: 8 steps
    case 4:  return 8;   // Edges In: 8 steps
    case 5:  return 16;  // Zip + Unzip: 16 steps
    case 6:  return 16;  // Dual Snake: 16 steps
    case 7:  return 32;  // Binary Counter: 32 steps
    case 8:  return 16;  // Sparkle / Stardust: 16 steps
    case 9:  return 16;  // Twinkle Sky: 16 steps
    case 10: return 32;  // Wave Fill: 32 steps
    case 11: return 16;  // Heartbeat Pulse: 16 steps
    case 12: return 30;  // Cylon Scanner: 30 steps
    case 13: return 16;  // Alternating Odd/Ev: 16 steps
    case 14: return 32;  // Falling Dominos: 32 steps
    case 15: return 18;  // Theater Marquee: 18 steps
    case 16: return 14;  // Raindrop Ripple: 14 steps
    case 17: return 18;  // Fireworks Burst: 18 steps
    case 18: return 30;  // Knight Rider: 30 steps
    case 19: return 16;  // Meteor Shower: 16 steps
    case 20: return 30;  // Double Bounce: 30 steps
    case 21: return 16;  // Cross Over: 16 steps
    case 22: return 16;  // Strobe Burst: 16 steps
    case 23: return 64;  // Stack Build: 64 steps
    case 24: return 64;  // Reverse Stack: 64 steps
    case 25: return 16;  // Quad Runner: 16 steps
    case 26: return 16;  // In-Out Breathe: 16 steps
    case 27: return 16;  // Segment Stride: 16 steps
    case 28: return 16;  // Police Strobe: 16 steps
    case 29: return 16;  // Curtain Reveal: 16 steps
    case 30: return 16;  // Cyber Glitch: 16 steps
    case 31: return 26;  // Ripple Reflect: 26 steps
    case 32: return 16;  // Marching Pairs: 16 steps
    case 33: return 16;  // Converge & Cross: 16 steps
    case 34: return 16;  // 4-Phase Wave: 16 steps
    case 35: return 16;  // Harbor Radar: 16 steps
    case 36: return 32;  // Fill & Drain: 32 steps
    case 37: return 20;  // Random Walk: 20 steps
    case 38: return 16;  // DNA Helix: 16 steps
    case 39: return 16;  // All Breather: 16 steps
    case EFFECT_CUSTOM: return (customPatternCount > 0 ? customPatternCount : 16);
    default: return 16;
  }
}

void changeEffect(int newEff) {
  if (newEff < 0 || newEff >= TOTAL_EFFECTS) return;
  currentEffect    = newEff;
  effectStep       = 0;
  currentCycleStep = 0;
  completedCycles  = 0;
  effectDir        = 1;
  effSub           = 0;
  effA             = 0;
  effB             = NUM_RELAYS - 1;
  effDirA          = 1;
  effDirB          = -1;
  effTop           = NUM_RELAYS - 1;
  rippleCenter     = random(2, NUM_RELAYS - 2);
  effectRunning    = true;
  manualMode       = false;
  allOff();
}

void advanceAutoEffect() {
  completedCycles  = 0;
  currentCycleStep = 0;
  autoTimer        = millis();
  int nextEff;
  if (autoShuffle) {
    do {
      nextEff = random(TOTAL_BUILTIN_FX);
    } while (nextEff == currentEffect);
  } else {
    nextEff = (currentEffect + 1) % TOTAL_BUILTIN_FX;
  }
  changeEffect(nextEff);
  flagStatePublish = true;
}

// ══════════════════════════════════════════════════════════════
//  DYNAMIC PATTERN LOADER (Zero Re-flashing Needed!)
// ══════════════════════════════════════════════════════════════
void loadCustomPattern(const char* name, int speed, const uint16_t* frames, uint8_t count, bool saveNVS) {
  if (count == 0) return;
  if (count > MAX_CUSTOM_FRAMES) count = MAX_CUSTOM_FRAMES;
  
  strncpy(customPatternName, name, sizeof(customPatternName) - 1);
  customPatternName[sizeof(customPatternName) - 1] = '\0';
  customPatternCount = count;
  
  for (uint8_t i = 0; i < count; i++) {
    customPatternFrames[i] = frames[i];
  }
  
  if (speed >= 40 && speed <= 1000) {
    effectSpeed = speed;
  }
  
  if (saveNVS) {
    prefs.begin("relay_pat", false);
    prefs.putString("name", customPatternName);
    prefs.putUChar("cnt", customPatternCount);
    prefs.putInt("spd", effectSpeed);
    prefs.putBytes("frames", customPatternFrames, customPatternCount * sizeof(uint16_t));
    prefs.end();
  }
  
  changeEffect(EFFECT_CUSTOM);
}

void restoreCustomPatternFromNVS() {
  prefs.begin("relay_pat", true);
  uint8_t cnt = prefs.getUChar("cnt", 0);
  if (cnt > 0 && cnt <= MAX_CUSTOM_FRAMES) {
    customPatternCount = cnt;
    String nm = prefs.getString("name", "Custom Pattern");
    strncpy(customPatternName, nm.c_str(), sizeof(customPatternName) - 1);
    effectSpeed = prefs.getInt("spd", 120);
    prefs.getBytes("frames", customPatternFrames, customPatternCount * sizeof(uint16_t));
    Serial.printf("[Pattern] Loaded '%s' (%d frames) from flash!\n", customPatternName, customPatternCount);
  } else {
    // Default dynamic sample pattern
    customPatternCount = 16;
    for (int i = 0; i < 16; i++) {
      customPatternFrames[i] = (1 << i);
    }
  }
  prefs.end();
}

// ══════════════════════════════════════════════════════════════
//  WIFI & NETWORK MANAGEMENT (Dual Mode: AP + Router)
// ══════════════════════════════════════════════════════════════
void initWiFi() {
  Serial.println("[WiFi] Starting Dual Mode (AP + Station)...");

  // 1. Generate unique hardware Device ID from physical silicon OTP eFuse
  // Never returns 000000 regardless of radio state!
  uint64_t chipmac = ESP.getEfuseMac();
  char idBuf[32];
  snprintf(idBuf, sizeof(idBuf), "relay16_%02X%02X%02X",
           (uint8_t)(chipmac >> 16),
           (uint8_t)(chipmac >> 8),
           (uint8_t)(chipmac));
  device_id = String(idBuf);
  Serial.printf("[Device] Hardware Device ID: %s\n", device_id.c_str());

  // 2. Read stored WiFi & MQTT credentials from NVS flash
  prefs.begin("relay_cfg", false);
  sta_ssid    = prefs.getString("ssid", "");
  sta_pass    = prefs.getString("pass", "");
  mqtt_broker = prefs.getString("broker", MQTT_BROKER_DEFAULT);
  if (mqtt_broker == "broker.emqx.io" || mqtt_broker.length() == 0) {
    mqtt_broker = MQTT_BROKER_DEFAULT;
    prefs.putString("broker", mqtt_broker);
  }
  mqtt_port   = prefs.getInt("port", MQTT_PORT_DEFAULT);
  prefs.end();

  // If no credentials in NVS, fallback to code defaults if defined
  if (sta_ssid.length() == 0 && strlen(DEFAULT_HOME_WIFI_SSID) > 0) {
    sta_ssid = DEFAULT_HOME_WIFI_SSID;
    sta_pass = DEFAULT_HOME_WIFI_PASS;
  }

  // 3. Configure WiFi: persist credentials and auto-reconnect
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_AP_STA);

  // Disable sleep during initial connection to prevent 802.11 handshake packet drop
  WiFi.setSleep(false);

  // 4. Start Fallback Access Point (Always available for direct connection & recovery)
  WiFi.softAP(AP_SSID_DEFAULT, AP_PASSWORD_DEFAULT, 1, 0, 4);
  Serial.printf("[WiFi] Access Point started: '%s' (IP: %s)\n", AP_SSID_DEFAULT, WiFi.softAPIP().toString().c_str());

  // 5. Connect to Station if router SSID is configured
  if (sta_ssid.length() > 0) {
    Serial.printf("[WiFi] Connecting to home router: '%s'...\n", sta_ssid.c_str());
    WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
    sta_connecting = true;
    sta_connect_start = millis();
  } else {
    Serial.println("[WiFi] No saved home router. Connect to AP 'ESP32_Relay_Chaser' (192.168.4.1) to configure.");
  }

  // 6. Start mDNS (http://relay16.local)
  if (MDNS.begin(MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] http://%s.local\n", MDNS_NAME);
  }
}

void checkWiFi() {
  if (millis() - wifiTimer < 3500) return;
  wifiTimer = millis();

  // Case 1: Currently in an active connection attempt
  if (sta_connecting) {
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
      sta_connecting = false;
      WiFi.setSleep(true); // Now safe to enable modem sleep for power & thermal optimization
      Serial.printf("[WiFi] Connected to router! SSID: '%s', IP: %s, RSSI: %d dBm\n",
                    WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
      publishStateMQTT();
    } else if (millis() - sta_connect_start > 15000) {
      // 15 seconds passed; re-trigger connection in background without blocking
      Serial.printf("[WiFi] Still connecting to '%s'... Retrying background connection.\n", sta_ssid.c_str());
      WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
      sta_connect_start = millis();
    }
  }
  // Case 2: Configured with router credentials, but disconnected (e.g. router rebooted or power cycled)
  else if (sta_ssid.length() > 0) {
    if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
      Serial.printf("[WiFi] Connection lost. Auto-reconnecting to '%s'...\n", sta_ssid.c_str());
      WiFi.setSleep(false);
      WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
      sta_connecting = true;
      sta_connect_start = millis();
    }
  }

  // Guard: ensure softAP is running if somehow stopped
  if (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
    WiFi.softAP(AP_SSID_DEFAULT, AP_PASSWORD_DEFAULT, 1, 0, 4);
  }
}

// ══════════════════════════════════════════════════════════════
//  OUTBOUND CLOUD MQTT (Zero Port Forwarding Remote Control)
// ══════════════════════════════════════════════════════════════
void initMQTT() {
  mqttClient.setServer(mqtt_broker.c_str(), mqtt_port);
  mqttClient.setCallback(mqttCallback);
  // Increase buffer size to 2048 bytes to receive custom frame payloads
  mqttClient.setBufferSize(2048);
}

void checkMQTT() {
  // CRITICAL GUARD: Only attempt cloud connection if WiFi Station has a valid IP!
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    return;
  }
  
  if (mqttClient.connected()) {
    mqttClient.loop();
    // Periodic status publish every 15s
    if (millis() - statePubTimer > 15000) {
      statePubTimer = millis();
      publishStateMQTT();
    }
    return;
  }

  // Reconnect with non-blocking throttle (every 5 seconds)
  if (millis() - mqttRetryTimer < 5000) return;
  mqttRetryTimer = millis();

  String cmdTopic    = device_id + "/cmd";
  String stateTopic  = device_id + "/state";
  String statusTopic = device_id + "/status";

  Serial.printf("[MQTT] Connecting to %s:%d as %s...\n", mqtt_broker.c_str(), mqtt_port, device_id.c_str());
  
  // Connect with Last Will & Testament (LWT)
  if (mqttClient.connect(device_id.c_str(), statusTopic.c_str(), 1, true, "offline")) {
    Serial.printf("[MQTT] Connected to cloud broker! Subscribed to '%s'\n", cmdTopic.c_str());
    mqttClient.publish(statusTopic.c_str(), "online", true);
    mqttClient.subscribe(cmdTopic.c_str());
    publishStateMQTT();
  } else {
    Serial.printf("[MQTT] Connect failed (rc=%d). Retrying in 5s...\n", mqttClient.state());
  }
}

void publishStateMQTT() {
  if (!mqttClient.connected()) return;
  String stateTopic = device_id + "/state";
  String payload = buildStateJSON();
  mqttClient.publish(stateTopic.c_str(), payload.c_str(), true); // Retain=true delivers state instantly to newly connected browsers!
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buf[2048];
  if (length >= sizeof(buf)) length = sizeof(buf) - 1;
  memcpy(buf, payload, length);
  buf[length] = '\0';

  String msg = String(buf);
  Serial.printf("[MQTT RX] %s: %s\n", topic, msg.c_str());

  // Command Parser
  if (msg.indexOf("\"cmd\":\"request_state\"") >= 0 || msg.indexOf("\"cmd\":\"status\"") >= 0) {
    publishStateMQTT();
    return;
  }
  else if (msg.indexOf("\"cmd\":\"relay\"") >= 0) {
    int chIdx = msg.indexOf("\"ch\":");
    int valIdx = msg.indexOf("\"v\":");
    if (chIdx >= 0 && valIdx >= 0) {
      int ch = msg.substring(chIdx + 5).toInt();
      int val = msg.substring(valIdx + 4).toInt();
      enterManual();
      setRelay(ch, val == 1);
    }
  }
  else if (msg.indexOf("\"cmd\":\"effect\"") >= 0) {
    int idIdx = msg.indexOf("\"id\":");
    if (idIdx >= 0) {
      int id = msg.substring(idIdx + 5).toInt();
      changeEffect(id);
    }
  }
  else if (msg.indexOf("\"cmd\":\"speed\"") >= 0) {
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      int spd = msg.substring(vIdx + 4).toInt();
      effectSpeed = constrain(spd, 40, 1000);
      prefs.begin("relay_cfg", false);
      prefs.putInt("spd", effectSpeed);
      prefs.end();
    }
  }
  else if (msg.indexOf("\"cmd\":\"repeats\"") >= 0) {
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      autoCycleRepeats = constrain(msg.substring(vIdx + 4).toInt(), 1, 50);
      autoCycleMode = 0; // Repeat Count Mode
      completedCycles = 0;
      prefs.begin("relay_cfg", false);
      prefs.putInt("repeats", autoCycleRepeats);
      prefs.putInt("cyc_mode", 0);
      prefs.end();
      Serial.printf("[MQTT] Auto Cycle Repeats set to %dx\n", autoCycleRepeats);
    }
  }
  else if (msg.indexOf("\"cmd\":\"cyc_mode\"") >= 0) {
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      autoCycleMode = constrain(msg.substring(vIdx + 4).toInt(), 0, 1);
      prefs.begin("relay_cfg", false);
      prefs.putInt("cyc_mode", autoCycleMode);
      prefs.end();
    }
  }
  else if (msg.indexOf("\"cmd\":\"cycle\"") >= 0) {
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      autoCycleTime = constrain(msg.substring(vIdx + 4).toInt(), 3000, 60000);
      prefs.begin("relay_cfg", false);
      prefs.putInt("cycle_t", autoCycleTime);
      prefs.end();
    }
  }
  else if (msg.indexOf("\"cmd\":\"action\"") >= 0) {
    int aIdx = msg.indexOf("\"a\":\"");
    if (aIdx >= 0) {
      String a = msg.substring(aIdx + 5);
      int endA = a.indexOf("\"");
      if (endA >= 0) a = a.substring(0, endA);
      
      if (a == "all_on") { enterManual(); allOn(); }
      else if (a == "all_off") { enterManual(); allOff(); }
      else if (a == "toggle_run") { if (manualMode) exitManual(); else effectRunning = !effectRunning; }
      else if (a == "toggle_auto") {
        autoMode = !autoMode;
        if (autoMode) {
          manualMode = false;
          effectRunning = true;
          autoTimer = millis();
          completedCycles = 0;
        }
        prefs.begin("relay_cfg", false);
        prefs.putBool("auto_mode", autoMode);
        prefs.end();
      }
      else if (a == "toggle_shuffle") {
        autoShuffle = !autoShuffle;
        prefs.begin("relay_cfg", false);
        prefs.putBool("auto_shuf", autoShuffle);
        prefs.end();
      }
      else if (a == "toggle_manual") { if (manualMode) exitManual(); else enterManual(); }
      else if (a == "next") { changeEffect((currentEffect + 1) % TOTAL_EFFECTS); }
      else if (a == "prev") { changeEffect((currentEffect - 1 + TOTAL_EFFECTS) % TOTAL_EFFECTS); }
      else if (a == "odd") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==0); }
      else if (a == "even") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==1); }
      else if (a == "invert") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, !relayState[i]); }
    }
  }
  else if (msg.indexOf("\"cmd\":\"stream_frame\"") >= 0) {
    int mIdx = msg.indexOf("\"mask\":");
    if (mIdx >= 0) {
      uint16_t mask = (uint16_t)msg.substring(mIdx + 7).toInt();
      enterManual();
      setRelayMask(mask);
    }
  }
  else if (msg.indexOf("\"cmd\":\"load_pattern\"") >= 0) {
    // Dynamic Pattern JSON: {"cmd":"load_pattern","name":"FX1","speed":100,"frames":[1,2,4,8...]}
    String patName = "Custom Pattern";
    int nIdx = msg.indexOf("\"name\":\"");
    if (nIdx >= 0) {
      String sub = msg.substring(nIdx + 8);
      int endQ = sub.indexOf("\"");
      if (endQ >= 0) patName = sub.substring(0, endQ);
    }
    int spd = effectSpeed;
    int sIdx = msg.indexOf("\"speed\":");
    if (sIdx >= 0) spd = msg.substring(sIdx + 8).toInt();

    int fIdx = msg.indexOf("\"frames\":[");
    if (fIdx >= 0) {
      String fStr = msg.substring(fIdx + 10);
      int endBracket = fStr.indexOf("]");
      if (endBracket >= 0) {
        fStr = fStr.substring(0, endBracket);
        uint16_t parsedFrames[MAX_CUSTOM_FRAMES];
        uint8_t count = 0;
        int lastPos = 0;
        while (count < MAX_CUSTOM_FRAMES) {
          int comma = fStr.indexOf(",", lastPos);
          String token;
          if (comma >= 0) {
            token = fStr.substring(lastPos, comma);
            lastPos = comma + 1;
          } else {
            token = fStr.substring(lastPos);
          }
          token.trim();
          if (token.length() > 0) {
            parsedFrames[count++] = (uint16_t)token.toInt();
          }
          if (comma < 0) break;
        }
        if (count > 0) {
          loadCustomPattern(patName.c_str(), spd, parsedFrames, count, true);
        }
      }
    }
  }

  publishStateMQTT();
}

// ══════════════════════════════════════════════════════════════
//  WATCHDOG TASK (FreeRTOS Core 0)
// ══════════════════════════════════════════════════════════════
void wdTask(void*) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(2500));
    if ((millis() - wdTimer) > 25000) {
      Serial.println("[WDT] Hang detected! Safely rebooting ESP32...");
      ESP.restart();
    }
  }
}

// ══════════════════════════════════════════════════════════════
//  40 BUILT-IN EFFECTS + 1 DYNAMIC CUSTOM PATTERN ENGINE
// ══════════════════════════════════════════════════════════════
void runEffectStep() {
  if (manualMode || !effectRunning) return;
  if ((millis() - effectTimer) < (unsigned long)effectSpeed) return;
  effectTimer = millis();

  bool cycleFinished = false;

  switch (currentEffect) {
    case 0: // Left Chaser (16 steps)
      allOff();
      setRelay(effectStep % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;

    case 1: // Right Chaser (16 steps)
      allOff();
      setRelay(NUM_RELAYS - 1 - (effectStep % NUM_RELAYS), true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;

    case 2: // Ping Pong (30 steps roundtrip)
      allOff();
      setRelay(effectStep, true);
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) {
        effectDir = -1;
      } else if (effectStep <= 0) {
        effectDir = 1;
        cycleFinished = true;
      }
      break;

    case 3: { // Center Out (8 steps)
      allOff();
      int s = effectStep % 8;
      setRelay(7 - s, true);
      setRelay(8 + s, true);
      effectStep++;
      if (effectStep >= 8) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 4: { // Edges In (8 steps)
      allOff();
      int s = effectStep % 8;
      setRelay(s, true);
      setRelay(NUM_RELAYS - 1 - s, true);
      effectStep++;
      if (effectStep >= 8) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 5: { // Zip + Unzip (16 steps)
      allOff();
      int s = effectStep % 16;
      if (s < 8) {
        setRelay(7 - s, true);
        setRelay(8 + s, true);
      } else {
        int d = s - 8;
        setRelay(d, true);
        setRelay(NUM_RELAYS - 1 - d, true);
      }
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 6: { // Dual Snake (16 steps)
      allOff();
      int p1 = effectStep % NUM_RELAYS;
      int p2 = (effectStep + 8) % NUM_RELAYS;
      setRelay(p1, true);
      setRelay((p1 + 1) % NUM_RELAYS, true);
      setRelay(p2, true);
      setRelay((p2 + 1) % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 7: { // Binary Counter (32 steps)
      for (int i=0; i<NUM_RELAYS; i++) setRelay(i, (effectStep >> i) & 1);
      effectStep++;
      if (effectStep >= 32) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 8: { // Sparkle / Stardust (16 steps)
      allOff();
      int count = random(2, 5);
      for (int i=0; i<count; i++) setRelay(random(NUM_RELAYS), true);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 9: { // Twinkle Sky (16 steps)
      setRelay(random(NUM_RELAYS), random(2) == 0);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 10: { // Wave Fill (32 steps)
      int s = effectStep;
      if (s < NUM_RELAYS) {
        setRelay(s, true);
      } else {
        setRelay(s - NUM_RELAYS, false);
      }
      effectStep++;
      if (effectStep >= NUM_RELAYS * 2) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 11: { // Heartbeat Pulse (Interleaved soft-start banks, NO coil slam!)
      const uint8_t p[] = {1, 0, 1, 0, 0, 0};
      if (p[effectStep % 6]) {
        for (int i = 0; i < NUM_RELAYS; i += 2) setRelay(i, true);
        delayMicroseconds(2000); // Soft bank stagger
        for (int i = 1; i < NUM_RELAYS; i += 2) setRelay(i, true);
      } else {
        allOff();
      }
      effectStep++;
      if (effectStep >= 12) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 12: { // Cylon Scanner (30 steps)
      allOff();
      if (effectStep > 0) setRelay(effectStep - 1, true);
      setRelay(effectStep, true);
      if (effectStep < NUM_RELAYS - 1) setRelay(effectStep + 1, true);
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) effectDir = -1;
      else if (effectStep <= 0) { effectDir = 1; cycleFinished = true; }
      break;
    }

    case 13: { // Alternating Odd/Ev (16 steps)
      for (int i=0; i<NUM_RELAYS; i++) setRelay(i, (i % 2) == (effectStep % 2));
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 14: { // Falling Dominos (32 steps)
      int s = effectStep;
      if (s < NUM_RELAYS) setRelay(s, true);
      else setRelay(NUM_RELAYS * 2 - 1 - s, false);
      effectStep++;
      if (effectStep >= NUM_RELAYS * 2) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 15: { // Theater Marquee (18 steps)
      allOff();
      for (int i = effectStep % 3; i < NUM_RELAYS; i += 3) setRelay(i, true);
      effectStep++;
      if (effectStep >= 18) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 16: { // Raindrop Ripple (2 full ripples)
      if (effSub == 0) {
        allOff();
        rippleCenter = random(2, NUM_RELAYS - 2);
        setRelay(rippleCenter, true);
        effSub = 1;
      } else if (effSub <= 6) {
        allOff();
        int l = rippleCenter - effSub;
        int r = rippleCenter + effSub;
        if (l >= 0) setRelay(l, true);
        if (r < NUM_RELAYS) setRelay(r, true);
        effSub++;
      } else {
        allOff();
        effSub = 0;
        effectStep++;
        if (effectStep >= 2) { effectStep = 0; cycleFinished = true; }
      }
      break;
    }

    case 17: { // Fireworks Burst (2 full bursts)
      if (effSub == 0) {
        allOff();
        rippleCenter = random(4, 12);
        effA = 0;
        effSub = 1;
      } else if (effSub == 1) {
        allOff();
        setRelay(effA, true);
        effA++;
        if (effA >= rippleCenter) effSub = 2;
      } else if (effSub == 2) {
        allOff();
        for (int w = -2; w <= 2; w++) {
          int p = rippleCenter + w;
          if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
        }
        effSub = 3;
      } else {
        allOff();
        effSub = 0;
        effectStep++;
        if (effectStep >= 2) { effectStep = 0; cycleFinished = true; }
      }
      break;
    }

    case 18: { // Knight Rider (30 steps)
      allOff();
      for (int w = -1; w <= 1; w++) {
        int p = effectStep + w;
        if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
      }
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) effectDir = -1;
      else if (effectStep <= 0) { effectDir = 1; cycleFinished = true; }
      break;
    }

    case 19: { // Meteor Shower (16 steps)
      for (int i=0; i<NUM_RELAYS; i++) {
        if (relayState[i] && random(3) == 0) setRelay(i, false);
      }
      int p = effectStep % NUM_RELAYS;
      setRelay(p, true);
      if (p > 0) setRelay(p - 1, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 20: { // Double Bounce (30 steps)
      allOff();
      setRelay(effA, true);
      setRelay(effB, true);
      effA += effDirA;
      effB += effDirB;
      if (effA >= NUM_RELAYS - 1 || effA <= 0) effDirA = -effDirA;
      if (effB >= NUM_RELAYS - 1 || effB <= 0) effDirB = -effDirB;
      effectStep++;
      if (effectStep >= 30) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 21: { // Cross Over (16 steps)
      allOff();
      int p1 = effectStep % NUM_RELAYS;
      int p2 = NUM_RELAYS - 1 - p1;
      setRelay(p1, true);
      setRelay(p2, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 22: { // Strobe Burst (Soft staggered flash)
      int s = effectStep % 8;
      if (s == 0 || s == 2 || s == 4) {
        allOn();
      } else {
        allOff();
      }
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 23: { // Stack Build (Full 16 channels stacked)
      allOff();
      for (int i = effTop; i < NUM_RELAYS; i++) setRelay(i, true);
      setRelay(effA, true);
      effA++;
      if (effA >= effTop) {
        effTop--;
        effA = 0;
        if (effTop < 0) {
          effTop = NUM_RELAYS - 1;
          allOff();
          cycleFinished = true; // Complete 16-channel stack finished!
        }
      }
      break;
    }

    case 24: { // Reverse Stack (Full 16 channels reverse stacked)
      allOff();
      for (int i = 0; i <= (NUM_RELAYS - 1 - effTop); i++) setRelay(i, true);
      setRelay(effB, true);
      effB--;
      if (effB <= (NUM_RELAYS - 1 - effTop)) {
        effTop--;
        effB = NUM_RELAYS - 1;
        if (effTop < 0) {
          effTop = NUM_RELAYS - 1;
          allOff();
          cycleFinished = true; // Complete 16-channel reverse stack finished!
        }
      }
      break;
    }

    case 25: { // Quad Runner (16 steps)
      allOff();
      for (int k = 0; k < 4; k++) setRelay((effectStep + k * 4) % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 26: { // In-Out Breathe (16 steps)
      allOff();
      int r = (effectStep < 8) ? effectStep : (15 - effectStep);
      for (int i = 7 - r; i <= 8 + r; i++) setRelay(i, true);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 27: { // Segment Stride (16 steps)
      allOff();
      int blk = (effectStep % 4);
      for (int i = 0; i < 4; i++) setRelay(blk * 4 + i, true);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 28: { // Police Strobe (16 steps)
      int s = effectStep % 8;
      if (s == 0 || s == 2) {
        for (int i=0; i<8; i++) setRelay(i, true);
        for (int i=8; i<16; i++) setRelay(i, false);
      } else if (s == 4 || s == 6) {
        for (int i=0; i<8; i++) setRelay(i, false);
        for (int i=8; i<16; i++) setRelay(i, true);
      } else {
        allOff();
      }
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 29: { // Curtain Reveal (16 steps)
      allOff();
      int s = effectStep % 16;
      if (s < 8) {
        for (int i = 7 - s; i <= 8 + s; i++) setRelay(i, true);
      } else {
        int c = s - 8;
        for (int i = c; i < NUM_RELAYS - c; i++) setRelay(i, true);
      }
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 30: { // Cyber Glitch (16 steps)
      if (random(4) == 0) {
        allOff();
      } else {
        int n = random(2, 5);
        for (int i=0; i<n; i++) setRelay(random(NUM_RELAYS), true);
      }
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 31: { // Ripple Reflect (26 steps)
      allOff();
      for (int w = 0; w < 3; w++) {
        int p = effectStep + w;
        if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
      }
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 3) effectDir = -1;
      else if (effectStep <= 0) { effectDir = 1; cycleFinished = true; }
      break;
    }

    case 32: { // Marching Pairs (16 steps)
      allOff();
      int p = effectStep % NUM_RELAYS;
      setRelay(p, true);
      setRelay((p + 1) % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 33: { // Converge & Cross (16 steps)
      allOff();
      int s = effectStep % 16;
      setRelay(s, true);
      setRelay((s + 1) % NUM_RELAYS, true);
      setRelay(NUM_RELAYS - 1 - s, true);
      setRelay((NUM_RELAYS - 2 - s + NUM_RELAYS) % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 34: { // 4-Phase Wave (16 steps)
      for (int i=0; i<NUM_RELAYS; i++) setRelay(i, ((i + effectStep) % 4) == 0);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 35: { // Harbor Radar (16 steps)
      allOff();
      int h = effectStep % NUM_RELAYS;
      setRelay(h, true);
      setRelay((h + 1) % NUM_RELAYS, true);
      setRelay((h + 2) % NUM_RELAYS, true);
      effectStep++;
      if (effectStep >= NUM_RELAYS) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 36: { // Fill & Drain (32 steps)
      int s = effectStep % (NUM_RELAYS * 2);
      if (s < NUM_RELAYS) {
        for (int i=0; i<=s; i++) setRelay(i, true);
      } else {
        int d = s - NUM_RELAYS;
        for (int i=0; i<=d; i++) setRelay(i, false);
        for (int i=d+1; i<NUM_RELAYS; i++) setRelay(i, true);
      }
      effectStep++;
      if (effectStep >= NUM_RELAYS * 2) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 37: { // Random Walk (20 steps)
      allOff();
      effA += random(-1, 2);
      if (effA < 0) effA = 0;
      if (effA >= NUM_RELAYS) effA = NUM_RELAYS - 1;
      setRelay(effA, true);
      effectStep++;
      if (effectStep >= 20) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 38: { // DNA Helix (16 steps)
      allOff();
      int s = effectStep % 8;
      int p1 = s * 2;
      int p2 = (NUM_RELAYS - 1 - (s * 2));
      if (p1 < NUM_RELAYS) setRelay(p1, true);
      if (p2 >= 0 && p2 < NUM_RELAYS) setRelay(p2, true);
      effectStep++;
      if (effectStep >= 16) { effectStep = 0; cycleFinished = true; }
      break;
    }

    case 39: { // All Breather (6 steps, soft banks)
      int s = effectStep % 6;
      if (s < 3) allOn(); else allOff();
      effectStep++;
      if (effectStep >= 6) { effectStep = 0; cycleFinished = true; }
      break;
    }

    // 40: DYNAMIC CUSTOM PATTERN (Loaded directly over MQTT / Web API)
    case EFFECT_CUSTOM: {
      if (customPatternCount > 0) {
        uint16_t mask = customPatternFrames[effectStep % customPatternCount];
        setRelayMask(mask);
        effectStep++;
        if (effectStep >= customPatternCount) {
          effectStep = 0;
          cycleFinished = true;
        }
      }
      break;
    }
  }

  // AUTO CYCLE ENGINE (Strict Cycle Completion: Zero Premature Cutoff & Zero Dead Lag!)
  if (cycleFinished) {
    completedCycles++;
    if (autoMode && !manualMode) {
      bool shouldAdvance = false;
      if (autoCycleMode == 0) {
        // Repeat Count Mode (e.g. repeat current effect 3 or 5 full times)
        if (completedCycles >= autoCycleRepeats) {
          shouldAdvance = true;
        }
      } else {
        // Timed Mode: only advance once target time has elapsed AND current cycle completed!
        if ((millis() - autoTimer) >= (unsigned long)autoCycleTime) {
          shouldAdvance = true;
        }
      }
      if (shouldAdvance) {
        advanceAutoEffect();
      }
    }
  }
}

void handleAutoMode() {
  // All auto effect transitions happen deterministically at cycle boundaries inside runEffectStep()!
  // This guarantees every effect always runs 100% of its channels without mid-animation cutoff.
}

// ══════════════════════════════════════════════════════════════
//  SYSTEM STATUS & JSON BUILDER
// ══════════════════════════════════════════════════════════════
float getChipTemp() {
  #if defined(ESP_ARDUINO_VERSION_MAJOR)
    return temperatureRead();
  #else
    return 0.0;
  #endif
}

String buildStateJSON() {
  unsigned long now = millis();
  int cd = 0;
  if (autoMode && !manualMode) {
    long remain = (long)autoCycleTime - (long)(now - autoTimer);
    cd = (remain > 0) ? (remain / 1000) : 0;
  }

  bool staOk = (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0));
  String staIpStr = staOk ? WiFi.localIP().toString() : (sta_connecting ? "Connecting..." : "Not Connected");

  String j = "{";
  j += "\"device_id\":\"" + device_id + "\",";
  j += "\"e\":" + String(currentEffect) + ",";
  j += "\"ename\":\"" + String(EFFECT_NAMES[currentEffect]) + "\",";
  j += "\"s\":" + String(effectSpeed) + ",";
  j += "\"a\":" + String(autoMode ? 1 : 0) + ",";
  j += "\"sh\":" + String(autoShuffle ? 1 : 0) + ",";
  j += "\"r\":" + String(effectRunning ? 1 : 0) + ",";
  j += "\"m\":" + String(manualMode ? 1 : 0) + ",";
  j += "\"c\":" + String(autoCycleTime / 1000) + ",";
  j += "\"cd\":" + String(cd) + ",";
  j += "\"reps\":" + String(autoCycleRepeats) + ",";
  j += "\"cur_rep\":" + String(completedCycles + 1) + ",";
  j += "\"cyc_mode\":" + String(autoCycleMode) + ",";
  j += "\"temp\":" + String(getChipTemp(), 1) + ",";
  j += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  j += "\"uptime\":" + String(now / 1000) + ",";
  j += "\"mqtt\":{";
  j += "\"connected\":" + String(mqttClient.connected() ? 1 : 0) + ",";
  j += "\"broker\":\"" + mqtt_broker + "\",";
  j += "\"port\":" + String(mqtt_port) + ",";
  j += "\"topic\":\"" + device_id + "\"";
  j += "},";
  j += "\"wifi\":{";
  j += "\"sta_ok\":" + String(staOk ? 1 : 0) + ",";
  j += "\"sta_connecting\":" + String(sta_connecting ? 1 : 0) + ",";
  j += "\"sta_ssid\":\"" + (staOk ? WiFi.SSID() : (sta_ssid.length() > 0 ? sta_ssid : "None")) + "\",";
  j += "\"sta_ip\":\"" + staIpStr + "\",";
  j += "\"rssi\":" + String(staOk ? WiFi.RSSI() : 0) + ",";
  j += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
  j += "\"mdns\":\"http://" + String(MDNS_NAME) + ".local\"";
  j += "},";
  j += "\"custom_pat\":{";
  j += "\"name\":\"" + String(customPatternName) + "\",";
  j += "\"count\":" + String(customPatternCount);
  j += "},";
  j += "\"ch\":[";
  for (int i = 0; i < NUM_RELAYS; i++) {
    j += relayState[i] ? "1" : "0";
    if (i < NUM_RELAYS - 1) j += ",";
  }
  j += "]}";
  return j;
}

void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
}

void sendState() {
  addCORSHeaders();
  server.send(200, "application/json", buildStateJSON());
}

// ══════════════════════════════════════════════════════════════
//  LOCAL HTTP SERVER ROUTES & WEB OTA
// ══════════════════════════════════════════════════════════════
void setupWebServer() {
  // Built-in emergency UI
  server.on("/", HTTP_GET, []() {
    addCORSHeaders();
    server.send(200, "text/html", FPSTR(INDEX_HTML));
  });

  // REST API Endpoints
  server.on("/api/state", HTTP_GET, sendState);
  server.on("/state",     HTTP_GET, sendState);

  server.on("/api/effect", HTTP_GET, []() {
    if (server.hasArg("id")) changeEffect(server.arg("id").toInt());
    sendState();
    publishStateMQTT();
  });

  server.on("/api/relay", HTTP_GET, []() {
    if (server.hasArg("ch") && server.hasArg("v")) {
      enterManual();
      setRelay(server.arg("ch").toInt(), server.arg("v").toInt() == 1);
    }
    sendState();
    publishStateMQTT();
  });

  server.on("/api/speed", HTTP_GET, []() {
    if (server.hasArg("v")) {
      effectSpeed = constrain(server.arg("v").toInt(), 40, 1000);
      prefs.begin("relay_cfg", false);
      prefs.putInt("spd", effectSpeed);
      prefs.end();
    }
    sendState();
    flagStatePublish = true;
  });

  server.on("/api/cycle", HTTP_GET, []() {
    if (server.hasArg("v")) {
      autoCycleTime = constrain(server.arg("v").toInt(), 3000, 60000);
      prefs.begin("relay_cfg", false);
      prefs.putInt("cycle_t", autoCycleTime);
      prefs.end();
    }
    addCORSHeaders();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/repeats", HTTP_GET, []() {
    if (server.hasArg("v")) {
      autoCycleRepeats = constrain(server.arg("v").toInt(), 1, 50);
      autoCycleMode = 0; // Switch to cycle repeats mode
      completedCycles = 0;
      currentCycleStep = 0;
      prefs.begin("relay_cfg", false);
      prefs.putInt("repeats", autoCycleRepeats);
      prefs.putInt("cyc_mode", autoCycleMode);
      prefs.end();
    }
    sendState();
    flagStatePublish = true;
  });

  server.on("/api/cycmode", HTTP_GET, []() {
    if (server.hasArg("v")) {
      autoCycleMode = server.arg("v").toInt() == 1 ? 1 : 0;
      completedCycles = 0;
      currentCycleStep = 0;
      autoTimer = millis();
      prefs.begin("relay_cfg", false);
      prefs.putInt("cyc_mode", autoCycleMode);
      prefs.end();
    }
    sendState();
    flagStatePublish = true;
  });

  server.on("/api/cmd", HTTP_GET, []() {
    String a = server.arg("a");
    if (a == "all_on") { enterManual(); allOn(); }
    else if (a == "all_off") { enterManual(); allOff(); }
    else if (a == "toggle_run") { if (manualMode) exitManual(); else effectRunning = !effectRunning; }
    else if (a == "toggle_auto") {
      autoMode = !autoMode;
      if (autoMode) { manualMode = false; effectRunning = true; autoTimer = millis(); completedCycles = 0; currentCycleStep = 0; }
      prefs.begin("relay_cfg", false);
      prefs.putBool("auto_mode", autoMode);
      prefs.end();
    }
    else if (a == "toggle_shuffle") {
      autoShuffle = !autoShuffle;
      prefs.begin("relay_cfg", false);
      prefs.putBool("auto_shuf", autoShuffle);
      prefs.end();
    }
    else if (a == "toggle_manual") { if (manualMode) exitManual(); else enterManual(); }
    else if (a == "next") { advanceAutoEffect(); }
    else if (a == "prev") { changeEffect((currentEffect - 1 + TOTAL_EFFECTS) % TOTAL_EFFECTS); }
    else if (a == "odd") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==0); }
    else if (a == "even") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==1); }
    else if (a == "invert") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, !relayState[i]); }
    sendState();
    flagStatePublish = true;
  });

  // Dynamic Pattern Upload over HTTP
  server.on("/api/pattern/load", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      // Reuse MQTT parser logic
      int sIdx = body.indexOf("\"speed\":");
      int spd = effectSpeed;
      if (sIdx >= 0) spd = body.substring(sIdx + 8).toInt();

      String patName = "Custom Pattern";
      int nIdx = body.indexOf("\"name\":\"");
      if (nIdx >= 0) {
        String sub = body.substring(nIdx + 8);
        int endQ = sub.indexOf("\"");
        if (endQ >= 0) patName = sub.substring(0, endQ);
      }

      int fIdx = body.indexOf("\"frames\":[");
      if (fIdx >= 0) {
        String fStr = body.substring(fIdx + 10);
        int endB = fStr.indexOf("]");
        if (endB >= 0) {
          fStr = fStr.substring(0, endB);
          uint16_t parsedFrames[MAX_CUSTOM_FRAMES];
          uint8_t count = 0;
          int lastPos = 0;
          while (count < MAX_CUSTOM_FRAMES) {
            int comma = fStr.indexOf(",", lastPos);
            String token = (comma >= 0) ? fStr.substring(lastPos, comma) : fStr.substring(lastPos);
            lastPos = (comma >= 0) ? comma + 1 : fStr.length();
            token.trim();
            if (token.length() > 0) parsedFrames[count++] = (uint16_t)token.toInt();
            if (comma < 0) break;
          }
          if (count > 0) {
            loadCustomPattern(patName.c_str(), spd, parsedFrames, count, true);
          }
        }
      }
    }
    sendState();
    publishStateMQTT();
  });

  // WiFi Settings
  server.on("/api/wifi/scan", HTTP_GET, []() {
    int n = WiFi.scanNetworks();
    String out = "[";
    for (int i = 0; i < n; i++) {
      out += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
      if (i < n - 1) out += ",";
    }
    out += "]";
    addCORSHeaders();
    server.send(200, "application/json", out);
  });

  server.on("/api/wifi/save", HTTP_GET, []() {
    if (server.hasArg("ssid")) {
      String s = server.arg("ssid");
      String p = server.hasArg("pass") ? server.arg("pass") : "";
      s.trim();
      p.trim();
      prefs.begin("relay_cfg", false);
      prefs.putString("ssid", s);
      prefs.putString("pass", p);
      prefs.end();
      sta_ssid = s;
      sta_pass = p;
      Serial.printf("[WiFi] Saved new credentials to NVS: SSID='%s'\n", sta_ssid.c_str());
      WiFi.disconnect(false);
      WiFi.setSleep(false);
      WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
      sta_connecting = true;
      sta_connect_start = millis();
    }
    sendState();
  });

  server.on("/api/wifi/reset", HTTP_GET, []() {
    prefs.begin("relay_cfg", false);
    prefs.remove("ssid");
    prefs.remove("pass");
    prefs.end();
    sta_ssid = "";
    sta_pass = "";
    sta_connecting = false;
    WiFi.disconnect(false);
    Serial.println("[WiFi] Saved credentials removed from NVS.");
    sendState();
  });

  // Web OTA Firmware Update endpoints
  server.on("/update", HTTP_GET, []() {
    addCORSHeaders();
    server.send(200, "text/html",
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<h2>ESP32 Firmware OTA Upload</h2>"
      "<input type='file' name='update' accept='.bin'><br><br>"
      "<input type='submit' value='Upload & Flash'>"
      "</form>");
  });

  server.on("/update", HTTP_POST, []() {
    addCORSHeaders();
    server.send(200, "text/plain", (Update.hasError()) ? "OTA FAIL" : "OTA SUCCESS! Rebooting...");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("[OTA] Success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.onNotFound([]() {
    addCORSHeaders();
    server.send(404, "text/plain", "404 Not Found");
  });

  server.begin();
  Serial.println("[Web] Server running on port 80!");
}

// ══════════════════════════════════════════════════════════════
//  DEDICATED ASYNC NETWORK TASK (Core 0)
//  Handles WiFi, HTTP Server, and Cloud MQTT in background
//  Guarantees Core 1 relay animations have ZERO latency / ZERO jitter!
// ══════════════════════════════════════════════════════════════
void networkTask(void* pvParameters) {
  for (;;) {
    server.handleClient();
    checkWiFi();
    checkMQTT();
    if (flagStatePublish) {
      flagStatePublish = false;
      publishStateMQTT();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ══════════════════════════════════════════════════════════════
//  SETUP INITIALIZATION
// ══════════════════════════════════════════════════════════════
void setup() {
  // 1. Optimize CPU to 160MHz (Drops thermal output & power by ~40%)
  setCpuFrequencyMhz(160);

  Serial.begin(115200);
  delay(200);
  Serial.println("\n\n=======================================================");
  Serial.println(" ESP32 16-CH RELAY CONTROLLER v10.0 (CLOUD & NEXT.JS PRO)");
  Serial.println(" Zero Port Forwarding • Dynamic Effects • Low-Heat Core");
  Serial.println("=======================================================");

  // 2. Safe GPIO Init - 16 Relays Latched OFF instantly
  initRelayPins();
  Serial.println("[1] 16 GPIOs Latched OFF.");

  // 3. Restore Dynamic Custom Pattern from NVS
  restoreCustomPatternFromNVS();

  // 4. Auto-Startup Restore from NVS (Runs immediately on power-on / power restore)
  prefs.begin("relay_cfg", true);
  autoMode         = prefs.getBool("auto_mode", true); // Default TRUE for auto startup!
  autoShuffle      = prefs.getBool("auto_shuf", false);
  currentEffect    = prefs.getInt("eff", 0);
  effectSpeed      = prefs.getInt("spd", 120);
  autoCycleRepeats = prefs.getInt("repeats", 3);       // Default 3 repeats per effect!
  autoCycleMode    = prefs.getInt("cyc_mode", 0);      // Default 0 = Repeat Count Mode!
  autoCycleTime    = prefs.getInt("cycle_t", 8000);
  prefs.end();

  completedCycles  = 0;
  currentCycleStep = 0;
  manualMode       = false;
  effectRunning    = true;

  // 5. Start Dual WiFi
  initWiFi();

  // 6. Start Web Server & OTA
  setupWebServer();

  // 7. Start Outbound Cloud MQTT (broker.hivemq.com)
  initMQTT();

  // 8. Hardware Watchdog on Core 0
  wdTimer = millis();
  xTaskCreatePinnedToCore(wdTask, "WDT", 2048, NULL, 1, NULL, 0);

  // 9. Start Background Networking on Core 0
  xTaskCreatePinnedToCore(networkTask, "NET", 4096, NULL, 1, NULL, 0);

  // 10. Test pulse on Relay 1 (40ms click)
  setRelay(0, true);
  delay(40);
  setRelay(0, false);

  effectTimer    = millis();
  autoTimer      = millis();
  wifiTimer      = millis();
  mqttRetryTimer = millis();
  statePubTimer  = millis();

  Serial.println("=======================================================");
  Serial.printf(" READY! Device ID: %s\n", device_id.c_str());
  Serial.printf(" ESP32 TCP MQTT Broker : %s:%d\n", mqtt_broker.c_str(), mqtt_port);
  Serial.println(" Web Panel WebSocket   : wss://broker.hivemq.com:8884/mqtt");
  Serial.println(" Local Web Dashboard   : http://relay16.local");
  Serial.println(" Fallback AP Direct    : http://192.168.4.1 (ESP32_Relay_Chaser)");
  Serial.printf(" Auto Mode: ACTIVE (%d cycles/effect, Repeat Mode)\n", autoCycleRepeats);
  Serial.println("=======================================================\n");
}

// ══════════════════════════════════════════════════════════════
//  MAIN EXECUTION LOOP (Core 1 - 100% Dedicated to Relays)
// ══════════════════════════════════════════════════════════════
void loop() {
  wdTimer = millis();

  // Handle Relay Animation Step (Zero jitter, zero networking lag!)
  runEffectStep();

  // Handle Timed Auto Cycle fallback
  handleAutoMode();

  // FreeRTOS idle wait
  delay(1);
}
