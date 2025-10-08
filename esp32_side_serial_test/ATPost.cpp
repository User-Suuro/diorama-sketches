#include "ATPost.h"

ATPost::ATPost(HardwareSerial& serial, ATWiFi& wifi)
  : serialRef(serial), atwifi(wifi) {}

void ATPost::begin(const char* rootCA, const char* endpointUrl) {
  client.setCACert(rootCA);
  endpoint = endpointUrl;
  serialRef.println("OK"); // Signal ready
  Serial.println("[ATPost] Ready to handle HTTPS requests.");
}

void ATPost::sendPost(const String& payload) {
  if (WiFi.status() != WL_CONNECTED) {
    serialRef.println("ERROR: WIFI_NOT_CONNECTED");
    return;
  }

  HTTPClient https;
  https.begin(client, endpoint);
  https.addHeader("Content-Type", "application/json");

  Serial.println("[ATPost] Sending HTTPS POST...");
  int httpCode = https.POST(payload);
  String response = https.getString();

  serialRef.printf("HTTP_POST_CODE=%d\n", httpCode);
  serialRef.println("HTTP_POST_RESPONSE_BEGIN");
  serialRef.println(response);
  serialRef.println("HTTP_POST_RESPONSE_END");

  https.end();
  Serial.println("[ATPost] POST request completed.");
}

void ATPost::sendGet() {
  if (WiFi.status() != WL_CONNECTED) {
    serialRef.println("ERROR: WIFI_NOT_CONNECTED");
    return;
  }

  HTTPClient https;
  https.begin(client, endpoint);

  Serial.println("[ATPost] Sending HTTPS GET...");
  int httpCode = https.GET();
  String response = https.getString();

  serialRef.printf("HTTP_GET_CODE=%d\n", httpCode);
  serialRef.println("HTTP_GET_RESPONSE_BEGIN");
  serialRef.println(response);
  serialRef.println("HTTP_GET_RESPONSE_END");

  https.end();
  Serial.println("[ATPost] GET request completed.");
}

void ATPost::checkConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    serialRef.println("WIFI_CONNECTED");
  } else {
    serialRef.println("WIFI_NOT_CONNECTED");
  }
}
