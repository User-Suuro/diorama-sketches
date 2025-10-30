#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "JsonBuilder.h"
#include "ATSend.h"
#include "ATGet.h"

#define relay1 4
#define relay2 5
#define relay3 6
#define relay4 7

#define touch1 8
#define touch2 9
#define touch3 10
#define touch4 11

SoftwareSerial espSerial(2, 3); // RX, TX for ESP32 AT module

const char* ssid = "X8b";
const char* password = "12345678";

const char* host = "diorama-endpoint.vercel.app";
const int port = 443;

const char* check_connection_endpoint = "/api/arduino/check-connection";
const char* device_status = "/api/arduino/device-status";

ATSend atSend(espSerial, 8000);  // include timeout
ATGet atGet(atSend, espSerial, host, port);

const unsigned long GET_STATUS_INTERVAL = 3000;   
const unsigned long SEND_VALUES_INTERVAL = 5000;  
const unsigned long CONTROL_RELAYS_INTERVAL = 10; 

// states
unsigned long lastStatusTime = 0;
unsigned long lastSendTime = 0;
unsigned long lastRelayTime = 0;


unsigned long lastAttempt = 0;

bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  delay(1000);

  initPinModes();

  if (!checkESP()) {
    Serial.println("ESP32 not responding. Halting.");
    while (true);  // stop everything
  }

  if (!connectWiFi()) {
    Serial.println("Failed to connect to WiFi after multiple attempts.");
    while (true);  // stop or reset if you prefer
  }

  String resp = atGet.getRequest("/api/arduino/check-connection", true);
  Serial.println("Final Response:");
  Serial.println(resp);

  Serial.println("Setup complete — ready to continue.");
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastStatusTime >= GET_STATUS_INTERVAL) {
    lastStatusTime = currentMillis;

  }

  if (currentMillis - lastSendTime >= SEND_VALUES_INTERVAL) {
    lastSendTime = currentMillis;
  }

  if (currentMillis - lastRelayTime >= CONTROL_RELAYS_INTERVAL) {
    lastRelayTime = currentMillis;
    controlRelays();
  
  }
}

void controlRelays() {
  // Touch 1 controls Relay 1
  if (digitalRead(touch1) == HIGH) {
    relayState1 = !relayState1;
    digitalWrite(relay1, relayState1 ? LOW : HIGH);

    // Wait until touch 1 is released
    while (digitalRead(touch1) == HIGH) {
      delay(10);
    }
    delay(50); // Small debounce delay
  }

  // Touch 2 controls Relay 2
  else if (digitalRead(touch2) == HIGH) {
    relayState2 = !relayState2;
    digitalWrite(relay2, relayState2 ? LOW : HIGH);

    // Wait until touch 2 is released
    while (digitalRead(touch2) == HIGH) {
      delay(10);
    }
    delay(50);
  }

  // Touch 3 controls Relay 3
  else if (digitalRead(touch3) == HIGH) {
    relayState3 = !relayState3;
    digitalWrite(relay3, relayState3 ? LOW : HIGH);

    // Wait until touch 3 is released
    while (digitalRead(touch3) == HIGH) {
      delay(10);
    }
    delay(50);
  }

  // Touch 4 controls all relays (master switch)
  else if (digitalRead(touch4) == HIGH) {
    bool allOn = relayState1 && relayState2 && relayState3 && relayState4;

    relayState1 = !allOn;
    relayState2 = !allOn;
    relayState3 = !allOn;
    relayState4 = !allOn;

    digitalWrite(relay1, relayState1 ? LOW : HIGH);
    digitalWrite(relay2, relayState2 ? LOW : HIGH);
    digitalWrite(relay3, relayState3 ? LOW : HIGH);
    digitalWrite(relay4, relayState4 ? LOW : HIGH);

    // Wait until touch 4 is released
    while (digitalRead(touch4) == HIGH) {
      delay(10);
    }
    delay(50);
  }
}

void initPinModes() {
  pinMode(touch1, INPUT);
  pinMode(touch2, INPUT);
  pinMode(touch3, INPUT);
  pinMode(touch4, INPUT);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

bool checkESP() {
  String response = atSend.sendCommand("AT");
  if (response.indexOf("OK") != -1) {
    Serial.println("✅ ESP32 is ready!");
    return true;
  }
  return false;
}

bool connectWiFi() {
    Serial.println("🔄 Checking current WiFi connection...");
    String status = atSend.sendCommand("AT+CWJAP?");
    if (status.indexOf(ssid) != -1) {
        Serial.println("✅ Already connected to WiFi!");
        return true;
    }

    // Disconnect first
    atSend.sendCommand("AT+CWQAP");
    delay(1000);

    Serial.print("📶 Connecting to WiFi: ");
    Serial.println(ssid);

    String cmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";

    for (int attempt = 1; attempt <= 5; attempt++) {
        Serial.print("→ ");
        Serial.println(cmd);

        String response = atSend.sendCommand(cmd, 10000);  // 10s timeout
        Serial.println("← Response:");
        Serial.println(response);

        if (response.indexOf("WIFI CONNECTED") != -1 || 
            response.indexOf("OK") != -1 ||
            response.indexOf("GOT IP") != -1) {
            Serial.println("✅ WiFi Connected!");
            return true;
        }

        Serial.print("⚠️ Attempt ");
        Serial.print(attempt);
        Serial.println(" failed. Retrying in 3s...");
        delay(3000);
    }

    Serial.println("❌ Failed to connect to WiFi after 5 attempts.");
    return false;
}



// function for dfPlayer

// function for reading touch sensors

// function for handling relays with touch sensors

// function for reading proximity sensor

// function for reading light sensors
