/*
 ════════════════════════════════════════════════════════════════
  ESP32 × UCTRONICS 16-CH RELAY v8.5 PRO (UNIFIED LOCAL + CLOUD)
 ════════════════════════════════════════════════════════════════
  Dual Operation Architecture:
    1. Local Embedded Cyberpunk Web Controller (Port 80)
       - SoftAP Hotspot: SSID "ESP32_Relay" / "12345678" -> http://192.168.4.1
       - Local Station: SSID "loki" / "lokijiowifi"     -> http://<station-ip>
    2. Cloud HTTPS Remote Sync (Vercel Serverless + MongoDB Atlas)
       - Target: https://esp-32-16-ch-cloud-chaser.vercel.app
       - Device ID: esp32_c8ba49e4
       - Device Secret: a620f6f6fc444710b7f1253b00e011d8
       - Two-way real-time telemetry push & command polling

  Hardware:
    Board: UCTRONICS 16-Channel Relay (12V Coil, 5V Active-LOW)
    Logic: 3.3V Control with Optocoupler Isolation
    Pins:  GPIO 25, 4, 5, 26, 13, 27, 15, 32, 16, 17, 18, 19, 21, 22, 23, 33
 ════════════════════════════════════════════════════════════════
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "web_page.h"

// ── WiFi Credentials ──────────────────────────────────────────
const char* AP_SSID          = "ESP32_Relay";
const char* AP_PASSWORD      = "12345678";
const char* STA_SSID         = "loki";
const char* STA_PASSWORD     = "lokijiowifi";

// ── Cloud API Endpoints & Device Credentials ──────────────────
const char* CLOUD_BASE_URL   = "https://esp-32-16-ch-cloud-chaser.vercel.app";
const char* DEVICE_ID        = "esp32_c8ba49e4";
const char* DEVICE_SECRET    = "a620f6f6fc444710b7f1253b00e011d8";
const char* FIRMWARE_VER     = "2.0.0-http";

// ── GPIO Pin Mapping (Boot-safe, glitch-free) ─────────────────
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

#define NUM_RELAYS    16
#define TOTAL_EFFECTS 25
#define RELAY_ON      LOW   // Active-LOW logic
#define RELAY_OFF     HIGH

// ── System Operating Modes ────────────────────────────────────
enum SystemMode {
  MODE_AUTO,      // Automatically cycles through preset playlist
  MODE_EFFECT,    // Loops single selected effect
  MODE_MANUAL,    // Manual channel control (effects halted)
  MODE_OFF        // Master Off / Standby
};

SystemMode currentMode = MODE_AUTO;

// ── Effect Preset Playlists ───────────────────────────────────
#define TOTAL_PRESETS 5
struct PresetDefinition {
  const char* name;
  const uint8_t effects[TOTAL_EFFECTS];
  uint8_t count;
};

const PresetDefinition PRESETS[TOTAL_PRESETS] = {
  // Preset 0: Grand Showcase (all 25 effects)
  { "🌟 Grand Showcase (All 25)",
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24}, 25 },
  // Preset 1: High Energy & Strobes
  { "⚡ High Energy & Strobes",
    {6, 8, 11, 15, 22}, 5 },
  // Preset 2: Smooth Waves & Chasers
  { "🌊 Waves & Smooth Chasers",
    {0, 1, 2, 3, 4, 12, 18}, 7 },
  // Preset 3: Atmospheric & Fireworks
  { "🌧️ Atmospheric & Fireworks",
    {14, 16, 17, 19, 24}, 5 },
  // Preset 4: Matrix & Dual Bounce
  { "🎭 Matrix & Dual Bounce",
    {5, 7, 13, 20, 21}, 5 }
};

int currentPresetIndex = 0;
int presetStepIndex    = 0;

// ── State Variables ───────────────────────────────────────────
bool relayState[NUM_RELAYS];
int  currentEffect    = 0;
int  effectSpeed      = 120;   // Milliseconds per step (20ms - 500ms)
int  autoCycleTime    = 8000;  // Milliseconds per effect in auto cycle (2s - 30s)

// Non-blocking timers
unsigned long effectTimer    = 0;
unsigned long autoTimer      = 0;
unsigned long wifiRetryTimer = 0;
unsigned long cloudPollTimer = 0;
unsigned long cloudPushTimer = 0;
bool          cloudStatusDirty = true;

// Internal effect sequencer state variables (Cleanly reset on effect change)
int effectStep        = 0;
int effectDir         = 1;
int fx_hb             = 0;
int fx_firework_b     = 0;
int fx_firework_fc    = 8;
int fx_cross_pA       = 0;
int fx_cross_pB       = 8;
int fx_cross_dA       = 1;
int fx_cross_dB       = -1;
int fx_stack_top      = NUM_RELAYS - 1;
int fx_stack_ball     = 0;
int fx_phase          = 0;

WebServer server(80);

// ── Effect Names ──────────────────────────────────────────────
const char* EFFECT_NAMES[TOTAL_EFFECTS] = {
  "Left Chaser",             //  0
  "Right Chaser",            //  1
  "Bounce & Ping-Pong",      //  2
  "Center Out Expansion",    //  3
  "Edges In Convergence",    //  4
  "Zip and Unzip",           //  5
  "Double Dot Runner",       //  6
  "Binary Counter 16-Bit",   //  7
  "Random Sparkle Burst",    //  8
  "Smooth Twinkle Glow",     //  9
  "Wave Progressive Fill",   // 10
  "Heartbeat Pulse",         // 11
  "Cylon / Kitt Scanner",    // 12
  "Alternating Pairs",       // 13
  "Cascade Waterfall",       // 14
  "Theater 3-Phase Chaser",  // 15
  "Raindrop Simulation",     // 16
  "Fireworks Explosion",     // 17
  "Knight Rider Scanner",    // 18
  "Meteor Trail Dash",       // 19
  "Dual Ping-Pong Bounce",   // 20
  "Cross-Fade Breathing",    // 21
  "Multi-Strobe Flash",      // 22
  "Wipe and Fill",           // 23
  "Reverse Stack Builder"    // 24
};

// ══════════════════════════════════════════════════════════════
//  GLITCH-FREE GPIO INITIALIZATION
//  Prevents any spurious LOW pulse during ESP32 power-up
// ══════════════════════════════════════════════════════════════
void initRelayPins() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], INPUT_PULLUP);
    relayState[i] = false;
  }
  delayMicroseconds(200);
  for (int i = 0; i < NUM_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
    pinMode(RELAY_PINS[i], OUTPUT);
  }
}

// ══════════════════════════════════════════════════════════════
//  HARDWARE RELAY CONTROL
// ══════════════════════════════════════════════════════════════
void setRelay(int ch, bool on) {
  if (ch < 0 || ch >= NUM_RELAYS) return;
  relayState[ch] = on;
  digitalWrite(RELAY_PINS[ch], on ? RELAY_ON : RELAY_OFF);
}

void allOff() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, false);
  }
}

void allOn() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, true);
  }
}

void resetEffectState() {
  effectStep        = 0;
  effectDir         = 1;
  fx_hb             = 0;
  fx_firework_b     = 0;
  fx_firework_fc    = 8;
  fx_cross_pA       = 0;
  fx_cross_pB       = 8;
  fx_cross_dA       = 1;
  fx_cross_dB       = -1;
  fx_stack_top      = NUM_RELAYS - 1;
  fx_stack_ball     = 0;
  fx_phase          = 0;
  allOff();
}

void selectEffect(int id) {
  if (id < 0 || id >= TOTAL_EFFECTS) return;
  currentEffect = id;
  resetEffectState();
  currentMode = MODE_EFFECT;
  effectTimer = millis();
  cloudStatusDirty = true;
}

void selectPreset(int id) {
  if (id < 0 || id >= TOTAL_PRESETS) return;
  currentPresetIndex = id;
  presetStepIndex    = 0;
  currentEffect      = PRESETS[currentPresetIndex].effects[0];
  resetEffectState();
  currentMode        = MODE_AUTO;
  autoTimer          = millis();
  effectTimer        = millis();
  cloudStatusDirty   = true;
}

void nextEffectInCycle() {
  const PresetDefinition& p = PRESETS[currentPresetIndex];
  presetStepIndex = (presetStepIndex + 1) % p.count;
  currentEffect   = p.effects[presetStepIndex];
  resetEffectState();
  autoTimer = millis();
  cloudStatusDirty = true;
}

void prevEffectInCycle() {
  const PresetDefinition& p = PRESETS[currentPresetIndex];
  presetStepIndex = (presetStepIndex - 1 + p.count) % p.count;
  currentEffect   = p.effects[presetStepIndex];
  resetEffectState();
  autoTimer = millis();
  cloudStatusDirty = true;
}

// ══════════════════════════════════════════════════════════════
//  25 FULLY NON-BLOCKING LIGHTING EFFECTS
// ══════════════════════════════════════════════════════════════
void fx0_leftChaser() {
  allOff();
  setRelay(effectStep, true);
  effectStep = (effectStep + 1) % NUM_RELAYS;
}

void fx1_rightChaser() {
  allOff();
  setRelay(NUM_RELAYS - 1 - effectStep, true);
  effectStep = (effectStep + 1) % NUM_RELAYS;
}

void fx2_bounce() {
  allOff();
  setRelay(effectStep, true);
  effectStep += effectDir;
  if (effectStep >= NUM_RELAYS - 1) { effectStep = NUM_RELAYS - 1; effectDir = -1; }
  else if (effectStep <= 0)         { effectStep = 0;              effectDir =  1; }
}

void fx3_centerOut() {
  allOff();
  int l = 7 - effectStep;
  int r = 8 + effectStep;
  if (l >= 0)          setRelay(l, true);
  if (r <  NUM_RELAYS) setRelay(r, true);
  effectStep = (effectStep + 1) % 8;
}

void fx4_edgesIn() {
  allOff();
  int l = effectStep;
  int r = NUM_RELAYS - 1 - effectStep;
  if (l <= 7)          setRelay(l, true);
  if (r >= 8)          setRelay(r, true);
  effectStep = (effectStep + 1) % 8;
}

void fx5_zipUnzip() {
  if (effectStep < NUM_RELAYS) {
    setRelay(effectStep, true);
  } else {
    setRelay(effectStep - NUM_RELAYS, false);
  }
  effectStep = (effectStep + 1) % (NUM_RELAYS * 2);
}

void fx6_doubleDot() {
  allOff();
  setRelay(effectStep, true);
  setRelay((effectStep + 8) % NUM_RELAYS, true);
  effectStep = (effectStep + 1) % NUM_RELAYS;
}

void fx7_binaryCounter() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, (effectStep >> i) & 1);
  }
  effectStep++;
  if (effectStep > 65535) effectStep = 0;
}

void fx8_randomSparkle() {
  allOff();
  int count = random(2, 6);
  for (int i = 0; i < count; i++) {
    setRelay(random(0, NUM_RELAYS), true);
  }
}

void fx9_smoothTwinkle() {
  setRelay(random(0, NUM_RELAYS), random(0, 2) == 1);
}

void fx10_waveFill() {
  if (effectStep < NUM_RELAYS) {
    setRelay(effectStep, true);
  } else {
    setRelay(NUM_RELAYS - 1 - (effectStep - NUM_RELAYS), false);
  }
  effectStep = (effectStep + 1) % (NUM_RELAYS * 2);
}

void fx11_heartbeat() {
  allOff();
  if (fx_hb == 0) {
    for (int i = 5; i <= 10; i++) setRelay(i, true);
    fx_hb = 1;
  } else if (fx_hb == 1) {
    allOff();
    fx_hb = 2;
  } else if (fx_hb == 2) {
    allOn();
    fx_hb = 3;
  } else if (fx_hb == 3) {
    for (int i = 5; i <= 10; i++) setRelay(i, true);
    fx_hb = 4;
  } else {
    allOff();
    fx_hb = 0;
  }
}

void fx12_cylon() {
  allOff();
  for (int w = -1; w <= 1; w++) {
    int p = effectStep + w;
    if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
  }
  effectStep += effectDir;
  if (effectStep >= NUM_RELAYS - 1) { effectStep = NUM_RELAYS - 1; effectDir = -1; }
  else if (effectStep <= 0)         { effectStep = 0;              effectDir =  1; }
}

void fx13_alternating() {
  bool odd = (effectStep % 2 == 0);
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, (i % 2 == 0) ? odd : !odd);
  }
  effectStep = (effectStep + 1) % 2;
}

void fx14_cascade() {
  allOff();
  for (int i = 0; i < 4; i++) {
    int p = (effectStep - i + NUM_RELAYS) % NUM_RELAYS;
    setRelay(p, true);
  }
  effectStep = (effectStep + 1) % NUM_RELAYS;
}

void fx15_theater() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, ((i + effectStep) % 3) == 0);
  }
  effectStep = (effectStep + 1) % 3;
}

void fx16_raindrop() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (relayState[i] && random(0, 3) == 0) setRelay(i, false);
  }
  if (random(0, 2) == 0) setRelay(random(0, NUM_RELAYS), true);
}

void fx17_fireworks() {
  allOff();
  if (fx_firework_b < 16) {
    int p = fx_firework_fc + (fx_firework_b % 2 == 0 ? fx_firework_b / 2 : -(fx_firework_b / 2));
    if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
    fx_firework_b++;
  } else {
    allOn();
    fx_firework_b = 0;
    fx_firework_fc = random(3, 13);
  }
}

void fx18_knightRider() {
  allOff();
  for (int w = -2; w <= 2; w++) {
    int p = effectStep + w;
    if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
  }
  effectStep += effectDir;
  if (effectStep >= NUM_RELAYS - 1) { effectStep = NUM_RELAYS - 1; effectDir = -1; }
  else if (effectStep <= 0)         { effectStep = 0;              effectDir =  1; }
}

void fx19_meteor() {
  allOff();
  for (int i = 0; i < 4; i++) {
    int p = effectStep - i;
    if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
  }
  effectStep = (effectStep + 1) % (NUM_RELAYS + 4);
}

void fx20_dualBounce() {
  allOff();
  setRelay(effectStep, true);
  setRelay(NUM_RELAYS - 1 - effectStep, true);
  effectStep += effectDir;
  if (effectStep >= 7)      { effectStep = 7; effectDir = -1; }
  else if (effectStep <= 0) { effectStep = 0; effectDir =  1; }
}

void fx21_crossFade() {
  allOff();
  setRelay(fx_cross_pA, true);
  setRelay(fx_cross_pB, true);
  fx_cross_pA += fx_cross_dA;
  if (fx_cross_pA >= NUM_RELAYS - 1) { fx_cross_pA = NUM_RELAYS - 1; fx_cross_dA = -1; }
  else if (fx_cross_pA <= 0)         { fx_cross_pA = 0;              fx_cross_dA =  1; }
  fx_cross_pB += fx_cross_dB;
  if (fx_cross_pB >= NUM_RELAYS - 1) { fx_cross_pB = NUM_RELAYS - 1; fx_cross_dB = -1; }
  else if (fx_cross_pB <= 0)         { fx_cross_pB = 0;              fx_cross_dB =  1; }
}

void fx22_strobe() {
  fx_phase = !fx_phase;
  for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, fx_phase);
}

void fx23_wipeFill() {
  if (effectStep < NUM_RELAYS) {
    setRelay(effectStep, true);
  } else {
    setRelay(effectStep - NUM_RELAYS, false);
  }
  effectStep = (effectStep + 1) % (NUM_RELAYS * 2);
}

void fx24_stackBuild() {
  for (int i = fx_stack_top + 1; i < NUM_RELAYS; i++) {
    setRelay(i, true);
  }
  for (int i = 0; i <= fx_stack_top; i++) {
    setRelay(i, (i == fx_stack_ball));
  }
  fx_stack_ball++;
  if (fx_stack_ball > fx_stack_top) {
    fx_stack_ball = 0;
    fx_stack_top--;
    if (fx_stack_top < 0) {
      fx_stack_top = NUM_RELAYS - 1;
      allOff();
    }
  }
}

void runCurrentEffect() {
  switch (currentEffect) {
    case  0: fx0_leftChaser();    break;
    case  1: fx1_rightChaser();   break;
    case  2: fx2_bounce();        break;
    case  3: fx3_centerOut();     break;
    case  4: fx4_edgesIn();       break;
    case  5: fx5_zipUnzip();      break;
    case  6: fx6_doubleDot();     break;
    case  7: fx7_binaryCounter(); break;
    case  8: fx8_randomSparkle(); break;
    case  9: fx9_smoothTwinkle(); break;
    case 10: fx10_waveFill();     break;
    case 11: fx11_heartbeat();    break;
    case 12: fx12_cylon();        break;
    case 13: fx13_alternating();  break;
    case 14: fx14_cascade();      break;
    case 15: fx15_theater();      break;
    case 16: fx16_raindrop();     break;
    case 17: fx17_fireworks();    break;
    case 18: fx18_knightRider();  break;
    case 19: fx19_meteor();       break;
    case 20: fx20_dualBounce();   break;
    case 21: fx21_crossFade();    break;
    case 22: fx22_strobe();       break;
    case 23: fx23_wipeFill();     break;
    case 24: fx24_stackBuild();   break;
    default: fx0_leftChaser();    break;
  }
}

// ══════════════════════════════════════════════════════════════
//  LOCAL JSON STATE SERIALIZER
// ══════════════════════════════════════════════════════════════
void addCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void sendStateJson() {
  addCorsHeaders();
  String s = "{";
  s += "\"e\":" + String(currentEffect) + ",";
  s += "\"ename\":\"" + String(EFFECT_NAMES[currentEffect]) + "\",";
  s += "\"s\":" + String(effectSpeed) + ",";
  s += "\"m\":\"" + String(currentMode == MODE_AUTO ? "auto" :
                           currentMode == MODE_EFFECT ? "effect" :
                           currentMode == MODE_MANUAL ? "manual" : "off") + "\",";
  s += "\"c\":" + String(autoCycleTime / 1000) + ",";
  s += "\"p\":" + String(currentPresetIndex) + ",";
  s += "\"pn\":\"" + String(PRESETS[currentPresetIndex].name) + "\",";
  s += "\"ap\":\"" + WiFi.softAPIP().toString() + "\",";
  s += "\"sta\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0") + "\",";
  s += "\"ch\":[";
  for (int i = 0; i < NUM_RELAYS; i++) {
    s += String(relayState[i] ? 1 : 0);
    if (i < NUM_RELAYS - 1) s += ",";
  }
  s += "]}";
  server.send(200, "application/json", s);
}

// ══════════════════════════════════════════════════════════════
//  LOCAL HTTP SERVER HANDLERS (PORT 80)
// ══════════════════════════════════════════════════════════════
void handleRoot() {
  addCorsHeaders();
  String pg = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>"
                "<title>ESP32 16-CH Relay PRO</title>");
  pg += FPSTR(PCSS);
  pg += F("</head><body>");
  pg += FPSTR(PHTML);
  pg += FPSTR(PJS);
  pg += F("</body></html>");
  server.send(200, "text/html", pg);
}

void handlePreset() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    selectPreset(id);
  }
  sendStateJson();
}

void handleEffect() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    selectEffect(id);
  }
  sendStateJson();
}

void handleRelay() {
  if (server.hasArg("ch") && server.hasArg("v")) {
    int ch = server.arg("ch").toInt();
    int v  = server.arg("v").toInt();
    currentMode = MODE_MANUAL;
    setRelay(ch, v == 1);
    cloudStatusDirty = true;
  }
  sendStateJson();
}

void handleCmd() {
  if (server.hasArg("a")) {
    String a = server.arg("a");
    if (a == "all_on") {
      currentMode = MODE_MANUAL;
      allOn();
    } else if (a == "all_off") {
      currentMode = MODE_OFF;
      allOff();
    } else if (a == "toggle_run") {
      if (currentMode == MODE_OFF) {
        currentMode = MODE_AUTO;
        autoTimer = millis();
      } else {
        currentMode = MODE_OFF;
        allOff();
      }
    } else if (a == "toggle_auto") {
      if (currentMode == MODE_AUTO) {
        currentMode = MODE_EFFECT;
      } else {
        currentMode = MODE_AUTO;
        autoTimer = millis();
      }
    } else if (a == "toggle_manual") {
      if (currentMode == MODE_MANUAL) {
        currentMode = MODE_AUTO;
        autoTimer = millis();
      } else {
        currentMode = MODE_MANUAL;
      }
    } else if (a == "next") {
      nextEffectInCycle();
    } else if (a == "prev") {
      prevEffectInCycle();
    } else if (a == "odd") {
      currentMode = MODE_MANUAL;
      for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, (i % 2 == 0));
    } else if (a == "even") {
      currentMode = MODE_MANUAL;
      for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, (i % 2 == 1));
    } else if (a == "invert") {
      currentMode = MODE_MANUAL;
      for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, !relayState[i]);
    }
    cloudStatusDirty = true;
  }
  sendStateJson();
}

void handleSpeed() {
  if (server.hasArg("v")) {
    effectSpeed = constrain(server.arg("v").toInt(), 20, 500);
    cloudStatusDirty = true;
  }
  sendStateJson();
}

void handleCycle() {
  if (server.hasArg("v")) {
    autoCycleTime = constrain(server.arg("v").toInt(), 2000, 30000);
    cloudStatusDirty = true;
  }
  sendStateJson();
}

void handleNotFound() {
  addCorsHeaders();
  server.send(404, "text/plain", "Not Found");
}

// ══════════════════════════════════════════════════════════════
//  CLOUD HTTPS CLIENT: TELEMETRY PUSH & COMMAND POLLING
// ══════════════════════════════════════════════════════════════
void pushCloudStatus() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation for lightweight embedded TLS

  HTTPClient https;
  String url = String(CLOUD_BASE_URL) + "/api/esp/status";

  if (!https.begin(client, url)) {
    Serial.println("[CLOUD] Failed to begin HTTPS connection for status push");
    return;
  }

  https.addHeader("Content-Type", "application/json");
  https.addHeader("X-Device-Id", DEVICE_ID);
  https.addHeader("X-Device-Secret", DEVICE_SECRET);
  https.setTimeout(3500);

  // Build JSON payload
  StaticJsonDocument<1024> doc;
  doc["deviceId"]    = DEVICE_ID;
  doc["mode"]        = (currentMode == MODE_AUTO) ? "auto" :
                       (currentMode == MODE_MANUAL) ? "manual" : "stopped";
  doc["effect"]      = EFFECT_NAMES[currentEffect];
  doc["effectIndex"] = currentEffect;
  doc["cycle"]       = presetStepIndex + 1;
  doc["totalCycles"] = PRESETS[currentPresetIndex].count;

  const PresetDefinition& p = PRESETS[currentPresetIndex];
  int nextIdx = (presetStepIndex + 1) % p.count;
  doc["nextEffect"]  = EFFECT_NAMES[p.effects[nextIdx]];

  doc["speed"]       = effectSpeed;
  doc["uptime"]      = millis() / 1000;
  doc["rssi"]        = WiFi.RSSI();
  doc["heap"]        = ESP.getFreeHeap();
  doc["minHeap"]     = ESP.getMinFreeHeap();
  doc["ip"]          = WiFi.localIP().toString();
  doc["firmware"]    = FIRMWARE_VER;
  doc["presetIndex"] = currentPresetIndex;
  doc["presetName"]  = PRESETS[currentPresetIndex].name;

  JsonArray relays = doc.createNestedArray("relays");
  for (int i = 0; i < NUM_RELAYS; i++) {
    relays.add(relayState[i] ? 1 : 0);
  }

  String jsonString;
  serializeJson(doc, jsonString);

  int httpCode = https.POST(jsonString);
  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
    cloudStatusDirty = false;
  } else {
    Serial.printf("[CLOUD] Status push code: %d\n", httpCode);
  }
  https.end();
}

void pollCloudCommands() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = String(CLOUD_BASE_URL) + "/api/esp/poll";

  if (!https.begin(client, url)) return;

  https.addHeader("X-Device-Id", DEVICE_ID);
  https.addHeader("X-Device-Secret", DEVICE_SECRET);
  https.setTimeout(3000);

  int httpCode = https.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = https.getString();
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.containsKey("commands")) {
      JsonArray commands = doc["commands"].as<JsonArray>();
      for (JsonObject item : commands) {
        String cmd = item["cmd"] | "";
        JsonObject p = item["payload"].as<JsonObject>();

        Serial.printf("[CLOUD] Command Received: %s\n", cmd.c_str());

        if (cmd == "SET_RELAY") {
          int ch = p["channel"] | (p["ch"] | (p["relay"] | -1));
          bool st = p["state"] | (p["v"] | 1);
          if (ch >= 0 && ch < NUM_RELAYS) {
            currentMode = MODE_MANUAL;
            setRelay(ch, st);
            cloudStatusDirty = true;
          } else if (p.containsKey("bitmask")) {
            uint16_t mask = p["bitmask"];
            currentMode = MODE_MANUAL;
            for (int i = 0; i < NUM_RELAYS; i++) {
              setRelay(i, (mask >> i) & 1);
            }
            cloudStatusDirty = true;
          } else if (p.containsKey("invert")) {
            currentMode = MODE_MANUAL;
            for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, !relayState[i]);
            cloudStatusDirty = true;
          }
        } else if (cmd == "MASTER_ON") {
          currentMode = MODE_MANUAL;
          allOn();
          cloudStatusDirty = true;
        } else if (cmd == "MASTER_OFF" || cmd == "EMERGENCY_STOP" || cmd == "STOP") {
          currentMode = MODE_OFF;
          allOff();
          cloudStatusDirty = true;
        } else if (cmd == "SET_EFFECT") {
          int fx = p["effectIndex"] | (p["id"] | 0);
          selectEffect(fx);
        } else if (cmd == "SET_PRESET") {
          int pr = p["presetIndex"] | (p["id"] | 0);
          selectPreset(pr);
        } else if (cmd == "SET_SPEED") {
          int spd = p["speed"] | (p["v"] | 120);
          effectSpeed = constrain(spd, 20, 500);
          cloudStatusDirty = true;
        } else if (cmd == "SET_MODE") {
          String m = p["mode"] | "";
          if (m == "auto") {
            currentMode = MODE_AUTO;
            autoTimer = millis();
          } else if (m == "manual") {
            currentMode = MODE_MANUAL;
          } else if (m == "stopped" || m == "off") {
            currentMode = MODE_OFF;
            allOff();
          }
          cloudStatusDirty = true;
        } else if (cmd == "PAUSE") {
          currentMode = MODE_OFF;
          allOff();
          cloudStatusDirty = true;
        } else if (cmd == "RESUME") {
          currentMode = MODE_AUTO;
          autoTimer = millis();
          cloudStatusDirty = true;
        } else if (cmd == "NEXT_EFFECT") {
          nextEffectInCycle();
        } else if (cmd == "PREV_EFFECT") {
          prevEffectInCycle();
        } else if (cmd == "REQUEST_STATE") {
          cloudStatusDirty = true;
        }
      }
    }
  }
  https.end();
}

// ══════════════════════════════════════════════════════════════
//  SETUP & MAIN LOOP (Zero Blocking Delays)
// ══════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n==================================================");
  Serial.println("  ESP32 16-CH RELAY CONTROLLER v8.5 PRO");
  Serial.printf("  Device ID    : %s\n", DEVICE_ID);
  Serial.printf("  Cloud Server : %s\n", CLOUD_BASE_URL);
  Serial.println("==================================================");

  // 1. Initialize Relays safely (Active LOW, all forced OFF)
  initRelayPins();
  Serial.println("[1] 16-Channel Hardware Relays initialized OFF.");

  // 2. Configure WiFi: SoftAP + Station Mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("[2] SoftAP Hotspot '%s' active: %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());

  WiFi.begin(STA_SSID, STA_PASSWORD);
  Serial.printf("[3] Connecting to Home WiFi '%s'...\n", STA_SSID);

  // 3. Start Local Embedded Web Server
  server.on("/", handleRoot);
  server.on("/state", sendStateJson);
  server.on("/preset", handlePreset);
  server.on("/effect", handleEffect);
  server.on("/relay", handleRelay);
  server.on("/cmd", handleCmd);
  server.on("/speed", handleSpeed);
  server.on("/cycle", handleCycle);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[4] Local Web Server active on port 80.");

  // 4. Initialize Hardware Watchdog (30 seconds, ESP-IDF v5 API)
  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = 30000,
      .idle_core_mask = 0,
      .trigger_panic = true
  };
  esp_task_wdt_init(&twdt_config);
  esp_task_wdt_add(NULL);
  Serial.println("[5] Hardware Watchdog configured (30s).");

  // 5. Seed Random Generator
  randomSeed(esp_random());

  // 6. Start Initial Effect
  currentEffect = PRESETS[currentPresetIndex].effects[0];
  resetEffectState();
  autoTimer   = millis();
  effectTimer = millis();

  Serial.println("[6] Non-blocking lighting engine started.");
  Serial.println("==================================================\n");
}

void loop() {
  // Feed Hardware Watchdog
  esp_task_wdt_reset();

  unsigned long now = millis();

  // 1. Service Local HTTP Web Server Clients
  server.handleClient();

  // 2. Wi-Fi Auto-Reconnect Logic (Check every 10s)
  if (WiFi.status() != WL_CONNECTED) {
    if (now - wifiRetryTimer >= 10000) {
      wifiRetryTimer = now;
      WiFi.reconnect();
    }
  }

  // 3. Cloud Command Polling (Every 1200ms when WiFi is connected)
  if (now - cloudPollTimer >= 1200) {
    cloudPollTimer = now;
    pollCloudCommands();
  }

  // 4. Cloud Telemetry Status Push (Every 2500ms or immediately when dirty)
  if (cloudStatusDirty || (now - cloudPushTimer >= 2500)) {
    cloudPushTimer = now;
    pushCloudStatus();
  }

  // 5. Autonomous Cycle Sequencer (Advances effect when in AUTO mode)
  if (currentMode == MODE_AUTO) {
    if (now - autoTimer >= (unsigned long)autoCycleTime) {
      nextEffectInCycle();
    }
  }

  // 6. Effect Stepping Timer (Non-blocking lighting animation)
  if (currentMode == MODE_AUTO || currentMode == MODE_EFFECT) {
    if (now - effectTimer >= (unsigned long)effectSpeed) {
      effectTimer = now;
      runCurrentEffect();
    }
  }
}
