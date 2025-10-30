#ifndef ATWIFI_H
#define ATWIFI_H

#include <Arduino.h>
#include "ATUtils.h"

class ATWifi {
public:
  ATWifi(ATUtils &util, const String &ssid, const String &password);

  bool connect(bool printDebug = true);
  bool isConnected(bool printDebug = true);
  bool disconnect(bool printDebug = true);

private:
  ATUtils &_util;
  String _ssid;
  String _password;
};

#endif
