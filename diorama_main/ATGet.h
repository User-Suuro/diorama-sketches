#ifndef ATGET_H
#define ATGET_H

#include <Arduino.h>
#include "ATSend.h"

class ATGet {
private:
    ATSend &_at;
    Stream &_serial;
    String _host;
    int _port;

    String _readFullResponse(unsigned long idleTimeout);
    String _extractRawBody(const String &raw);

public:
    ATGet(ATSend &at, Stream &serialPort, const String &host, int port);
    String getRequest(const String &path, bool printDebug = true);
};

#endif
