// ATUtil.h
#ifndef ATUTILS_H
#define ATUTILS_H

#include <Arduino.h>

class ATUtils {
public:
  ATUtils(Stream &serial, unsigned long timeout = 5000);

  bool sendCommand(const String &cmd, String &response, const String &expected = "OK");
  void clearBuffer();

private:
  Stream &_serial;
  unsigned long _timeout;
};

#endif
