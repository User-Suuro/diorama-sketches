#ifndef ATSEND_H
#define ATSEND_H

#include <Arduino.h>

class ATSend {
public:
    ATSend(Stream &serial, unsigned long timeout = 3000);

    // Send a command and get full response
    String sendCommand(const String &cmd, bool printDebug = true);

    // Wait specifically for "OK" or "ERROR"
    bool sendCommandWaitOK(const String &cmd, bool printDebug = true);
    Stream &getSerial() { return _serial; }
    unsigned long _timeout;
private:
    Stream &_serial;
    String _readResponse();
};

#endif
