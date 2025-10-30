#ifndef ATWIFI_H
#define ATWIFI_H

#include <Arduino.h>
#include "ATUtils.h"

class ATWifi {
public:
  ATWifi(ATUtils &atUtils);

  // Check if WiFi is connected (returns true if module has an IP)
  bool isConnected();

  // Optional: connect to WiFi
  bool connect(const String &ssid, const String &password, unsigned long timeoutMs = 10000);

private:
  ATUtils &at;
};

#endif
