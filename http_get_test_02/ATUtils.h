#ifndef ATUTILS_H
#define ATUTILS_H

#include <Arduino.h>

class ATUtils {
public:
    ATUtils(Stream &stream, Stream *debugSerial = nullptr);

    // Sends an AT command and stores the response in buffer
    // Returns length of the response
    size_t sendCommand(const char* cmd, char* responseBuffer, size_t bufferSize, 
                       unsigned long timeoutMs = 5000, unsigned long silentMs = 500);

    Stream &stream;

private:
    Stream *debug;

    // Reads response into buffer, returns length
    size_t readResponse(char* buffer, size_t bufferSize, unsigned long timeoutMs, unsigned long silentMs);

    // Trims leading/trailing whitespace in buffer
    static void trimBoth(char* buffer);
};

#endif
