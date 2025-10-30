#include "ATUtils.h"

// ===== Constructor =====
ATUtils::ATUtils(HardwareSerial& serialPort, unsigned long timeout)
    : espSerial(serialPort), defaultTimeout(timeout) {}


// ===== Send Command =====
void ATUtils::sendCommand(const String& cmd, bool newline) {
    espSerial.print(cmd);
    if (newline) espSerial.print("\r\n");

#if defined(DEBUG_SERIAL)
    DEBUG_SERIAL.print("📤 [AT] ");
    DEBUG_SERIAL.println(cmd);
#endif
}


// ===== Read Response (Non-blocking, cooperative) =====
String ATUtils::readResponse(unsigned long timeout) {
    unsigned long start = millis();
    unsigned long lastByteTime = millis();
    String response = "";

    while (millis() - start < (timeout ? timeout : defaultTimeout)) {
        if (espSerial.available()) {
            char c = espSerial.read();
            response += c;
            lastByteTime = millis();
        } else {
            // if no data for 50ms, assume response ended
            if (millis() - lastByteTime > 50) break;
            yield(); // ✅ allow other tasks to run
        }
    }

#if defined(DEBUG_SERIAL)
    if (response.length()) {
        DEBUG_SERIAL.print("📥 [RESP] ");
        DEBUG_SERIAL.println(response);
    } else {
        DEBUG_SERIAL.println("⚠️ No response received.");
    }
#endif

    return trimResponse(response);
}


// ===== Wait for Specific Response (Cooperative) =====
bool ATUtils::waitForResponse(const String& expected, unsigned long timeout) {
    unsigned long start = millis();
    unsigned long lastByteTime = millis();
    String buffer;

    while (millis() - start < (timeout ? timeout : defaultTimeout)) {
        if (espSerial.available()) {
            char c = espSerial.read();
            buffer += c;
            lastByteTime = millis();

            if (buffer.indexOf(expected) >= 0) {
#if defined(DEBUG_SERIAL)
                DEBUG_SERIAL.print("✅ [Match] ");
                DEBUG_SERIAL.println(expected);
#endif
                return true;
            }
        } else {
            if (millis() - lastByteTime > 50) break;
            yield();  // ✅ cooperative multitasking
        }
    }

#if defined(DEBUG_SERIAL)
    DEBUG_SERIAL.print("❌ Timeout waiting for: ");
    DEBUG_SERIAL.println(expected);
#endif
    return false;
}


// ===== Expect "OK" =====
bool ATUtils::expectOK(unsigned long timeout) {
    return waitForResponse("OK", timeout);
}


// ===== Flush Serial Input =====
void ATUtils::flushInput() {
    while (espSerial.available()) espSerial.read();
}


// ===== AT Module Tests =====
bool ATUtils::testAT() {
    sendCommand("AT");
    return expectOK();
}

bool ATUtils::setEcho(bool enabled) {
    sendCommand(String("ATE") + (enabled ? "1" : "0"));
    return expectOK();
}

bool ATUtils::setMode(uint8_t mode) {
    sendCommand("AT+CWMODE=" + String(mode));
    return expectOK();
}

bool ATUtils::resetModule() {
    sendCommand("AT+RST");
    return waitForResponse("ready", 8000);
}

bool ATUtils::setBaudRate(long baud) {
    sendCommand("AT+UART_DEF=" + String(baud) + ",8,1,0,0");
    return expectOK();
}


// ===== Utility Helpers =====
String ATUtils::trimResponse(String raw) {
    raw.trim();
    raw.replace("\r", "");
    raw.replace("\n\n", "\n");
    return raw;
}

bool ATUtils::containsIgnoreCase(const String& src, const String& target) {
    String lowerSrc = src;
    String lowerTarget = target;
    lowerSrc.toLowerCase();
    lowerTarget.toLowerCase();
    return lowerSrc.indexOf(lowerTarget) >= 0;
}

String ATUtils::pollSerial() {
    String data = "";
    unsigned long start = millis();
    while (espSerial.available() && millis() - start < 10) { // read small bursts
        data += (char)espSerial.read();
        yield();  // ✅ don’t block the CPU
    }
    return data;
}
