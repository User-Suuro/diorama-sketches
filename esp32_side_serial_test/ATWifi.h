#ifndef ATWIFI_H
#define ATWIFI_H

#include <Arduino.h>
#include <WiFi.h>

class ATWiFi {
public:
  ATWiFi(HardwareSerial& serial);
  void begin();
  void listen();
  
  // Core Wi-Fi actions (public so .ino can call them)
  void connectToWiFi(String ssid, String password);
  void checkWiFiStatus();
  void disconnectWiFi();

  // Serial reference for printing responses
  HardwareSerial& serialRef;
};

#endif
