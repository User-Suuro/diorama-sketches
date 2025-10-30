#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "ATUtils.h"
#include "ATWifi.h"
#include "ATGet.h"

// == DIGITAL PINS == //

#define SOUND_PIN 3

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

#define TRIG_PIN 22
#define ECHO_PIN 23

// == ANALOGS PINS == //

#define LIGHT_PIN A1

// == SERIAL == //

#define ESP_SERIAL Serial3

// == CONSTANTS == //

const char* ssid  = "X8b";
const char* password = "12345678";
const char* host = "diorama-endpoint.vercel.app";
const int port = 443;

const char* device_status = "/api/arduino/device-status";

// == INIT IMPORTED UTILITIES == //

SoftwareSerial mp3Serial(DF_RX, DF_TX);
DFRobotDFPlayerMini mp3;
ATUtils util(ESP_SERIAL, 8000); // 8-second timeout
ATWifi wifi(util, ssid, password);
ATGet httpGET(util, host, port);

// == STATES == // 

bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;
String response; 

// == TASK MANAGEMENT == //

struct Task {
  unsigned long interval;
  unsigned long lastRun;
  void (*callback)();
};

void controlRelays() {
  const uint8_t touchPins[4] = {TOUCH_PIN_01, TOUCH_PIN_02, TOUCH_PIN_03, TOUCH_PIN_04};
  const uint8_t relayPins[4] = {RELAY_PIN_01, RELAY_PIN_02, RELAY_PIN_03, RELAY_PIN_04};
  bool *relayStates[4] = {&relayState1, &relayState2, &relayState3, &relayState4};

  static bool lastTouchState[4] = {false, false, false, false};

  for (int i = 0; i < 4; i++) {
    bool current = digitalRead(touchPins[i]);

    // Toggle only when the touch goes from LOW → HIGH
    if (current && !lastTouchState[i]) {
      *relayStates[i] = !(*relayStates[i]);
      digitalWrite(relayPins[i], *relayStates[i] ? LOW : HIGH);
    }

    lastTouchState[i] = current; // update for next loop
  }
}


void fetchUpdateToServer(const char *endpoint) {
  static bool started = false;  // persistent flag between calls

  // Continue processing async GET if one is running
  httpGET.process();

  // Start new request only if none is active
  if (!started && httpGET.isFinished()) {
    started = httpGET.beginAsync(endpoint);
    if (started) Serial.println("🌐 [HTTP] Async GET started...");
  }

  // Once finished, handle the result
  if (started && httpGET.isFinished()) {
    Serial.println("✅ HTTP Done:");
    Serial.println(httpGET.getResponse());
    started = false;  // reset for next cycle
  }
}


Task tasks[] = {
  {100, 0, controlRelays},
  {10000, 0, []() { fetchUpdateToServer(device_status); }} 
};


const int numTasks = sizeof(tasks) / sizeof(Task);

// == MAIN == //

void setup() {

  Serial.begin(9600);
  delay(500);

  ESP_SERIAL.begin(9600);
  delay(1000);

  initPins();

  if (wifi.isConnected()) {
    delay(1000);
    wifi.disconnect(true);
  } 

  delay(1000);

  while (!wifi.connect(true));
  
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < numTasks; i++) {
    if (now - tasks[i].lastRun >= tasks[i].interval) {
      tasks[i].lastRun = now;
      tasks[i].callback();
    }
  }
}


// == UTILITIES == //

void initPins() {

  // relays
  pinMode(RELAY_PIN_01, OUTPUT);
  pinMode(RELAY_PIN_02, OUTPUT);
  pinMode(RELAY_PIN_03, OUTPUT);
  pinMode(RELAY_PIN_04, OUTPUT);

  // touch
  pinMode(TOUCH_PIN_01, INPUT);
  pinMode(TOUCH_PIN_02, INPUT);
  pinMode(TOUCH_PIN_03, INPUT);
  pinMode(TOUCH_PIN_04, INPUT);

  // proximity
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // light
  pinMode(LIGHT_PIN, INPUT);

  // initiliaze values
  digitalWrite(RELAY_PIN_01, HIGH);
  digitalWrite(RELAY_PIN_02, HIGH);
  digitalWrite(RELAY_PIN_03, HIGH);
  digitalWrite(RELAY_PIN_04, HIGH);
}

void postUpdateToServer() {

}



