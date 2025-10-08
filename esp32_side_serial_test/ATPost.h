#ifndef ATPOST_H
#define ATPOST_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "ATWiFi.h"

class ATPost {
private:
  HardwareSerial& serialRef;
  WiFiClientSecure client;
  ATWiFi& atwifi;      // Reference to shared WiFi handler
  String endpoint;

public:
  ATPost(HardwareSerial& serial, ATWiFi& wifi);
  void begin(const char* rootCA, const char* endpointUrl);
  void sendPost(const String& payload);
  void sendGet();
  void checkConnection();
};

#endif
