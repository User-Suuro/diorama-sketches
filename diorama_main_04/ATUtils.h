#ifndef ATUTILS_H
#define ATUTILS_H

#include <Arduino.h>

/**
 * ATUtils
 * ----------
 * Core AT command utility for ESP32 AT Firmware (v4.1.1.0).
 * Designed for Arduino Mega using hardware Serial3 (TX3/RX3).
 */
class ATUtils {
private:
    HardwareSerial& espSerial;       // Dedicated serial to ESP32
    unsigned long defaultTimeout;    // Default timeout (ms)

public:
    ATUtils(HardwareSerial& serialPort, unsigned long timeout = 5000);

    // ===== Core Communication =====
    void sendCommand(const String& cmd, bool newline = true);
    String readResponse(unsigned long timeout = 0);
    String pollSerial();
    bool waitForResponse(const String& expected, unsigned long timeout = 0);
    bool expectOK(unsigned long timeout = 0);
    void flushInput();

    // ===== Basic Controls =====
    bool testAT();
    bool setEcho(bool enabled);
    bool setMode(uint8_t mode);  // 1=Station, 2=AP, 3=Both
    bool resetModule();
    bool setBaudRate(long baud);

    // ===== Utility =====
    static String trimResponse(String raw);
    static bool containsIgnoreCase(const String& src, const String& target);
};

#endif
