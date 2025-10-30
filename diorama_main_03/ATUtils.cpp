// ATUtil.cpp
#include "ATUtils.h"

ATUtils::ATUtils(Stream &serial, unsigned long timeout)
  : _serial(serial), _timeout(timeout) {}

void ATUtils::clearBuffer() {
  while (_serial.available()) {
    _serial.read();
  }
}

bool ATUtils::sendCommand(const String &cmd, String &response, const String &expected) {
  clearBuffer();

  // Send the AT command with CRLF
  _serial.println(cmd);

  unsigned long startTime = millis();
  response = "";

  while (millis() - startTime < _timeout) {
    while (_serial.available()) {
      char c = _serial.read();
      response += c;
    }

    // If expected keyword is found, we’re done
    if (response.indexOf(expected) != -1) {
      return true;
    }

    // If error is found early
    if (response.indexOf("ERROR") != -1 || response.indexOf("FAIL") != -1) {
      return false;
    }
  }

  // Timeout
  return false;
}
