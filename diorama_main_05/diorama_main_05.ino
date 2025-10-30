#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "JsonBuilder.h"

#define LIGHT_PIN A1

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

#define TRIG_PIN 50
#define ECHO_PIN 51

// == SERIAL == //
#define ESP_SERIAL Serial3
SoftwareSerial MP3_SERIAL(DF_RX, DF_TX);

// == CONSTANTS == //
const char* ssid  = "X8b";
const char* password = "123456789";
const char* host = "diorama-endpoint.vercel.app";
const int port = 443;

const char* endp_controllers = "/api/arduino/controllers";
const char* endp_sensors = "/api/arduino/sensors";

// == MODULES == //
LiquidCrystal_I2C lcd(0x27, 16, 2);
DFRobotDFPlayerMini mp3;

// == STATES == //
bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

int lumsVal = 0;
int clapsVal = 0;
int visitorsVal = 0;

bool hasLocalChange = false;
bool doFetchNext = true;
bool espBusy = false;

bool initSensorVal = false;
bool initSwitchesVal = false;

String espBuffer;
String cmdType;

unsigned long espStart = 0;
const unsigned long espTimeout = 16000;

// == ESP INITIALIZATION FUNCTIONS == //

String sendCMD(const String& cmd, unsigned long timeout = 5000) {
  while (ESP_SERIAL.available()) ESP_SERIAL.read();
  espBuffer = "";
  ESP_SERIAL.println(cmd);

  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (ESP_SERIAL.available()) {
      char c = ESP_SERIAL.read();
      espBuffer += c;
    }
    if (espBuffer.indexOf("OK") != -1 || 
        espBuffer.indexOf("ERROR") != -1 || 
        espBuffer.indexOf("FAIL") != -1) break;
    delay(5);
  }
  espBuffer.trim();
  return espBuffer;
}

void waitForESP(const String& cmd, const String& expected, unsigned long retryDelay = 500, unsigned long maxWait = 10000) {
  unsigned long startTime = millis();
  while (millis() - startTime < maxWait) {
    String response = sendCMD(cmd);
    if (response.indexOf(expected) != -1 || response.indexOf("OK") != -1) return;
    delay(retryDelay);
  }
  Serial.println(F("⚠️ waitForESP: Timeout waiting for expected response"));
}

void connectToWifi() {
  String wifi_cmd = String("AT+CWJAP=\"") + ssid + "\",\"" + password + "\"";
  waitForESP(wifi_cmd, "WIFI CONNECTED", 1000, 15000);
}

void connectToHost() {
  String ssl_cmd = String("AT+CIPSTART=\"SSL\",\"") + host + String("\",") + String(port);
  waitForESP(ssl_cmd, "CONNECT", 500, 10000);
}

void initESP() {
  Serial.println(F("Initializing ESP..."));
  waitForESP("AT+RST", "ready");
  waitForESP("AT", "OK");
 
  renderToLCD("ESP OK");

  renderToLCD("CONNECTING TO   WIFI");
  waitForESP("AT+CWMODE=1", "OK");
  connectToWifi();
  renderToLCD("WIFI OK");
  
  renderToLCD("CONNECTING TO   HOST");
  connectToHost();
  renderToLCD("HOST OK");

  Serial.println(F("✅ ESP Ready"));
  renderToLCD("ESP READY");
}

// == ESP LOOP FUNCTIONS == // 

void startESPCommand(const String& command) {
  if (espBusy) return;
  espBuffer = "";
  espBusy = true;
  espStart = millis();

  Serial.print(F("Sending to ESP: "));
  Serial.println(command);
  ESP_SERIAL.print(command);
}

void handleHTTPResponse() {
  while (ESP_SERIAL.available()) {
    char c = ESP_SERIAL.read();
    espBuffer += c;
  }

  if (espBusy && millis() - espStart > espTimeout) {
    Serial.println(F("ESP Timeout"));
    espBusy = false;
    espBuffer = "";
    return;
  }

  // handle post response

  if (cmdType == "localChange" || cmdType == "postSensors") { 

    // do nothing, just read if it return something

    if (espBuffer.indexOf("{") != -1) {
      espBusy = false;
      doFetchNext = true;

      if (cmdType == "localChange") {
        hasLocalChange = false;
      }
    }

    return;
  }

  // handle get response

  if (espBuffer.indexOf("}") != -1) {
    parseResponse();
  }
}


void parseResponse() {
  Serial.println(F("🔍 Parsing ESP response..."));

  int jsonStart = espBuffer.indexOf('{');
  int jsonEnd = espBuffer.lastIndexOf('}');

  String json = "";

  if (jsonStart != -1 && jsonEnd != -1) {
    json = espBuffer.substring(jsonStart, jsonEnd + 1);
  }

  Serial.println(json);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, json);

  if(cmdType == "fetchSwitch") {
    Serial.println("Processing change in switch!");

    if (!error) {
      bool front = doc["front_switch"];
      bool back = doc["back_switch"];
      bool left = doc["left_switch"];
      bool inside = doc["inside_switch"];
      bool is_arduino = doc["is_arduino"];

      if (!is_arduino || !initSwitchesVal) {
        relayState1 = back;
        relayState2 = left;
        relayState3 = front;
        relayState4 = inside;
        toggleRelays();

        if (!initSwitchesVal) initSwitchesVal = true;

      } else {
        
        // to make sure that the current state matches in db
        if (relayState1 != back || relayState2 != left || relayState3 != front ||relayState4 != inside) {
          hasLocalChange = true;
        }

      }

      espBuffer = "";
      espBusy = false; 
      
    } else {
      Serial.println("Failed to parse JSON");
    }
  } else if (cmdType == "initSensorVal") {
    
    if (!error) {
      visitorsVal = doc["visitors_val"];
      clapsVal = doc["claps_val"];
    };
  }
}

// == NETWORK TASK == //

void alternateNetworkEvent() {

  if (espBusy) return;

  if (!initSensorVal) {
    cmdType = "initSensorVal";
    initSensorVal = true;
    getData(endp_sensors);
  }

  else if (hasLocalChange) {
    cmdType = "localChange";
    Serial.println(F("Posting local switch change..."));
    JsonBuilder jb;
  
    jb.add("back_switch", relayState1);
    jb.add("left_switch", relayState2);
    jb.add("front_switch", relayState3);
    jb.add("inside_switch", relayState4);
    jb.add("is_arduino", true);

    String payload = jb.build();
    postData(endp_controllers, payload);
  } 

  else if (doFetchNext) {
    cmdType = "fetchSwitch";
    doFetchNext = false;  
    Serial.println(F("📤Fetching Switch..."));
    getData(endp_controllers);
  } 

  else {
    cmdType = "postSensors";
    doFetchNext = true;
    Serial.println(F("📤 Posting Sensors..."));

    JsonBuilder jb;
    jb.add("lums_val", lumsVal);
    jb.add("visitors_val", visitorsVal);
    jb.add("claps_val", clapsVal);
    String payload = jb.build();
    postData(endp_sensors, payload);
  } 

}

void postData(String endpoint, String payload) {
  String request = "POST " + String(endpoint) + " HTTP/1.1\r\n";
  request += "Host: " + String(host) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(payload.length()) + "\r\n";
  request += "Connection: keep-alive\r\n\r\n";
  request += payload;

  ESP_SERIAL.println("AT+CIPSEND=" + String(request.length()));
  delay(10);

  startESPCommand(request);
}

void getData(String endpoint) {
  String request = "GET " + String(endpoint) + " HTTP/1.1\r\n";
  request += "Host: " + String(host) + "\r\n";
  request += "Connection: keep-alive\r\n\r\n";

  ESP_SERIAL.println("AT+CIPSEND=" + String(request.length()));
  delay(10);

  startESPCommand(request);
}

// == SENSORS & RELAYS == //

void controlRelays() {
  const uint8_t touchPins[4] = {TOUCH_PIN_01, TOUCH_PIN_02, TOUCH_PIN_03, TOUCH_PIN_04};
  bool* relayStates[4] = {&relayState1, &relayState2, &relayState3, &relayState4};
  const uint8_t relayPins[4] = {RELAY_PIN_01, RELAY_PIN_02, RELAY_PIN_03, RELAY_PIN_04};
  static bool lastTouchState[4] = {false, false, false, false};

  for (int i = 0; i < 4; i++) {
    bool current = digitalRead(touchPins[i]);
    if (current && !lastTouchState[i]) {
      *relayStates[i] = !(*relayStates[i]);
      digitalWrite(relayPins[i], *relayStates[i] ? LOW : HIGH);
      hasLocalChange = true;
    }
    lastTouchState[i] = current;
  }
}

void toggleRelays() {
  digitalWrite(RELAY_PIN_01, relayState1 ? LOW : HIGH);
  digitalWrite(RELAY_PIN_02, relayState2 ? LOW : HIGH);
  digitalWrite(RELAY_PIN_03, relayState3 ? LOW : HIGH);
  digitalWrite(RELAY_PIN_04, relayState4 ? LOW : HIGH);
}

void readSensorEvent() {
  readLightSensor();
  readSoundSensor();
}

void readLightSensor() {
  lumsVal = analogRead(LIGHT_PIN);
  // Serial.println(lumsVal);
}

void readProximity() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 60000UL);
  float distance = duration * 0.0343f / 2.0f;

  // Serial.println(distance);
}

void readSoundSensor() {
  int soundState = digitalRead(SOUND_PIN); 

  if (soundState == HIGH) {
    relayState1 = true;
    relayState2 = true;
    relayState3 = true;
    relayState4 = true;

    toggleRelays();
    hasLocalChange = true;
    clapsVal++;
  } 
}


// == INITIALIZATION == //

void initPins() {
  uint8_t relayPins[4] = {RELAY_PIN_01, RELAY_PIN_02, RELAY_PIN_03, RELAY_PIN_04};
  uint8_t touchPins[4] = {TOUCH_PIN_01, TOUCH_PIN_02, TOUCH_PIN_03, TOUCH_PIN_04};

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    pinMode(touchPins[i], INPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  pinMode(SOUND_PIN, INPUT); 

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
}

void initMP3Player() {
  if (!mp3.begin(MP3_SERIAL)) {
    Serial.println(F("DFPlayer Not Responding"));
    return;
  }
  Serial.println(F("DFPlayer Initialized."));
  mp3.volume(50);
  delay(50);
  playMP3(1);
}

// 1 = welcome message
// 2 = ?

void playMP3(int trackNumber) {
  mp3.play(trackNumber);
}

void initLCD() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  delay(100);
}

void renderToLCD(const String &text) {
  lcd.clear(); 
  lcd.setCursor(0, 0);
  for (int i = 0; i < text.length() && i < 32; i++) {
    if (i == 16) lcd.setCursor(0, 1);
    lcd.print(text[i]);
  }
}

void loopLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("B: ");
  lcd.print(relayState1 ? "ON " : "OFF");
  lcd.print(" L: ");
  lcd.print(relayState2 ? "ON" : "OFF");

  lcd.setCursor(0, 1);
  lcd.print("F: ");
  lcd.print(relayState3 ? "ON " : "OFF");
  lcd.print(" I: ");
  lcd.print(relayState4 ? "ON" : "OFF");
}

// == TASK SCHEDULER == //

struct Task {
  unsigned long lastRun;
  unsigned long interval;
  void (*callback)();
};

Task tasks[] = {
  {0, 50, controlRelays},
  {0, 500, readSensorEvent},
  {0, 1000, loopLCD},
  {0, 1500, alternateNetworkEvent}
};

const int numTasks = sizeof(tasks) / sizeof(Task);

// == MAIN == //

void setup() {
  Serial.begin(9600);
  ESP_SERIAL.begin(9600);
  MP3_SERIAL.begin(9600);
  delay(500);

  initPins();
  initLCD();
    
  renderToLCD("Initializing MP3");
  initMP3Player();
  initESP();
}

void loop() {
  unsigned long now = millis();
  for (int i = 0; i < numTasks; i++) {
    if (now - tasks[i].lastRun >= tasks[i].interval) {
      tasks[i].lastRun = now;
      tasks[i].callback();
    }
  }
  handleHTTPResponse();
  delay(1);
}
