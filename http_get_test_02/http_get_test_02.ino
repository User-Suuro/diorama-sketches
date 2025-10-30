#include <SoftwareSerial.h>
#include "ATUtils.h"
#include "ATWifi.h"

SoftwareSerial espSerial(2, 3); // RX, TX

ATUtils at(espSerial);
ATWifi wifi(at);

const char* ssid = "X8b";
const char* password = "123456789";

const char* host = "diorama-endpoint.vercel.app";
const int port = 443;

const char* check_connection_endpoint = "/api/arduino/check-connection";

char response[512]; // buffer for response

void setup() {
    Serial.begin(9600);
    espSerial.begin(9600);
    delay(2000);
    memoryReset();

    Serial.println("AT Test");
   
    // Send simple AT command
    size_t len = at.sendCommand("AT", response, sizeof(response), 3000);



    // Connect WiFi example
    if (wifi.connect(ssid, password, 10000)) {
      Serial.println("✅ WiFi connected");
    } else {
      Serial.println("❌ WiFi failed");
    }

    
    memoryReset();
}


void loop() {
  // put your main code here, to run repeatedly:

}

void memoryReset() {
  memset(response, 0, sizeof(response));
}
