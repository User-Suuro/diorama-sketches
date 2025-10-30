#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ATUtils.h"
#include "ATWifi.h"
#include "ATGet.h"
#include "ATPost.h"
#include <avr/wdt.h>

// == DIGITAL PINS == //

#define SOUND_PIN A0
#define LIGHT_PIN A1

#define RELAY_PIN_01 4
#define RELAY_PIN_02 5
#define RELAY_PIN_03 6
#define RELAY_PIN_04 7

#define TOUCH_PIN_01 8
#define TOUCH_PIN_02 9
#define TOUCH_PIN_03 10
#define TOUCH_PIN_04 11
#define DF_RX 12
#define DF_TX 13

#define TRIG_PIN 30
#define ECHO_PIN 31

// == SERIAL == //

#define ESP_SERIAL Serial3

// == CONSTANTS == //

const char* ssid  = "X8b";
const char* password = "123456789";
const char* host = "diorama-endpoint.vercel.app";
const int port = 443;
const char* device_status = "/api/arduino/device-status";

// == INIT IMPORTED UTILITIES == //

SoftwareSerial mp3Serial(DF_RX, DF_TX);
DFRobotDFPlayerMini mp3;
LiquidCrystal_I2C lcd(0x27, 16, 2);

ATUtils at(ESP_SERIAL);
ATWifi wifi(at);
ATGet httpGET(at);
ATPost httpPOST(at);

// == STATES == //

bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

bool fetchInProgress = false;
bool postInProgress = false;
String response;
bool readyToPost = false;
unsigned long lastNetworkAction = 0;
unsigned long lastLocalChange = 0;
bool hasPendingLocalChange = false;

const unsigned long NETWORK_COOLDOWN = 5000;
const unsigned long LOCAL_CHANGE_COOLDOWN = 5000;

// == SOUND SENSOR VARIABLES == //

const float SMOOTHING_ALPHA = 0.3;
float baseline = 0.0;
float smoothed = 0.0;

// == TASK MANAGEMENT == //

struct Task {
  unsigned long interval;
  unsigned long lastRun;
  void (*callback)();
};

// ======== RELAY CONTROL MANAGER ======== //

// ======== RELAY CONTROL MANAGER ======== //
void setAllRelays(bool on, const char* source) {
  relayState1 = relayState2 = relayState3 = relayState4 = on;

  digitalWrite(RELAY_PIN_01, on ? LOW : HIGH);
  digitalWrite(RELAY_PIN_02, on ? LOW : HIGH);
  digitalWrite(RELAY_PIN_03, on ? LOW : HIGH);
  digitalWrite(RELAY_PIN_04, on ? LOW : HIGH);

  Serial.print("⚙️ Relays set by ");
  Serial.print(source);
  Serial.print(" → State: ");
  Serial.println(on ? "ON" : "OFF");

  // ===== LCD FEEDBACK =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Source: ");
  lcd.print(source);
  lcd.setCursor(0, 1);
  lcd.print("Lights: ");
  lcd.print(on ? "ON " : "OFF");

  lastLocalChange = millis();
  hasPendingLocalChange = true;
}


// ======== LIGHT SENSOR ======== //

void checkLightSensor() { 
  static unsigned long lastCheck = 0;
  const unsigned long interval = 500;  // check every 0.5 seconds

  if (millis() - lastCheck < interval) return;
  lastCheck = millis();

  // 🧩 Skip if a local change (clap or touch) happened recently
  if (hasPendingLocalChange && millis() - lastLocalChange < LOCAL_CHANGE_COOLDOWN) {
    return;
  }

  int lightValue = analogRead(LIGHT_PIN);
  Serial.print("💡 Light sensor value: ");
  Serial.println(lightValue);

  // --- Your desired logic ---
  if (lightValue < 100 && !relayState1) {
    Serial.println("🌑 Dark detected — turning ON all relays.");
    setAllRelays(true, "Light Sensor");
  }
  else if (lightValue > 100 && relayState1) {
    Serial.println("☀️ Bright environment — turning OFF all relays.");
    setAllRelays(false, "Light Sensor");
  }
}


// ======== SOUND SENSOR ======== //
// ======== SOUND SENSOR (Clap with Daytime Override) ======== //
void checkSoundSensor() {
  static unsigned long lastClapMs = 0;
  static unsigned long temporaryOnUntil = 0; // For daytime override timer

  const unsigned long CLAP_WINDOW_US   = 40000;  // 40ms sample window
  const int           CLAP_THRESHOLD   = 35;     // adjust sensitivity (25–50)
  const unsigned long CLAP_COOLDOWN_MS = 800;    // prevent double triggers
  const unsigned long DAYTIME_ON_DURATION = 5000; // 5 seconds ON duration during bright light

  unsigned long now = millis();
  if (now - lastClapMs < CLAP_COOLDOWN_MS) return;

  // --- Capture min/max in short sampling window ---
  unsigned long start = micros();
  int minVal = 1023;
  int maxVal = 0;
  int raw = 0;

  while ((micros() - start) < CLAP_WINDOW_US) {
    raw = analogRead(SOUND_PIN);
    if (raw < minVal) minVal = raw;
    if (raw > maxVal) maxVal = raw;
  }

  int p2p = maxVal - minVal;  // peak-to-peak difference
  int lightValue = analogRead(LIGHT_PIN);  // current light level

  // --- Detect clap ---
  if (p2p >= CLAP_THRESHOLD) {
    bool isBright = lightValue > 100;
    bool newState = !relayState1;

    if (isBright && !relayState1) {
      // 🌞 Daytime: allow lights ON for a short period
      Serial.println("☀️ Bright environment — lights ON temporarily (5s).");
      setAllRelays(true, "Sound Sensor (Daytime Override)");
      temporaryOnUntil = now + DAYTIME_ON_DURATION;
      hasPendingLocalChange = true;
      lastLocalChange = millis();
      lastClapMs = now;
      return;
    }

    // 🌑 Normal behavior (dark or turning off)
    Serial.print("👏 Clap detected! Lights are now ");
    Serial.println(newState ? "ON" : "OFF");
    setAllRelays(newState, "Sound Sensor (Clap)");

    hasPendingLocalChange = true;
    lastLocalChange = millis();
    lastClapMs = now;
  }

  // --- Handle daytime auto-off after override --- 
  if (temporaryOnUntil > 0 && now >= temporaryOnUntil && relayState1) {
    Serial.println("⏰ Daytime auto-off (override ended).");
    setAllRelays(false, "Auto-Off Timer");
    temporaryOnUntil = 0;
  }
}


// ======== PROXIMITY SENSOR ======== //

float getDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  noInterrupts();
  long duration = pulseIn(ECHO_PIN, HIGH, 60000);
  interrupts();

  return duration * 0.0343 / 2.0;
}

// ======== PROXIMITY SENSOR ======== //

void checkProximitySensor() {
  static bool objectDetected = false;
  static unsigned long lastTrigger = 0;
  const unsigned long triggerCooldown = 3000;
  const float detectionThreshold = 20.0;

  float distance = getDistanceCm();

  if (distance > 0 && distance <= detectionThreshold) {
    unsigned long now = millis();

    if (!objectDetected && (now - lastTrigger) > triggerCooldown) {
      Serial.print("📏 Object detected at ");
      Serial.print(distance);
      Serial.println(" cm — playing 002.mp3!");

      // Play sound
      mp3.play(2);

      // === LCD MESSAGE ===
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome, Bitch!");
      lcd.setCursor(0, 1);

      // Return LCD to status view
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Source: Ultrasonic");
      lcd.setCursor(0, 1);
      lcd.print("Sound: ON");


      objectDetected = true;
      lastTrigger = now;
    }
  } else {
    objectDetected = false;
  }
}


// ======== TOUCH RELAY CONTROL ======== //

void controlRelays() {
  const uint8_t touchPins[4] = {TOUCH_PIN_01, TOUCH_PIN_02, TOUCH_PIN_03, TOUCH_PIN_04};
  bool *relayStates[4] = {&relayState1, &relayState2, &relayState3, &relayState4};
  const uint8_t relayPins[4] = {RELAY_PIN_01, RELAY_PIN_02, RELAY_PIN_03, RELAY_PIN_04};

  static bool lastTouchState[4] = {false, false, false, false};

  for (int i = 0; i < 4; i++) {
    bool current = digitalRead(touchPins[i]);
    if (current && !lastTouchState[i]) {
      *relayStates[i] = !(*relayStates[i]);
      digitalWrite(relayPins[i], *relayStates[i] ? LOW : HIGH);
      Serial.print(F("🖐️ Local relay toggle detected (R"));
      Serial.print(i + 1);
      Serial.println(F(")."));
      lastLocalChange = millis();
      hasPendingLocalChange = true;
    }
    lastTouchState[i] = current;
  }
}

// ======== NETWORK ======== //

void fetchUpdateToServer(const char *endpoint) {
  if (fetchInProgress || postInProgress) return;
  Serial.println("🌐 Starting async fetch...");
  fetchInProgress = true;
  httpGET.beginAsync(host, port, endpoint, 15000);
}

void postDeviceStatus() {
  static bool lastRelayState[4] = {false, false, false, false};
  if (postInProgress || fetchInProgress) return;

  bool stateChanged =
    (relayState1 != lastRelayState[0]) ||
    (relayState2 != lastRelayState[1]) ||
    (relayState3 != lastRelayState[2]) ||
    (relayState4 != lastRelayState[3]);

  if (!stateChanged) return;

  postInProgress = true;
  char payload[160];
  snprintf(payload, sizeof(payload),
    "{\"switch_01\":%s,\"switch_02\":%s,\"switch_03\":%s,\"switch_04\":%s}",
    relayState1 ? "true" : "false",
    relayState2 ? "true" : "false",
    relayState3 ? "true" : "false",
    relayState4 ? "true" : "false"
  );

  Serial.println(F("🌐 Starting async POST (state changed)..."));
  httpPOST.beginAsync(host, port, device_status, payload, 8000);

  lastRelayState[0] = relayState1;
  lastRelayState[1] = relayState2;
  lastRelayState[2] = relayState3;
  lastRelayState[3] = relayState4;
}

// ======== TASKS ======== //

Task tasks[] = {
  {2000, 0, checkProximitySensor},
  {20000, 0, []() { fetchUpdateToServer(device_status); }},
};

const int numTasks = sizeof(tasks) / sizeof(Task);

// ======== INITIALIZATION ======== //

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

  pinMode(LIGHT_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);

  digitalWrite(RELAY_PIN_01, HIGH);
  digitalWrite(RELAY_PIN_02, HIGH);
  digitalWrite(RELAY_PIN_03, HIGH);
  digitalWrite(RELAY_PIN_04, HIGH);
}

void initSound() {
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(SOUND_PIN);
    delay(10);
  }
  baseline = sum / 100.0;
  smoothed = 0;
}

void initMP3Player() {
  if (!mp3.begin(mp3Serial)) {
    Serial.println("❌ DFPlayer not responding!");
    return;
  }
  Serial.println("✅ DFPlayer Mini initialized.");
  mp3.volume(30);
  delay(200);
  mp3.play(1);
}

void initESP() {
  if (at.testAT()) Serial.println("✅ ESP ready.");
  else Serial.println("❌ No AT response.");
  at.setEcho(false);
  at.setMode(1);
  if (wifi.connect(ssid, password)) {
    Serial.println("✅ Wi-Fi connected!");
    wifi.printStatus();
  }
}

void initLCD() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Diorama System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
}

// ======== MAIN ======== //

void setup() {
  Serial.begin(9600);
  ESP_SERIAL.begin(9600);
  mp3Serial.begin(9600);

  delay(500);
  initPins();
  initESP();
  initMP3Player();
  initSound();
  initLCD();

  Serial.println("✅ System ready.");
}

void loop() {
  unsigned long now = millis();

  // Keep sensors highly responsive
  checkSoundSensor();
  checkLightSensor();
  controlRelays();

  // Update network tasks
  if (fetchInProgress) httpGET.update();
  if (postInProgress) httpPOST.update();

  // Scheduled low-priority tasks
  for (int i = 0; i < numTasks; i++) {
    if (now - tasks[i].lastRun >= tasks[i].interval) {
      tasks[i].lastRun = now;
      tasks[i].callback();
    }
  }

  // Automatic posting after cooldown
  if (!postInProgress && millis() - lastNetworkAction > NETWORK_COOLDOWN) {
    postDeviceStatus();
    lastNetworkAction = millis();
  }

  delay(1);
}
