#ifndef ATWIFI_H
#define ATWIFI_H

#include <Arduino.h>
#include "ATUtils.h"

class ATWifi {
public:
    ATWifi(ATUtils &atUtils);

    // Check if WiFi is connected (returns true if module has an IP)
    bool isConnected();

    // Connect to WiFi (station mode)
    bool connect(const char* ssid, const char* password, unsigned long timeoutMs = 10000);

private:
    ATUtils &at;

    // Helper to convert response to uppercase in-place
    void toUpperCase(char* str);
};

#endif
