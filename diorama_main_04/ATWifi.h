#ifndef ATWIFI_H
#define ATWIFI_H

#include <Arduino.h>
#include "ATUtils.h"

/**
 * ATWifi
 * -------
 * Wi-Fi management class for ESP32 AT Firmware (v4.1.1.0)
 * Built on top of ATUtils.
 */
class ATWifi {
private:
    ATUtils& at;  // Reference to ATUtils instance

public:
    ATWifi(ATUtils& atUtils);

    // === Core Wi-Fi Operations ===
    bool connect(const char* ssid, const char* password, unsigned long timeout = 15000);
    bool disconnect();
    bool isConnected();
    bool getIP(String& ip);
    bool getMAC(String& mac);
    bool getRSSI(int& rssi);
    bool setAutoConnect(bool enable);

    // === Utility ===
    void printStatus();
};

#endif
