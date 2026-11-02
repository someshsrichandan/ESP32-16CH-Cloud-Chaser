/*
 * ESP32 x UCTRONICS 16-CH RELAY v8.5 PRO
 * Non-Blocking Effects Engine, 5 Presets, Dual WiFi (SoftAP + Home Station)
 * Embedded Cyberpunk Web Controller
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_task_wdt.h>
#include "web_page.h"

// --- WiFi Credentials ---
const char* AP_SSID       = "ESP32_Relay";
const char* AP_PASSWORD   = "12345678";
const char* STA_SSID      = "loki";
const char* STA_PASSWORD  = "lokijiowifi";

// --- GPIO Pin Mapping (Boot-safe, glitch-free) ---
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

// --- System Operating Modes ---
enum SystemMode {
  MODE_AUTO,      // Automatically cycles through preset playlist
  MODE_EFFECT,    // Loops single selected effect
  MODE_MANUAL,    // Manual channel control (effects halted)
  MODE_OFF        // Master Off / Standby
};

SystemMode currentMode = MODE_AUTO;

// --- Effect Preset Playlists ---
#define TOTAL_PRESETS 5
struct PresetDefinition {
  const char* name;
  uint8_t count;
  uint8_t effects[TOTAL_EFFECTS];
};

const PresetDefinition PRESETS[TOTAL_PRESETS] = {
  // Preset 0: Grand Showcase (all 25 effects)
  { "Grand Showcase (All 25)",
    25,
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24} },
  // Preset 1: High Energy & Strobes
  { "High Energy & Strobes",
    5,
    {6, 8, 11, 15, 22} },
  // Preset 2: Smooth Waves & Chasers
  { "Waves & Smooth Chasers",
    7,
    {0, 1, 2, 3, 4, 12, 18} },
  // Preset 3: Atmospheric & Fireworks
  { "Atmospheric & Fireworks",
    5,
    {14, 16, 17, 19, 24} },
  // Preset 4: Matrix & Alternating
  { "Matrix & Dual Bounce",
    5,
    {5, 7, 13, 20, 21} }
};

int currentPresetIndex = 0;
int presetStepIndex    = 0;

// --- State Variables ---
bool relayState[NUM_RELAYS];
int  currentEffect    = 0;
int  effectSpeed      = 120;   // Milliseconds per step (20ms - 500ms)
int  autoCycleTime    = 8000;  // Milliseconds per effect in auto cycle (2s - 30s)

// Non-blocking timers
unsigned long effectTimer = 0;
unsigned long autoTimer   = 0;
unsigned long wifiTimer   = 0;
unsigned long wdTimer     = 0;

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

// --- Hardware Relay Control Functions ---
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
}

void nextEffectInCycle() {
  const PresetDefinition& p = PRESETS[currentPresetIndex];
  presetStepIndex = (presetStepIndex + 1) % p.count;
  currentEffect   = p.effects[presetStepIndex];
  resetEffectState();
  autoTimer = millis();
}

void prevEffectInCycle() {
  const PresetDefinition& p = PRESETS[currentPresetIndex];
  presetStepIndex = (presetStepIndex - 1 + p.count) % p.count;
  currentEffect   = p.effects[presetStepIndex];
  resetEffectState();
  autoTimer = millis();
}

void selectPreset(int pIdx) {
  if (pIdx < 0 || pIdx >= TOTAL_PRESETS) return;
  currentPresetIndex = pIdx;
  presetStepIndex    = 0;
  currentEffect      = PRESETS[currentPresetIndex].effects[0];
  resetEffectState();
  currentMode = MODE_AUTO;
  autoTimer   = millis();
}

// --- Glitch-Free GPIO Initialization ---
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

// --- Non-Blocking Dual WiFi (SoftAP + Home Station) ---
void setupWiFi() {
  Serial.println("[WiFi] Starting Dual Mode (AP + Station)...");
  WiFi.mode(WIFI_AP_STA);

  // SoftAP starts immediately (< 50ms)
  WiFi.softAP(AP_SSID, AP_PASSWORD, 1, 0, 4);
  Serial.print("[WiFi] SoftAP IP: ");
  Serial.println(WiFi.softAPIP());

  // Station connects in background
  if (STA_SSID != nullptr && strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASSWORD);
    Serial.printf("[WiFi] Connecting to Home Station: %s\n", STA_SSID);
  }
}

void checkWiFiNonBlocking() {
  unsigned long now = millis();
  if (now - wifiTimer < 5000) return;
  wifiTimer = now;

  if (WiFi.status() == WL_CONNECTED) {
    static bool logged = false;
    if (!logged) {
      Serial.print("[WiFi] Connected to Home WiFi! Station IP: ");
      Serial.println(WiFi.localIP());
      logged = true;
    }
  } else {
    static unsigned long lastReconnect = 0;
    if (now - lastReconnect > 15000) {
      lastReconnect = now;
      if (STA_SSID != nullptr && strlen(STA_SSID) > 0) {
        WiFi.begin(STA_SSID, STA_PASSWORD);
      }
    }
  }
}

// --- 25 Non-Blocking Effects ---
void runEffectStep() {
  if (currentMode == MODE_MANUAL || currentMode == MODE_OFF) {
    return;
  }

  unsigned long now = millis();
  if (now - effectTimer < (unsigned long)effectSpeed) {
    return;
  }
  effectTimer = now;

  switch (currentEffect) {
    // 0: Left Chaser
    case 0:
      allOff();
      setRelay(effectStep % NUM_RELAYS, true);
      effectStep++;
      break;

    // 1: Right Chaser
    case 1:
      allOff();
      setRelay(NUM_RELAYS - 1 - (effectStep % NUM_RELAYS), true);
      effectStep++;
      break;

    // 2: Bounce
    case 2:
      allOff();
      setRelay(effectStep, true);
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) effectDir = -1;
      if (effectStep <= 0) effectDir = 1;
      break;

    // 3: Center Out
    case 3: {
      allOff();
      int c = NUM_RELAYS / 2;
      int s = effectStep % (c + 1);
      for (int i = c - s; i <= c + s && i < NUM_RELAYS; i++) {
        if (i >= 0) setRelay(i, true);
      }
      effectStep++;
      if (effectStep > c * 2) effectStep = 0;
      break;
    }

    // 4: Edges In
    case 4: {
      allOff();
      int s = effectStep % (NUM_RELAYS / 2 + 1);
      for (int i = 0; i <= s; i++) setRelay(i, true);
      for (int i = NUM_RELAYS - 1; i >= NUM_RELAYS - 1 - s; i--) setRelay(i, true);
      effectStep++;
      if (effectStep > NUM_RELAYS / 2 + 1) effectStep = 0;
      break;
    }

    // 5: Zip + Unzip
    case 5: {
      int h = NUM_RELAYS / 2;
      allOff();
      if (effectStep < h) {
        setRelay(h - 1 - effectStep, true);
        setRelay(h + effectStep, true);
      } else {
        int s = effectStep - h;
        setRelay(s, true);
        setRelay(NUM_RELAYS - 1 - s, true);
      }
      effectStep++;
      if (effectStep >= NUM_RELAYS) effectStep = 0;
      break;
    }

    // 6: Double Dot Chaser
    case 6:
      allOff();
      setRelay(effectStep % NUM_RELAYS, true);
      setRelay((effectStep + 1) % NUM_RELAYS, true);
      effectStep++;
      break;

    // 7: Binary Counter
    case 7:
      for (int i = 0; i < NUM_RELAYS; i++) {
        setRelay(i, (effectStep >> i) & 1);
      }
      effectStep++;
      if (effectStep > 65535) effectStep = 0;
      break;

    // 8: Sparkle
    case 8:
      allOff();
      for (int i = 0; i < random(3, 8); i++) {
        setRelay(random(NUM_RELAYS), true);
      }
      break;

    // 9: Twinkle
    case 9:
      setRelay(random(NUM_RELAYS), (random(2) == 0));
      break;

    // 10: Wave Fill
    case 10: {
      int s = effectStep % (NUM_RELAYS * 2);
      if (s < NUM_RELAYS) setRelay(s, true);
      else setRelay(s - NUM_RELAYS, false);
      effectStep++;
      break;
    }

    // 11: Heartbeat
    case 11: {
      const uint8_t p[] = {1, 0, 1, 0, 0, 0, 0, 0};
      if (p[fx_hb % 8]) allOn();
      else allOff();
      fx_hb++;
      break;
    }

    // 12: Cylon Eye
    case 12:
      allOff();
      if (effectStep > 0) setRelay(effectStep - 1, true);
      setRelay(effectStep, true);
      if (effectStep < NUM_RELAYS - 1) setRelay(effectStep + 1, true);
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) effectDir = -1;
      if (effectStep <= 0) effectDir = 1;
      break;

    // 13: Alternating Odd / Even
    case 13:
      for (int i = 0; i < NUM_RELAYS; i++) {
        setRelay(i, (i % 2) == (effectStep % 2));
      }
      effectStep++;
      break;

    // 14: Cascade
    case 14:
      if (effectStep < NUM_RELAYS) setRelay(effectStep, true);
      else setRelay(effectStep - NUM_RELAYS, false);
      effectStep++;
      if (effectStep >= NUM_RELAYS * 2) effectStep = 0;
      break;

    // 15: Theater 3-Phase
    case 15:
      allOff();
      for (int i = effectStep % 3; i < NUM_RELAYS; i += 3) {
        setRelay(i, true);
      }
      effectStep++;
      break;

    // 16: Rain Drop
    case 16:
      for (int i = NUM_RELAYS - 1; i > 0; i--) {
        setRelay(i, relayState[i - 1]);
      }
      setRelay(0, random(3) == 0);
      break;

    // 17: Fireworks
    case 17:
      if (fx_firework_b == 0) {
        allOff();
        fx_firework_fc = random(2, NUM_RELAYS - 2);
        setRelay(fx_firework_fc, true);
        fx_firework_b = 1;
      } else if (fx_firework_b <= 6) {
        for (int i = 0; i < NUM_RELAYS; i++) {
          if (relayState[i]) {
            setRelay(i, false);
            if (i > 0) setRelay(i - 1, true);
            if (i < NUM_RELAYS - 1) setRelay(i + 1, true);
          }
        }
        fx_firework_b++;
      } else {
        allOff();
        fx_firework_b = 0;
      }
      break;

    // 18: Knight Rider (5-channel wide)
    case 18:
      allOff();
      for (int w = -2; w <= 2; w++) {
        int p = effectStep + w;
        if (p >= 0 && p < NUM_RELAYS) setRelay(p, true);
      }
      effectStep += effectDir;
      if (effectStep >= NUM_RELAYS - 1) effectDir = -1;
      if (effectStep <= 0) effectDir = 1;
      break;

    // 19: Meteor with Trail
    case 19:
      for (int i = 0; i < NUM_RELAYS; i++) {
        if (relayState[i] && random(3) == 0) setRelay(i, false);
      }
      {
        int p = effectStep % NUM_RELAYS;
        setRelay(p, true);
        if (p > 0) setRelay(p - 1, true);
      }
      effectStep++;
      break;

    // 20: Dual Bounce Crossing
    case 20:
      allOff();
      setRelay(fx_cross_pA, true);
      setRelay(fx_cross_pB, true);
      fx_cross_pA += fx_cross_dA;
      fx_cross_pB += fx_cross_dB;
      if (fx_cross_pA >= NUM_RELAYS - 1 || fx_cross_pA <= 0) fx_cross_dA = -fx_cross_dA;
      if (fx_cross_pB >= NUM_RELAYS - 1 || fx_cross_pB <= 0) fx_cross_dB = -fx_cross_dB;
      break;

    // 21: Cross Fade (Odd / All / Even)
    case 21:
      if (fx_phase == 0) {
        for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, i % 2 == 0);
      } else if (fx_phase == 1) {
        allOn();
      } else {
        for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, i % 2 == 1);
      }
      fx_phase = (fx_phase + 1) % 3;
      break;

    // 22: Strobe Flash
    case 22:
      if (effectStep % 2 == 0) allOn();
      else allOff();
      effectStep++;
      break;

    // 23: Wipe Fill & Clear
    case 23:
      if (effectStep < NUM_RELAYS) {
        setRelay(effectStep, true);
      } else {
        setRelay(NUM_RELAYS * 2 - 1 - effectStep, false);
      }
      effectStep++;
      if (effectStep >= NUM_RELAYS * 2) effectStep = 0;
      break;

    // 24: Stack Build
    case 24:
      allOff();
      for (int i = fx_stack_top; i < NUM_RELAYS; i++) {
        setRelay(i, true);
      }
      setRelay(fx_stack_ball, true);
      fx_stack_ball++;
      if (fx_stack_ball >= fx_stack_top) {
        fx_stack_top--;
        fx_stack_ball = 0;
        if (fx_stack_top < 0) {
          fx_stack_top = NUM_RELAYS - 1;
          allOff();
        }
      }
      break;
  }
}

// --- Auto Cycle Transitions (Non-blocking) ---
void handleAutoMode() {
  if (currentMode != MODE_AUTO) return;

  unsigned long now = millis();
  if (now - autoTimer >= (unsigned long)autoCycleTime) {
    autoTimer = now;
    nextEffectInCycle();
  }
}

// --- JSON State API & HTTP Helpers ---
String buildStateJson() {
  String modeStr = "auto";
  if (currentMode == MODE_MANUAL) modeStr = "manual";
  else if (currentMode == MODE_EFFECT) modeStr = "effect";
  else if (currentMode == MODE_OFF) modeStr = "off";

  String ipStr = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "0.0.0.0";
  String apStr = WiFi.softAPIP().toString();

  String j = "{";
  j += "\"e\":" + String(currentEffect) + ",";
  j += "\"s\":" + String(effectSpeed) + ",";
  j += "\"m\":\"" + modeStr + "\",";
  j += "\"c\":" + String(autoCycleTime / 1000) + ",";
  j += "\"p\":" + String(currentPresetIndex) + ",";
  j += "\"pn\":\"" + String(PRESETS[currentPresetIndex].name) + "\",";
  j += "\"ap\":\"" + apStr + "\",";
  j += "\"sta\":\"" + ipStr + "\",";
  j += "\"ch\":[";
  for (int i = 0; i < NUM_RELAYS; i++) {
    j += relayState[i] ? "1" : "0";
    if (i < NUM_RELAYS - 1) j += ",";
  }
  j += "]}";
  return j;
}

void addCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Connection", "close");
}

void sendState() {
  addCorsHeaders();
  server.send(200, "application/json", buildStateJson());
}

// --- HTTP Route Handlers ---
void handleRoot() {
  addCorsHeaders();
  String pg = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>ESP32 16-CH Relay</title>");
  pg += FPSTR(PCSS);
  pg += F("</head><body>");
  pg += FPSTR(PHTML);
  pg += FPSTR(PJS);
  pg += F("</body></html>");
  server.send(200, "text/html", pg);
}

void handleState() {
  sendState();
}

void handleEffect() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    selectEffect(id);
  }
  sendState();
}

void handlePreset() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    selectPreset(id);
  }
  sendState();
}

void handleRelay() {
  if (server.hasArg("ch") && server.hasArg("v")) {
    int ch = server.arg("ch").toInt();
    bool val = (server.arg("v").toInt() == 1);
    currentMode = MODE_MANUAL;
    setRelay(ch, val);
  }
  sendState();
}

void handleSpeed() {
  if (server.hasArg("v")) {
    effectSpeed = constrain(server.arg("v").toInt(), 20, 500);
  }
  sendState();
}

void handleCycle() {
  if (server.hasArg("v")) {
    autoCycleTime = constrain(server.arg("v").toInt(), 2000, 30000);
  }
  sendState();
}

void handleCmd() {
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
      autoTimer   = millis();
    } else {
      currentMode = MODE_OFF;
      allOff();
    }
  } else if (a == "toggle_auto") {
    if (currentMode == MODE_AUTO) {
      currentMode = MODE_EFFECT;
    } else {
      currentMode = MODE_AUTO;
      autoTimer   = millis();
    }
  } else if (a == "toggle_manual") {
    if (currentMode == MODE_MANUAL) {
      currentMode = MODE_AUTO;
      autoTimer   = millis();
    } else {
      currentMode = MODE_MANUAL;
    }
  } else if (a == "next") {
    nextEffectInCycle();
  } else if (a == "prev") {
    prevEffectInCycle();
  } else if (a == "odd") {
    currentMode = MODE_MANUAL;
    for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, i % 2 == 0);
  } else if (a == "even") {
    currentMode = MODE_MANUAL;
    for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, i % 2 == 1);
  } else if (a == "invert") {
    currentMode = MODE_MANUAL;
    for (int i = 0; i < NUM_RELAYS; i++) setRelay(i, !relayState[i]);
  }
  sendState();
}

void handleNotFound() {
  addCorsHeaders();
  server.send(404, "text/plain", "Not Found");
}

// --- Setup & Main Loop (Zero Blocking Delays) ---
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n==================================================");
  Serial.println("  ESP32 16-CH RELAY v8.5 PRO (UCTRONICS 12V)");
  Serial.println("==================================================");

  // 1. Safe GPIO init (Glitch-free, all relays OFF)
  initRelayPins();
  Serial.println("[1] GPIO Initialized: All 16 Relays firmly OFF.");

  // 2. Start Dual WiFi (AP + Station) without blocking
  setupWiFi();

  // 3. Web Server Route Registration
  server.on("/",        handleRoot);
  server.on("/state",   handleState);
  server.on("/effect",  handleEffect);
  server.on("/preset",  handlePreset);
  server.on("/relay",   handleRelay);
  server.on("/speed",   handleSpeed);
  server.on("/cycle",   handleCycle);
  server.on("/cmd",     handleCmd);
  server.onNotFound(    handleNotFound);
  server.begin();
  Serial.println("[2] Web Server active on port 80.");

  // 4. Initialize Hardware Watchdog (30 seconds)
  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = 30000,
      .idle_core_mask = 0,
      .trigger_panic = true
  };
  esp_task_wdt_init(&twdt_config);
  esp_task_wdt_add(NULL);
  Serial.println("[3] Hardware Watchdog configured (30s).");

  // 5. Initialize Timer Baselines
  effectTimer = millis();
  autoTimer   = millis();
  wifiTimer   = millis();
  wdTimer     = millis();

  // Load default preset 0
  selectPreset(0);

  Serial.println("\n=== SYSTEM READY ===");
  Serial.println("Access Web Panel at:");
  Serial.println("  -> SoftAP:  http://192.168.4.1 (Connect WiFi: ESP32_Relay / 12345678)");
  Serial.println("  -> Home LAN: http://<local-ip> (When connected to loki)");
  Serial.println("==================================================\n");
}

void loop() {
  // Feed watchdog
  esp_task_wdt_reset();

  // Handle incoming HTTP client requests
  server.handleClient();

  // Advance automatic preset cycling (non-blocking)
  handleAutoMode();

  // Run single-step pattern generation (non-blocking)
  runEffectStep();

  // Background WiFi station keepalive (non-blocking)
  checkWiFiNonBlocking();

  yield();
}
