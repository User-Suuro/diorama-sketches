#include "ATGet.h"

ATGet::ATGet(ATSend &at, Stream &serialPort, const String &host, int port)
    : _at(at), _serial(serialPort), _host(host), _port(port) {}

// Read all data until idle for idleTimeout ms
String ATGet::_readFullResponse(unsigned long idleTimeout) {
    String response = "";
    unsigned long lastReceived = millis();

    while (millis() - lastReceived < idleTimeout) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            lastReceived = millis();
        }
    }

    return response;
}

// Extract raw body by stripping +IPD headers
String ATGet::_extractRawBody(const String &raw) {
    String rawBody = "";
    int ipdPos = 0;

    while ((ipdPos = raw.indexOf("+IPD,", ipdPos)) != -1) {
        int colon = raw.indexOf(":", ipdPos);
        if (colon != -1) {
            String chunk = raw.substring(colon + 1);
            int nextChunk = chunk.indexOf("+IPD,");
            if (nextChunk != -1) chunk = chunk.substring(0, nextChunk);
            int closed = chunk.indexOf("CLOSED");
            if (closed != -1) chunk = chunk.substring(0, closed);
            rawBody += chunk;
            ipdPos = colon + 1;
        } else break;
    }

    rawBody.trim();
    return rawBody;
}

// Perform HTTPS GET request (prints raw body)
String ATGet::getRequest(const String &path, bool printDebug) {
    if (printDebug)
        Serial.println("🌐 [ATGet] Starting HTTPS GET...");

    // 1️⃣ Start SSL connection
    while (true) {
        _serial.println("AT+CIPSTART=\"SSL\",\"" + _host + "\"," + String(_port));
        if (printDebug) Serial.println("→ AT+CIPSTART=\"SSL\",\"" + _host + "\"," + String(_port));

        String connectResp = "";
        unsigned long start = millis();
        bool connected = false;

        // Wait up to 8 seconds for "CONNECT" or retry
        while (millis() - start < 8000) {
            while (_serial.available()) {
                char c = _serial.read();
                connectResp += c;
                if (connectResp.indexOf("CONNECT") != -1) {
                    connected = true;
                    break;
                }
            }
            if (connected) break;
        }

        if (connected) {
            if (printDebug) Serial.println("✅ SSL CONNECTED");
            break;
        } else {
            Serial.println("❌ SSL failed, retrying in 3s...");
            delay(3000);
        }
    }

        // 2️⃣ Compose HTTP request
    String httpRequest = "GET " + path + " HTTP/1.1\r\n";
    httpRequest += "Host: " + _host + "\r\n";
    httpRequest += "Connection: close\r\n\r\n";

    // 3️⃣ Announce sending — block until ESP ready for input ('>' prompt)
    _serial.println("AT+CIPSEND=" + String(httpRequest.length()));
    if (printDebug) Serial.println("→ AT+CIPSEND=" + String(httpRequest.length()));

    String sendResp = "";
    unsigned long promptStart = millis();
    bool gotPrompt = false;

    while (millis() - promptStart < 8000) {  // Wait up to 8s for '>'
        while (_serial.available()) {
            char c = _serial.read();
            sendResp += c;
            if (sendResp.indexOf('>') != -1) {
                gotPrompt = true;
                break;
            }
        }
        if (gotPrompt) break;
    }

    if (!gotPrompt) {
        Serial.println("⚠️ No '>' prompt received, aborting.");
        return "";
    }

    // 4️⃣ Send actual HTTP request
    _serial.print(httpRequest);
    if (printDebug) {
        Serial.println("→ Sent HTTP request:");
        Serial.println(httpRequest);
    }

    // 5️⃣ Wait for full response until "CLOSED"
    if (printDebug) Serial.println("📨 Waiting for full response...");

    String response = "";
    String window = "";           // last few chars to detect "CLOSED"
    unsigned long lastData = millis();
    bool closed = false;

    while (!closed) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            window += c;
            if (window.length() > 6) window.remove(0, 1);  // keep 6-char sliding window
            lastData = millis();

            if (window.endsWith("CLOSED")) {
                closed = true;
                break;
            }
        }

        if (millis() - lastData > 5000) { // timeout after 5s no data
            Serial.println("⚠️ Timeout waiting for CLOSED.");
            break;
        }
    }

    // 6️⃣ Show full raw response
    Serial.println("← Full Raw Response:");
    Serial.println(response);

    // 7️⃣ Extract raw body
    String rawBody = "";
    int ipdPos = response.indexOf("+IPD,");
    if (ipdPos != -1) {
        int colon = response.indexOf(':', ipdPos);
        if (colon != -1) {
            rawBody = response.substring(colon + 1);
            int closedPos = rawBody.indexOf("CLOSED");
            if (closedPos != -1)
                rawBody = rawBody.substring(0, closedPos);
        }
    }

    rawBody.trim();
    Serial.println("📦 RawBody:");
    Serial.println(rawBody);

    // 8️⃣ Close connection cleanly
    _serial.println("AT+CIPCLOSE");
    delay(500);

    return rawBody;

}

