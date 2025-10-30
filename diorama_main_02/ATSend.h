#ifndef ATSEND_H
#define ATSEND_H

#include <Arduino.h>

class ATSend {
private:
    Stream& _serial;
    unsigned long _timeout;

    String _readResponse(); // internal helper

public:
    ATSend(Stream &serial, unsigned long timeout = 8000);

    String sendCommand(const String &cmd, bool printDebug = false);
    bool sendCommandWaitOK(const String &cmd, bool printDebug = false);
};

#endif
