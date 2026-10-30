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

// Free Public Cloud MQTT Broker (Outbound connection = NO PORT FORWARDING)
const char* MQTT_BROKER_DEFAULT = "broker.emqx.io";
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
bool autoMode        = false;
bool autoShuffle     = false;
bool manualMode      = false;
int  autoCycleTime   = 8000;    // ms

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
    relayState[i] = on;
    digitalWrite(RELAY_PINS[i], on ? RELAY_ON : RELAY_OFF);
  }
}

void allOff() {
  for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, false);
}

void allOn() {
  for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, true);
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

void changeEffect(int newEff) {
  if (newEff < 0 || newEff >= TOTAL_EFFECTS) return;
  currentEffect = newEff;
  effectStep    = 0;
  effectDir     = 1;
  effSub        = 0;
  effA          = 0;
  effB          = NUM_RELAYS - 1;
  effDirA       = 1;
  effDirB       = -1;
  effTop        = NUM_RELAYS - 1;
  rippleCenter  = random(2, NUM_RELAYS - 2);
  effectRunning = true;
  manualMode    = false;
  allOff();
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
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(true); // Power & thermal saving

  // Generate unique Device ID from MAC (e.g. relay16_a1b2c3)
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char idBuf[32];
  snprintf(idBuf, sizeof(idBuf), "relay16_%02x%02x%02x", mac[3], mac[4], mac[5]);
  device_id = String(idBuf);

  // Read stored WiFi & MQTT credentials
  prefs.begin("relay_cfg", false);
  sta_ssid    = prefs.getString("ssid", "");
  sta_pass    = prefs.getString("pass", "");
  mqtt_broker = prefs.getString("broker", MQTT_BROKER_DEFAULT);
  mqtt_port   = prefs.getInt("port", MQTT_PORT_DEFAULT);
  prefs.end();

  // Start Access Point
  WiFi.softAP(AP_SSID_DEFAULT, AP_PASSWORD_DEFAULT, 1, 0, 4);
  Serial.printf("[WiFi] AP: %s (IP: %s)\n", AP_SSID_DEFAULT, WiFi.softAPIP().toString().c_str());
  Serial.printf("[Device] Device ID: %s\n", device_id.c_str());

  if (sta_ssid.length() > 0) {
    Serial.printf("[WiFi] Connecting to home router: %s\n", sta_ssid.c_str());
    WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
    sta_connecting = true;
    sta_connect_start = millis();
  }

  // Start mDNS (http://relay16.local)
  if (MDNS.begin(MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] http://%s.local\n", MDNS_NAME);
  }
}

void checkWiFi() {
  if (millis() - wifiTimer < 4000) return;
  wifiTimer = millis();

  if (sta_connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      sta_connecting = false;
      Serial.printf("[WiFi] Connected to router! IP: %s\n", WiFi.localIP().toString().c_str());
    } else if (millis() - sta_connect_start > 25000) {
      sta_connecting = false;
      Serial.println("[WiFi] Connection timeout. Retrying in background...");
    }
  }

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
  if (WiFi.status() != WL_CONNECTED) return;
  
  if (mqttClient.connected()) {
    mqttClient.loop();
    // Periodic status publish every 15s
    if (millis() - statePubTimer > 15000) {
      statePubTimer = millis();
      publishStateMQTT();
    }
    return;
  }

  // Reconnect with non-blocking throttle
  if (millis() - mqttRetryTimer < 6000) return;
  mqttRetryTimer = millis();

  String cmdTopic   = device_id + "/cmd";
  String stateTopic = device_id + "/state";
  String statusTopic= device_id + "/status";

  Serial.printf("[MQTT] Connecting to %s:%d as %s...\n", mqtt_broker.c_str(), mqtt_port, device_id.c_str());
  
  // Connect with Last Will & Testament (LWT)
  if (mqttClient.connect(device_id.c_str(), statusTopic.c_str(), 1, true, "offline")) {
    Serial.println("[MQTT] Connected! Subscribing to command topic...");
    mqttClient.publish(statusTopic.c_str(), "online", true);
    mqttClient.subscribe(cmdTopic.c_str());
    publishStateMQTT();
  } else {
    Serial.printf("[MQTT] Connect failed (rc=%d). Will retry.\n", mqttClient.state());
  }
}

void publishStateMQTT() {
  if (!mqttClient.connected()) return;
  String stateTopic = device_id + "/state";
  String payload = buildStateJSON();
  mqttClient.publish(stateTopic.c_str(), payload.c_str(), false);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buf[2048];
  if (length >= sizeof(buf)) length = sizeof(buf) - 1;
  memcpy(buf, payload, length);
  buf[length] = '\0';

  String msg = String(buf);
  Serial.printf("[MQTT RX] %s: %s\n", topic, msg.c_str());

  // Command Parser
  if (msg.indexOf("\"cmd\":\"relay\"") >= 0) {
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
    }
  }
  else if (msg.indexOf("\"cmd\":\"cycle\"") >= 0) {
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      autoCycleTime = constrain(msg.substring(vIdx + 4).toInt(), 3000, 60000);
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
      else if (a == "toggle_auto") { autoMode = !autoMode; if (autoMode) { manualMode = false; effectRunning = true; autoTimer = millis(); } }
      else if (a == "toggle_shuffle") { autoShuffle = !autoShuffle; }
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

  switch (currentEffect) {
    case 0: allOff(); setRelay(effectStep % NUM_RELAYS, true); effectStep++; break;
    case 1: allOff(); setRelay(NUM_RELAYS - 1 - (effectStep % NUM_RELAYS), true); effectStep++; break;
    case 2: allOff(); setRelay(effectStep, true); effectStep += effectDir;
            if (effectStep >= NUM_RELAYS - 1) effectDir = -1; if (effectStep <= 0) effectDir = 1; break;
    case 3: { allOff(); int c = NUM_RELAYS/2; int s = effectStep % (c + 1);
              if (c - 1 - s >= 0) setRelay(c - 1 - s, true); if (c + s < NUM_RELAYS) setRelay(c + s, true);
              effectStep++; if (effectStep > c) effectStep = 0; break; }
    case 4: { allOff(); int c = NUM_RELAYS/2; int s = effectStep % (c + 1);
              setRelay(s, true); setRelay(NUM_RELAYS - 1 - s, true);
              effectStep++; if (effectStep > c) effectStep = 0; break; }
    case 5: { allOff(); int h = NUM_RELAYS/2; int s = effectStep % NUM_RELAYS;
              if (s < h) { setRelay(h - 1 - s, true); setRelay(h + s, true); }
              else { int d = s - h; setRelay(d, true); setRelay(NUM_RELAYS - 1 - d, true); }
              effectStep++; break; }
    case 6: { allOff(); int p1 = effectStep % NUM_RELAYS; int p2 = (effectStep + 8) % NUM_RELAYS;
              setRelay(p1, true); setRelay((p1 + 1) % NUM_RELAYS, true);
              setRelay(p2, true); setRelay((p2 + 1) % NUM_RELAYS, true); effectStep++; break; }
    case 7: for (int i=0; i<NUM_RELAYS; i++) setRelay(i, (effectStep >> i) & 1); effectStep++;
            if (effectStep > 65535) effectStep = 0; break;
    case 8: { allOff(); int count = random(3, 7); for (int i=0; i<count; i++) setRelay(random(NUM_RELAYS), true); break; }
    case 9: setRelay(random(NUM_RELAYS), random(2) == 0); break;
    case 10: { int s = effectStep % (NUM_RELAYS * 2);
               if (s < NUM_RELAYS) setRelay(s, true); else setRelay(s - NUM_RELAYS, false); effectStep++; break; }
    case 11: { const uint8_t p[] = {1, 0, 1, 0, 0, 0, 0, 0};
               if (p[effectStep % 8]) allOn(); else allOff(); effectStep++; break; }
    case 12: allOff(); if (effectStep > 0) setRelay(effectStep - 1, true); setRelay(effectStep, true);
             if (effectStep < NUM_RELAYS - 1) setRelay(effectStep + 1, true);
             effectStep += effectDir; if (effectStep >= NUM_RELAYS - 1) effectDir = -1; if (effectStep <= 0) effectDir = 1; break;
    case 13: for (int i=0; i<NUM_RELAYS; i++) setRelay(i, (i % 2) == (effectStep % 2)); effectStep++; break;
    case 14: { int s = effectStep % (NUM_RELAYS * 2);
               if (s < NUM_RELAYS) setRelay(s, true); else setRelay(NUM_RELAYS * 2 - 1 - s, false); effectStep++; break; }
    case 15: allOff(); for (int i = effectStep % 3; i < NUM_RELAYS; i += 3) setRelay(i, true); effectStep++; break;
    case 16: if (effSub == 0) { allOff(); rippleCenter = random(2, NUM_RELAYS - 2); setRelay(rippleCenter, true); effSub = 1; }
             else if (effSub <= 6) { allOff(); int l = rippleCenter - effSub; int r = rippleCenter + effSub;
               if (l >= 0) setRelay(l, true); if (r < NUM_RELAYS) setRelay(r, true); effSub++; }
             else { allOff(); effSub = 0; } break;
    case 17: if (effSub == 0) { allOff(); rippleCenter = random(4, 12); effA = 0; effSub = 1; }
             else if (effSub == 1) { allOff(); setRelay(effA, true); effA++; if (effA >= rippleCenter) effSub = 2; }
             else if (effSub == 2) { allOff(); for (int w = -2; w <= 2; w++) { int p = rippleCenter + w; if (p >= 0 && p < NUM_RELAYS) setRelay(p, true); } effSub = 3; }
             else { allOff(); effSub = 0; } break;
    case 18: allOff(); for (int w = -2; w <= 2; w++) { int p = effectStep + w; if (p >= 0 && p < NUM_RELAYS) setRelay(p, true); }
             effectStep += effectDir; if (effectStep >= NUM_RELAYS - 1) effectDir = -1; if (effectStep <= 0) effectDir = 1; break;
    case 19: for (int i=0; i<NUM_RELAYS; i++) { if (relayState[i] && random(3) == 0) setRelay(i, false); }
             { int p = effectStep % NUM_RELAYS; setRelay(p, true); if (p > 0) setRelay(p - 1, true); } effectStep++; break;
    case 20: allOff(); setRelay(effA, true); setRelay(effB, true); effA += effDirA; effB += effDirB;
             if (effA >= NUM_RELAYS - 1 || effA <= 0) effDirA = -effDirA;
             if (effB >= NUM_RELAYS - 1 || effB <= 0) effDirB = -effDirB; break;
    case 21: { allOff(); int p1 = effectStep % NUM_RELAYS; int p2 = NUM_RELAYS - 1 - p1;
               setRelay(p1, true); setRelay(p2, true); effectStep++; break; }
    case 22: { int s = effectStep % 8; if (s == 0 || s == 2 || s == 4) allOn(); else allOff(); effectStep++; break; }
    case 23: allOff(); for (int i = effTop; i < NUM_RELAYS; i++) setRelay(i, true);
             setRelay(effA, true); effA++; if (effA >= effTop) { effTop--; effA = 0; if (effTop < 0) { effTop = NUM_RELAYS - 1; allOff(); } } break;
    case 24: allOff(); for (int i = 0; i <= (NUM_RELAYS - 1 - effTop); i++) setRelay(i, true);
             setRelay(effB, true); effB--; if (effB <= (NUM_RELAYS - 1 - effTop)) { effTop--; effB = NUM_RELAYS - 1; if (effTop < 0) { effTop = NUM_RELAYS - 1; allOff(); } } break;
    case 25: allOff(); for (int k = 0; k < 4; k++) setRelay((effectStep + k * 4) % NUM_RELAYS, true); effectStep++; break;
    case 26: { allOff(); int r = (effectStep < 8) ? effectStep : (15 - effectStep);
               for (int i = 7 - r; i <= 8 + r; i++) setRelay(i, true); effectStep = (effectStep + 1) % 16; break; }
    case 27: { allOff(); int blk = (effectStep % 4); for (int i = 0; i < 4; i++) setRelay(blk * 4 + i, true); effectStep++; break; }
    case 28: { int s = effectStep % 8;
               if (s == 0 || s == 2) { for (int i=0; i<8; i++) setRelay(i, true); for (int i=8; i<16; i++) setRelay(i, false); }
               else if (s == 4 || s == 6) { for (int i=0; i<8; i++) setRelay(i, false); for (int i=8; i<16; i++) setRelay(i, true); }
               else allOff(); effectStep++; break; }
    case 29: { allOff(); int s = effectStep % 16;
               if (s < 8) { for (int i = 7 - s; i <= 8 + s; i++) setRelay(i, true); }
               else { int c = s - 8; for (int i = c; i < NUM_RELAYS - c; i++) setRelay(i, true); } effectStep++; break; }
    case 30: if (random(4) == 0) allOff(); else { int n = random(2, 6); for (int i=0; i<n; i++) setRelay(random(NUM_RELAYS), true); } break;
    case 31: allOff(); for (int w = 0; w < 3; w++) { int p = effectStep + w; if (p >= 0 && p < NUM_RELAYS) setRelay(p, true); }
             effectStep += effectDir; if (effectStep >= NUM_RELAYS - 3) effectDir = -1; if (effectStep <= 0) effectDir = 1; break;
    case 32: { allOff(); int p = effectStep % NUM_RELAYS; setRelay(p, true); setRelay((p + 1) % NUM_RELAYS, true); effectStep++; break; }
    case 33: { allOff(); int s = effectStep % 16; setRelay(s, true); setRelay((s + 1) % NUM_RELAYS, true);
               setRelay(NUM_RELAYS - 1 - s, true); setRelay((NUM_RELAYS - 2 - s + NUM_RELAYS) % NUM_RELAYS, true); effectStep++; break; }
    case 34: for (int i=0; i<NUM_RELAYS; i++) setRelay(i, ((i + effectStep) % 4) == 0); effectStep++; break;
    case 35: { allOff(); int h = effectStep % NUM_RELAYS; setRelay(h, true); setRelay((h + 1) % NUM_RELAYS, true); setRelay((h + 2) % NUM_RELAYS, true); effectStep++; break; }
    case 36: { int s = effectStep % (NUM_RELAYS * 2);
               if (s < NUM_RELAYS) for (int i=0; i<=s; i++) setRelay(i, true);
               else { int d = s - NUM_RELAYS; for (int i=0; i<=d; i++) setRelay(i, false); for (int i=d+1; i<NUM_RELAYS; i++) setRelay(i, true); }
               effectStep++; break; }
    case 37: allOff(); effA += random(-1, 2); if (effA < 0) effA = 0; if (effA >= NUM_RELAYS) effA = NUM_RELAYS - 1; setRelay(effA, true); break;
    case 38: { allOff(); int s = effectStep % 8; int p1 = s * 2; int p2 = (NUM_RELAYS - 1 - (s * 2));
               if (p1 < NUM_RELAYS) setRelay(p1, true); if (p2 >= 0 && p2 < NUM_RELAYS) setRelay(p2, true); effectStep++; break; }
    case 39: { int s = effectStep % 4; if (s < 2) allOn(); else allOff(); effectStep++; break; }

    // 40: DYNAMIC CUSTOM PATTERN (Loaded directly over MQTT / Web API)
    case EFFECT_CUSTOM: {
      if (customPatternCount > 0) {
        uint16_t mask = customPatternFrames[effectStep % customPatternCount];
        setRelayMask(mask);
        effectStep++;
      }
      break;
    }
  }
}

void handleAutoMode() {
  if (!autoMode || manualMode) return;
  if ((millis() - autoTimer) >= (unsigned long)autoCycleTime) {
    autoTimer = millis();
    int nextEff;
    if (autoShuffle) {
      do {
        nextEff = random(TOTAL_BUILTIN_FX);
      } while (nextEff == currentEffect);
    } else {
      nextEff = (currentEffect + 1) % TOTAL_BUILTIN_FX;
    }
    changeEffect(nextEff);
    publishStateMQTT();
  }
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

  bool staOk = (WiFi.status() == WL_CONNECTED);
  String staIpStr = staOk ? WiFi.localIP().toString() : "Not Connected";

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
  j += "\"temp\":" + String(getChipTemp(), 1) + ",";
  j += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  j += "\"uptime\":" + String(now / 1000) + ",";
  j += "\"mqtt\":{";
  j += "\"connected\":" + String(mqttClient.connected() ? 1 : 0) + ",";
  j += "\"broker\":\"" + mqtt_broker + "\",";
  j += "\"port\":" + String(mqtt_port);
  j += "},";
  j += "\"wifi\":{";
  j += "\"sta_ok\":" + String(staOk ? 1 : 0) + ",";
  j += "\"sta_ssid\":\"" + (staOk ? WiFi.SSID() : (sta_connecting ? "Connecting..." : "None")) + "\",";
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
    if (server.hasArg("v")) effectSpeed = constrain(server.arg("v").toInt(), 40, 1000);
    sendState();
    publishStateMQTT();
  });

  server.on("/api/cycle", HTTP_GET, []() {
    if (server.hasArg("v")) autoCycleTime = constrain(server.arg("v").toInt(), 3000, 60000);
    addCORSHeaders();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/cmd", HTTP_GET, []() {
    String a = server.arg("a");
    if (a == "all_on") { enterManual(); allOn(); }
    else if (a == "all_off") { enterManual(); allOff(); }
    else if (a == "toggle_run") { if (manualMode) exitManual(); else effectRunning = !effectRunning; }
    else if (a == "toggle_auto") { autoMode = !autoMode; if (autoMode) { manualMode = false; effectRunning = true; autoTimer = millis(); } }
    else if (a == "toggle_shuffle") { autoShuffle = !autoShuffle; }
    else if (a == "toggle_manual") { if (manualMode) exitManual(); else enterManual(); }
    else if (a == "next") { changeEffect((currentEffect + 1) % TOTAL_EFFECTS); }
    else if (a == "prev") { changeEffect((currentEffect - 1 + TOTAL_EFFECTS) % TOTAL_EFFECTS); }
    else if (a == "odd") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==0); }
    else if (a == "even") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, i%2==1); }
    else if (a == "invert") { enterManual(); for (int i=0; i<NUM_RELAYS; i++) setRelay(i, !relayState[i]); }
    sendState();
    publishStateMQTT();
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
      prefs.begin("relay_cfg", false);
      prefs.putString("ssid", s);
      prefs.putString("pass", p);
      prefs.end();
      sta_ssid = s; sta_pass = p;
      WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
      sta_connecting = true; sta_connect_start = millis();
    }
    sendState();
  });

  server.on("/api/wifi/reset", HTTP_GET, []() {
    prefs.begin("relay_cfg", false);
    prefs.remove("ssid"); prefs.remove("pass");
    prefs.end();
    sta_ssid = ""; sta_pass = "";
    WiFi.disconnect(false);
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

  // 2. Safe GPIO Init
  initRelayPins();
  Serial.println("[1] 16 GPIOs Latched OFF.");

  // 3. Restore Dynamic Custom Pattern from NVS
  restoreCustomPatternFromNVS();

  // 4. Start Dual WiFi
  initWiFi();

  // 5. Start Web Server & OTA
  setupWebServer();

  // 6. Start Outbound Cloud MQTT (No port forwarding needed!)
  initMQTT();

  // 7. Hardware Watchdog on Core 0
  wdTimer = millis();
  xTaskCreatePinnedToCore(wdTask, "WDT", 2048, NULL, 1, NULL, 0);

  // 8. Test pulse on Relay 1 (80ms click)
  setRelay(0, true);
  delay(80);
  setRelay(0, false);

  effectTimer    = millis();
  autoTimer      = millis();
  wifiTimer      = millis();
  mqttRetryTimer = millis();
  statePubTimer  = millis();

  Serial.println("=======================================================");
  Serial.printf(" READY! Device ID: %s\n", device_id.c_str());
  Serial.println(" MQTT Broker: broker.emqx.io:1883 (No Port Forwarding)");
  Serial.println(" Local mDNS : http://relay16.local");
  Serial.println(" Local AP   : http://192.168.4.1 (ESP32_Relay_Chaser)");
  Serial.println("=======================================================\n");
}

// ══════════════════════════════════════════════════════════════
//  MAIN EXECUTION LOOP
// ══════════════════════════════════════════════════════════════
void loop() {
  wdTimer = millis();

  // Handle local HTTP clients
  server.handleClient();

  // Handle Cloud MQTT
  checkMQTT();

  // Handle Continuous Auto Cycle
  handleAutoMode();

  // Handle Relay Animation Step
  runEffectStep();

  // Monitor Network
  checkWiFi();

  // ── Thermal Optimization: FreeRTOS idle waiti instruction ──
  delay(1);
}
