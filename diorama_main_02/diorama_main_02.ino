#include "ATSend.h"
#include "ATGet.h"
#include "ATPost.h"
#include "JsonBuilder.h"
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define SOUND_PIN 3

// ==== Relay & Touch Pin Definitions ====
#define RELAY_PIN_01 4
#define RELAY_PIN_02 5
#define RELAY_PIN_03 6
#define RELAY_PIN_04 7

#define TOUCH_PIN_01 8
#define TOUCH_PIN_02 9
#define TOUCH_PIN_03 10
#define TOUCH_PIN_04 11

// ==== DFPlayer ====
#define DF_RX 12
#define DF_TX 13
SoftwareSerial mp3Serial(DF_RX, DF_TX);
DFRobotDFPlayerMini mp3;

// ==== Ultrasonic & Light Sensor ====
#define TRIG_PIN 22
#define ECHO_PIN 23
#define LDR_PIN A1

// ==== WiFi and Server ====
const char ssid[] PROGMEM = "X8b";
const char password[] PROGMEM = "123456789";
const char host[] PROGMEM = "diorama-endpoint.vercel.app";
const char device_status[] PROGMEM = "/api/arduino/device-status";

// ==== ESP Serial ====
#define ESP_SERIAL Serial3
ATSend atSend(ESP_SERIAL, 10000);
ATGet atGet(atSend, ESP_SERIAL, host, port);
ATPost atPost(atSend, ESP_SERIAL, host, port);
JsonBuilder jsonBuilder;

// ==== Timers ====
const unsigned long RELAY_INTERVAL = 10;
const unsigned long FETCH_INTERVAL = 12000;
const unsigned long LDR_INTERVAL = 1000;
const unsigned long ULTRASONIC_INTERVAL = 500;

unsigned long lastRelayTime = 0;
unsigned long lastFetchTime = 4000;
unsigned long lastPostTime = 0;
unsigned long lastUltrasonicTime = 0;
unsigned long lastLDRTime = 0;

// ==== States ====
bool relayState1 = false, relayState2 = false, relayState3 = false, relayState4 = false;
bool ultrasonicTriggered = false;
bool hasLocalChange = false;

int LDRValue = 0;
char lastServerTimestamp[32] = {0};  // 🧩 Use char buffer instead of String

// === SETUP ===
void setup() {
  Serial.begin(9600);
  ESP_SERIAL.begin(9600);
  mp3Serial.begin(9600);
  delay(1000);

  initPins();
  initUltrasonic();

  Serial.println(F("🔌 Initializing..."));
  atSend.sendCommand("AT+RST", true);
  delay(2000);
  atSend.sendCommand("AT", true);

  connectWiFi();
  playWelcomeSound();

  Serial.println(F("✅ Ready — async fetch started"));
  atGet.beginAsync(device_status, true);
}

// === MAIN LOOP ===
void loop() {
  unsigned long now = millis();

  // --- Always update async operations first ---
  atGet.updateAsync();
  atPost.updateAsync();


  // === 4. Handle GET / POST communications ===

  // 🧩 Avoid both running at the same time
  bool commsBusy = atGet.isBusy() || atPost.isBusy();

  // --- a. Handle GET response if ready ---
  if (atGet.hasResponse()) {
    Serial.println(F("🌐 Got response (truncated preview):"));
    const char* resp = atGet.getResponse().c_str();
    for (int i = 0; i < 200 && resp[i]; i++) Serial.write(resp[i]);
    Serial.println();
    parseAndUpdateRelays(resp);
    atGet.clearResponse();
  }

  // --- b. Handle POST response if ready ---
  if (atPost.hasResponse()) {
    Serial.println(F("✅ POST Done"));
    atPost.clearResponse();
  }

  // --- c. Schedule next GET only if idle ---
  if (!commsBusy && (now - lastFetchTime >= FETCH_INTERVAL)) {
    lastFetchTime = now;
    Serial.println(F("🌐 Async GET start..."));
    atGet.beginAsync(device_status, true);
  }

  // --- d. Handle local relay change and queue POST ---
  static bool postQueued = false;

  // If a change occurred and communications are idle
  if (hasLocalChange && !commsBusy && !postQueued) {
    hasLocalChange = false;
    postQueued = true; // prevent overlapping post requests

    jsonBuilder.begin();
    jsonBuilder.add("switch_01", relayState1);
    jsonBuilder.add("switch_02", relayState2);
    jsonBuilder.add("switch_03", relayState3);
    jsonBuilder.add("switch_04", relayState4);
    jsonBuilder.add("lums_val", LDRValue);
    jsonBuilder.add("is_arduino", true);
    jsonBuilder.end();

    const String jsonPayload = jsonBuilder.str();
    Serial.println(F("📤 Queued POST (short JSON):"));
    Serial.println(jsonPayload);

    atPost.beginAsync(device_status, jsonPayload, true);
    lastPostTime = now;
  }

  // Reset flag after POST completes
  if (postQueued && !atPost.isBusy()) {
    postQueued = false;
  }

  // === 5. Small cooperative delay to yield CPU ===
  delay(5);
}


// === SUPPORT FUNCTIONS ===

void initPins() {
  pinMode(RELAY_PIN_01, OUTPUT);
  pinMode(RELAY_PIN_02, OUTPUT);
  pinMode(RELAY_PIN_03, OUTPUT);
  pinMode(RELAY_PIN_04, OUTPUT);

  pinMode(TOUCH_PIN_01, INPUT);
  pinMode(TOUCH_PIN_02, INPUT);
  pinMode(TOUCH_PIN_03, INPUT);
  pinMode(TOUCH_PIN_04, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // Default: relays off (HIGH)
  digitalWrite(RELAY_PIN_01, HIGH);
  digitalWrite(RELAY_PIN_02, HIGH);
  digitalWrite(RELAY_PIN_03, HIGH);
  digitalWrite(RELAY_PIN_04, HIGH);
}

void initUltrasonic() {
  Serial.println(F("📏 Ultrasonic ready"));
}

void controlRelays() {
  if (digitalRead(TOUCH_PIN_01) == HIGH) {
    relayState1 = !relayState1;
    digitalWrite(RELAY_PIN_01, relayState1 ? LOW : HIGH);
    hasLocalChange = true;
    while (digitalRead(TOUCH_PIN_01) == HIGH) delay(10);
  }

  if (digitalRead(TOUCH_PIN_02) == HIGH) {
    relayState2 = !relayState2;
    digitalWrite(RELAY_PIN_02, relayState2 ? LOW : HIGH);
    hasLocalChange = true;
    while (digitalRead(TOUCH_PIN_02) == HIGH) delay(10);
  }

  if (digitalRead(TOUCH_PIN_03) == HIGH) {
    relayState3 = !relayState3;
    digitalWrite(RELAY_PIN_03, relayState3 ? LOW : HIGH);
    hasLocalChange = true;
    while (digitalRead(TOUCH_PIN_03) == HIGH) delay(10);
  }

  if (digitalRead(TOUCH_PIN_04) == HIGH) {
    bool allOn = relayState1 && relayState2 && relayState3 && relayState4;
    relayState1 = relayState2 = relayState3 = relayState4 = !allOn;
    digitalWrite(RELAY_PIN_01, relayState1 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_02, relayState2 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_03, relayState3 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_04, relayState4 ? LOW : HIGH);
    hasLocalChange = true;
    while (digitalRead(TOUCH_PIN_04) == HIGH) delay(10);
  }
}

void connectWiFi() {
  Serial.println(F("📶 Connecting WiFi..."));

  char ssidBuf[64];
  char passBuf[64];
  strcpy_P(ssidBuf, ssid);
  strcpy_P(passBuf, password);

  char cmd[160];
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssidBuf, passBuf);

  Serial.print(F("→ ")); 
  Serial.println(cmd);

  atSend.sendCommand(cmd, true);
  delay(1500);
}

void parseAndUpdateRelays(const char* json) {
  const char* tsPtr = strstr(json, "\"createdAt\":\"");
  if (!tsPtr) return;
  tsPtr += 13;

  char ts[32] = {0};
  strncpy(ts, tsPtr, 31);
  char* quote = strchr(ts, '"');
  if (quote) *quote = 0;

  if (strcmp(ts, lastServerTimestamp) > 0) {
    strcpy(lastServerTimestamp, ts);
    bool s1 = strstr(json, "\"switch_01\":true");
    bool s2 = strstr(json, "\"switch_02\":true");
    bool s3 = strstr(json, "\"switch_03\":true");
    bool s4 = strstr(json, "\"switch_04\":true");

    digitalWrite(RELAY_PIN_01, s1 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_02, s2 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_03, s3 ? LOW : HIGH);
    digitalWrite(RELAY_PIN_04, s4 ? LOW : HIGH);

    relayState1 = s1; relayState2 = s2; relayState3 = s3; relayState4 = s4;
    Serial.println(F("✅ Updated from server"));
  }
}

void playWelcomeSound() {
  if (!mp3.begin(mp3Serial)) {
    Serial.println(F("❌ DFPlayer not found!"));
    return;
  }
  mp3.volume(25);
  mp3.play(1);
}

void checkUltrasonic() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  if (distance > 0 && distance <= 5) {
    if (!ultrasonicTriggered) {
      ultrasonicTriggered = true;
      mp3.play(2);
    }
  } else ultrasonicTriggered = false;
}

void checkLightSensor() {
  LDRValue = analogRead(LDR_PIN);
  if (LDRValue <= 80) {
    relayState1 = relayState2 = relayState3 = relayState4 = true;
    digitalWrite(RELAY_PIN_01, LOW);
    digitalWrite(RELAY_PIN_02, LOW);
    digitalWrite(RELAY_PIN_03, LOW);
    digitalWrite(RELAY_PIN_04, LOW);
    hasLocalChange = true;
  }
}
