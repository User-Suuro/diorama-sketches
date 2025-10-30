#include "ATSend.h"

ATSend::ATSend(Stream &serial, unsigned long timeout)
    : _serial(serial), _timeout(timeout) {}

String ATSend::_readResponse() {
    String response = "";
    unsigned long start = millis();

    // Wait for first data
    while (!_serial.available() && (millis() - start < _timeout)) {
        delay(10);
    }

    if (!_serial.available()) {
        return ""; // timeout, no data
    }

    unsigned long lastByteTime = millis();
    const unsigned long innerTimeout = 200; // ms between bytes

    while (millis() - lastByteTime < innerTimeout) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            lastByteTime = millis();
        }
    }

    return response;
}

String ATSend::sendCommand(const String &cmd, bool printDebug) {
    // Flush any stale data
    while (_serial.available()) _serial.read();

    _serial.println(cmd);

    if (printDebug) {
        Serial.print("→ "); Serial.println(cmd);
    }

    String resp = _readResponse();

    if (printDebug) {
        Serial.println("← Response:");
        Serial.println(resp);
    }

    return resp;
}

bool ATSend::sendCommandWaitOK(const String &cmd, bool printDebug) {
    String resp = sendCommand(cmd, printDebug);
    if (resp.indexOf("OK") != -1) return true;
    if (resp.indexOf("ERROR") != -1) return false;
    return false; // timeout or unknown response
}
