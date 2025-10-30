#include "ATSend.h"

ATSend::ATSend(Stream &serial, unsigned long timeout)
    : _serial(serial), _timeout(timeout) {}

/**
 * @brief Reads all available characters from the serial until no new data arrives.
 */
String ATSend::_readResponse() {
    String response = "";
    unsigned long start = millis();

    // Wait until the ESP starts responding
    while (!_serial.available() && (millis() - start < _timeout)) {
        delay(10);
    }

    if (!_serial.available()) {
        return ""; // No response at all
    }

    // Continue reading until data stops arriving
    unsigned long lastByteTime = millis();
    const unsigned long interByteTimeout = 1000; // max idle between bytes

    while (millis() - lastByteTime < interByteTimeout) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            lastByteTime = millis();
        }
    }

    return response;
}

String ATSend::sendCommand(const String &cmd, bool printDebug) {
    // Clear any pending garbage input
    while (_serial.available()) _serial.read();

    // Send AT command
    if (cmd.length() > 0) {
        _serial.println(cmd);
        delay(100);
        if (printDebug) {
            Serial.print(F("→ "));
            Serial.println(cmd);
        }
    }

    // Wait for and capture response
    String resp = _readResponse();

    if (printDebug) {
        Serial.println(F("← Response:"));
        Serial.println(resp);
    }

    return resp;
}

bool ATSend::sendCommandWaitOK(const String &cmd, bool printDebug) {
    String resp = sendCommand(cmd, printDebug);
    if (resp.indexOf("OK") != -1) return true;
    if (resp.indexOf("ERROR") != -1) return false;
    return false;
}
