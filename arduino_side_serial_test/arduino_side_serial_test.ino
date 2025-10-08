#include <SoftwareSerial.h>

SoftwareSerial ESPSerial(2, 3); // RX = 2, TX = 3

const char* SSID = "X8b";
const char* PASS = "12345678";

bool waitForResponse(unsigned long timeout = 15000) {
  unsigned long start = millis();
  String response = "";

  Serial.println("Waiting for response from ESP32...");

  while (millis() - start < timeout) {
    while (ESPSerial.available()) {
      char c = ESPSerial.read();
      response += c;
      Serial.print(c);
    }

    if (response.indexOf("WIFI_CONNECTED") != -1) {
      Serial.println("\n✅ WiFi connected successfully!");
      return true;
    }

    if (response.indexOf("WIFI_FAILED") != -1) {
      Serial.println("\n❌ WiFi connection failed.");
      return false;
    }
  }

  Serial.println("\n⚠ Timeout waiting for response.");
  return false;
}

void connectToWiFi() {
  while (true) {  // Keep trying until connected
    Serial.println("\nSending WiFi connect command...");
    
    ESPSerial.print("+AT=CONNECT_WIFI,\"");
    ESPSerial.print(SSID);
    ESPSerial.print("\",\"");
    ESPSerial.print(PASS);
    ESPSerial.println("\"");

    if (waitForResponse()) {
      break;  // Stop looping once connected
    }

    Serial.println("Retrying in 5 seconds...\n");
    delay(5000);
  }
}

void setup() {
  Serial.begin(9600);
  ESPSerial.begin(9600);
  delay(2000);

  connectToWiFi();  // Block until Wi-Fi is connected
}

void loop() {
  // You can now safely proceed with network-dependent tasks here
}
