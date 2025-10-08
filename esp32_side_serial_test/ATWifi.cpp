#include "ATWiFi.h"

ATWiFi::ATWiFi(HardwareSerial& serial) : serialRef(serial) {}

void ATWiFi::begin() {
  serialRef.println("OK");  // Signal ready state to Arduino
  Serial.println("ATWiFi Ready (Waiting for AT Commands)");
}

void ATWiFi::listen() {
  // Only handle serial reception; command logic is in .ino
  if (serialRef.available()) {
    String cmd = serialRef.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      Serial.println("Received: " + cmd);
      // Let the .ino handle the command
      // (return it via Serial to main sketch)
      Serial.println("[ATWiFi] Command forwarded to main sketch.");
      // You may emit event-like print if needed, but not required.
    }
  }
}

void ATWiFi::connectToWiFi(String ssid, String password) {
  serialRef.println("CONNECTING...");
  Serial.printf("Connecting to SSID: %s\n", ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  uint8_t retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    serialRef.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    serialRef.println("\nWIFI_CONNECTED");
    serialRef.print("IP=");
    serialRef.println(WiFi.localIP());
  } else {
    serialRef.println("\nWIFI_FAILED");
  }
}

void ATWiFi::checkWiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    serialRef.println("WIFI_OK");
    serialRef.print("IP=");
    serialRef.println(WiFi.localIP());
  } else {
    serialRef.println("WIFI_NOT_CONNECTED");
  }
}

void ATWiFi::disconnectWiFi() {
  WiFi.disconnect();
  serialRef.println("WIFI_DISCONNECTED");
}
