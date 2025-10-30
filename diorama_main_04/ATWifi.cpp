#include "ATWifi.h"

ATWifi::ATWifi(ATUtils& atUtils) : at(atUtils) {}

// ===== Connect to Wi-Fi =====
bool ATWifi::connect(const char* ssid, const char* password, unsigned long timeout) {
    at.flushInput();
    at.sendCommand("AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"");

#if defined(DEBUG_SERIAL)
    DEBUG_SERIAL.println("🌐 Connecting to Wi-Fi...");
#endif

    unsigned long start = millis();
    while (millis() - start < timeout) {
        String resp = at.readResponse(500);
        if (ATUtils::containsIgnoreCase(resp, "WIFI CONNECTED") ||
            ATUtils::containsIgnoreCase(resp, "GOT IP")) {
#if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.println("✅ Wi-Fi Connected!");
#endif
            return true;
        }
        if (ATUtils::containsIgnoreCase(resp, "FAIL")) {
#if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.println("❌ Wi-Fi Connection Failed!");
#endif
            return false;
        }
    }

#if defined(DEBUG_SERIAL)
    DEBUG_SERIAL.println("⏰ Wi-Fi connection timeout.");
#endif
    return false;
}


// ===== Disconnect from Wi-Fi =====
bool ATWifi::disconnect() {
    at.sendCommand("AT+CWQAP");
    return at.expectOK();
}


// ===== Check if Wi-Fi is Connected =====
bool ATWifi::isConnected() {
    at.sendCommand("AT+CWJAP?");
    String resp = at.readResponse();
    return resp.indexOf("No AP") == -1 && resp.indexOf("OK") != -1;
}


// ===== Get Local IP Address =====
bool ATWifi::getIP(String& ip) {
    at.sendCommand("AT+CIFSR");
    String resp = at.readResponse();
    int idx = resp.indexOf("+CIFSR:STAIP,\"");
    if (idx >= 0) {
        int start = idx + 14;
        int end = resp.indexOf("\"", start);
        ip = resp.substring(start, end);
        return true;
    }
    return false;
}


// ===== Get MAC Address =====
bool ATWifi::getMAC(String& mac) {
    at.sendCommand("AT+CIFSR");
    String resp = at.readResponse();
    int idx = resp.indexOf("+CIFSR:STAMAC,\"");
    if (idx >= 0) {
        int start = idx + 15;
        int end = resp.indexOf("\"", start);
        mac = resp.substring(start, end);
        return true;
    }
    return false;
}


// ===== Get RSSI (Signal Strength) =====
bool ATWifi::getRSSI(int& rssi) {
    at.sendCommand("AT+CWJAP?");
    String resp = at.readResponse();
    int idx = resp.indexOf(",") + 1;
    if (idx > 0) {
        int end = resp.indexOf(",", idx);
        String rssiStr = resp.substring(idx, end);
        rssi = rssiStr.toInt();
        return true;
    }
    return false;
}


// ===== Enable/Disable Auto Connect =====
bool ATWifi::setAutoConnect(bool enable) {
    at.sendCommand("AT+CWAUTOCONN=" + String(enable ? "1" : "0"));
    return at.expectOK();
}


// ===== Print Wi-Fi Status =====
void ATWifi::printStatus() {
    String ip, mac;
    if (getIP(ip) && getMAC(mac)) {
#if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("📶 Wi-Fi Status:");
        DEBUG_SERIAL.print("   IP: "); DEBUG_SERIAL.println(ip);
        DEBUG_SERIAL.print("   MAC: "); DEBUG_SERIAL.println(mac);
#endif
    } else {
#if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("⚠️ Could not retrieve Wi-Fi status.");
#endif
    }
}
