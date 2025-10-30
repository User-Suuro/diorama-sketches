#include "ATWifi.h"
#include <string.h>
#include <ctype.h>

ATWifi::ATWifi(ATUtils &atUtils) : at(atUtils) {}

// Convert char array to uppercase
void ATWifi::toUpperCase(char* str) {
    for (size_t i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Check WiFi connection
bool ATWifi::isConnected() {
    char resp[256]; // buffer for AT response
    at.sendCommand("AT+CWJAP?", resp, sizeof(resp), 3000, 100);

    toUpperCase(resp);

    if (strstr(resp, "CWJAP:") != nullptr) return true;
    if (strstr(resp, "CONNECTED") != nullptr) return true;
    return false;
}

// Connect to WiFi (station mode)
bool ATWifi::connect(const char* ssid, const char* password, unsigned long timeoutMs) {
    // 1️⃣ Skip if already connected
    if (isConnected()) return true;

    // 2️⃣ Set ESP to station mode
    char tmp[64];
    at.sendCommand("AT+CWMODE=1", tmp, sizeof(tmp), 2000, 100);

    // 3️⃣ Join AP
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

    char resp[512];
    at.sendCommand(cmd, resp, sizeof(resp), timeoutMs, 200);

    toUpperCase(resp);
    return strstr(resp, "OK") != nullptr;
}
