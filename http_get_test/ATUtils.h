#ifndef ATUTILS_H
#define ATUTILS_H

#include <Arduino.h>

class ATUtils {
public:
  ATUtils(Stream &stream, Stream *debugSerial = nullptr);
  String sendCommand(const String &cmd, unsigned long timeoutMs = 5000, unsigned long silentMs = 500);
  Stream &stream;

private:
  Stream *debug;

  String readResponse(const String &cmd, unsigned long timeoutMs, unsigned long silentMs);
  static String trimBoth(const String &s);
};

#endif
