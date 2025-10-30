#include "ATUtils.h"

ATUtils::ATUtils(Stream &stream, Stream *debugSerial) : stream(stream), debug(debugSerial) {}

// Sends a command and fills responseBuffer
size_t ATUtils::sendCommand(const char* cmd, char* responseBuffer, size_t bufferSize, 
                            unsigned long timeoutMs, unsigned long silentMs) {
    // Clear input buffer
    while (stream.available()) stream.read();

    if (debug) {
        debug->print(F("📤 CMD → "));
        debug->println(cmd);
    }

    // Send command
    stream.println(cmd);
    delay(10); // let device start responding

    size_t len = readResponse(responseBuffer, bufferSize, timeoutMs, silentMs);
    return len;
}

// Reads response into buffer
size_t ATUtils::readResponse(char* buffer, size_t bufferSize, unsigned long timeoutMs, unsigned long silentMs) {
    unsigned long start = millis();
    unsigned long lastRead = millis();
    size_t index = 0;

    while (millis() - start < timeoutMs) {
        while (stream.available()) {
            int ch = stream.read();
            if (ch >= 0 && index < bufferSize - 1) { // reserve space for null terminator
                buffer[index++] = (char)ch;
                lastRead = millis();
            }
        }

        if (index > 0 && (millis() - lastRead > silentMs)) break;

        delay(1);
    }

    buffer[index] = '\0'; // null terminate

    trimBoth(buffer);

    return index;
}

// Trim leading/trailing whitespace
void ATUtils::trimBoth(char* buffer) {
    size_t len = strlen(buffer);
    size_t start = 0;
    size_t end = len - 1;

    while (start <= end && isspace((unsigned char)buffer[start])) start++;
    while (end >= start && isspace((unsigned char)buffer[end])) end--;

    if (start > 0 || end < len - 1) {
        size_t j = 0;
        for (size_t i = start; i <= end; i++) {
            buffer[j++] = buffer[i];
        }
        buffer[j] = '\0';
    }
}
