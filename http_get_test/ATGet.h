#ifndef ATGET_H
#define ATGET_H

#include <Arduino.h>
#include "ATUtils.h"
#include "ATWifi.h"

class ATGet {
public:
    ATGet(ATUtils &atUtils, ATWifi &wifiModule);

    // Perform HTTP GET request
    String get(const String &host, const String &path, int port = 443, unsigned long timeoutMs = 15000);

private:
    ATUtils &at;
    ATWifi &wifi;

    bool startConnection(const String &host, int port);
    String sendHttpRequest(const String &host, const String &path);
    String readResponse(unsigned long timeoutMs);
};

#endif
