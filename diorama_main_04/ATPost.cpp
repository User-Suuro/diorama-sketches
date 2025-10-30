#include "ATPost.h"

ATPost::ATPost(ATUtils& atUtils, bool debug)
    : at(atUtils), debugMode(debug) {
    connected = false;
    success = false;
    state = ATPOST_IDLE;
}

void ATPost::beginAsync(const String& host, int port, const String& endpoint, const String& payload, unsigned long timeout) {
    if (state != ATPOST_IDLE && state != ATPOST_DONE) {
        if (debugMode) Serial.println("⚠️ [ATPost] Request already in progress, skipping...");
        return;
    }

    this->host = host;
    this->port = port;
    this->endpoint = endpoint;
    this->payload = payload;
    this->readTimeout = timeout;
    this->stateStart = millis();
    this->success = false;
    this->accumulated = "";
    this->lastResponse = "";

    // If already connected, reuse socket
    if (connected) {
        if (debugMode) Serial.println("🔁 [ATPost] Reusing existing socket...");
        state = ATPOST_SENDING_HEADER;
        return;
    }

    // Otherwise, connect
    state = ATPOST_CONNECTING;
    at.flushInput();
    String protocol = (port == 443) ? "SSL" : "TCP";
    at.sendCommand("AT+CIPSTART=\"" + protocol + "\",\"" + host + "\"," + String(port));

    if (debugMode) Serial.println("🌐 [ATPost] beginAsync (connecting new socket)");
}

void ATPost::update() {
    String chunk = at.pollSerial();
    if (chunk.length()) {
        accumulated += chunk;
        if (debugMode && chunk.length() < 200) {
            Serial.print("📡 [ATPost] Poll: ");
            Serial.println(chunk);
        }
    }

    switch (state) {
        // ------------------------------------------------
        case ATPOST_CONNECTING: {
            if (accumulated.indexOf("ALREADY CONNECTED") >= 0) {
                if (debugMode) Serial.println("🔁 [ATPost] Socket already connected.");
                connected = true;
                accumulated = "";
                state = ATPOST_SENDING_HEADER;
            }
            else if (accumulated.indexOf("CONNECT") >= 0 && accumulated.indexOf("OK") >= 0) {
                if (debugMode) Serial.println("✅ [ATPost] Connection established.");
                connected = true;
                accumulated = "";
                state = ATPOST_SENDING_HEADER;
            }
            else if (accumulated.indexOf("busy p") >= 0 || accumulated.indexOf("ERROR") >= 0) {
                if (debugMode) Serial.println("⚠️ [ATPost] Busy, retrying connection...");
                at.sendCommand("AT+CIPCLOSE");
                at.expectOK(1000);
                connected = false;
                accumulated = "";
                delay(150);
                String protocol = (port == 443) ? "SSL" : "TCP";
                at.sendCommand("AT+CIPSTART=\"" + protocol + "\",\"" + host + "\"," + String(port));
                stateStart = millis();
            }
            else if (millis() - stateStart > 15000) {
                if (debugMode) Serial.println("❌ [ATPost] Timeout connecting.");
                connected = false;
                state = ATPOST_ERROR;
            }
            break;
        }

        // ------------------------------------------------
        case ATPOST_SENDING_HEADER: {
          // === Build proper HTTP POST request ===
            String request =
                "POST " + endpoint + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: Arduino-Async/1.0\r\n" +
                "Content-Type: application/json\r\n" +
                "Content-Length: " + String(payload.length()) + "\r\n" +
                "Connection: Connection: keep-alive\r\n\r\n" +  
                payload + "\r\n";       

            if (debugMode) {
                Serial.println("🧾 [ATPost] Sending request:");
                Serial.println(request);
            }

            // === Tell ESP how many bytes ===
            at.sendCommand("AT+CIPSEND=" + String(request.length()));
            delay(50);

            // === Write full request ===
            at.sendCommand(request, false);

            accumulated = "";
            state = ATPOST_READING;
            stateStart = millis();
            break;
        }


        // ------------------------------------------------
        case ATPOST_READING: {
            if (chunk.length()) {
                int ipdIndex = chunk.indexOf("+IPD,");
                if (ipdIndex >= 0) {
                    int colonIndex = chunk.indexOf(':', ipdIndex);
                    if (colonIndex > 0 && colonIndex < (int)chunk.length() - 1) {
                        String data = chunk.substring(colonIndex + 1);
                        lastResponse += data;
                    }
                } else {
                    lastResponse += chunk;
                }
            }

            if (chunk.indexOf("CLOSED") >= 0) {
                if (debugMode) Serial.println("⚠️ [ATPost] Server closed connection.");
                connected = false;
                state = ATPOST_DONE;
                success = lastResponse.indexOf("HTTP/1.1 200") >= 0 || lastResponse.indexOf("201") >= 0;
            }
            else if (millis() - stateStart > readTimeout) {
                success = lastResponse.indexOf("HTTP/1.1 200") >= 0 || lastResponse.indexOf("201") >= 0;
                state = ATPOST_DONE;
                if (debugMode) Serial.println("⏱️ [ATPost] Timeout reached, completing POST.");
            }

            if (state == ATPOST_DONE && debugMode) {
                Serial.println("📥 [ATPost] Response complete (Keep-Alive):");
                Serial.println(lastResponse);
            }
            break;
        }

        // ------------------------------------------------
        case ATPOST_ERROR: {
            at.sendCommand("AT+CIPCLOSE");
            at.expectOK(1000);
            connected = false;
            state = ATPOST_DONE;
            success = false;
            break;
        }

        // ------------------------------------------------
        case ATPOST_DONE: {
            // Reset to idle so next request can run
            state = ATPOST_IDLE;
            break;
        }

        default:
            break;
    }
}

bool ATPost::isFinished() const { return state == ATPOST_DONE || state == ATPOST_IDLE; }
bool ATPost::isSuccess() const { return success; }
String ATPost::getLastResponse() const { return lastResponse; }
bool ATPost::isConnected() const { return connected; }
void ATPost::setConnected(bool value) { connected = value; }
