#include "ATGet.h"

ATGet::ATGet(ATUtils& atUtils, bool debug)
    : at(atUtils), debugMode(debug) {
    connected = false;
    state = ATGET_IDLE;
}

void ATGet::beginAsync(const String& host, int port, const String& endpoint, unsigned long timeout) {
    // Prevent duplicate send if still active
    if (state != ATGET_IDLE && state != ATGET_DONE) {
        if (debugMode) Serial.println("⚠️ [ATGet] Request already in progress, skipping...");
        return;
    }

    this->host = host;
    this->port = port;
    this->endpoint = endpoint;
    this->readTimeout = timeout;
    this->stateStart = millis();
    this->success = false;
    this->accumulated = "";
    this->lastResponse = "";

    // Reuse connection if already alive
    if (connected) {
        if (debugMode) Serial.println("🔁 [ATGet] Reusing existing keep-alive socket...");
        state = ATGET_SENDING;
        return;
    }

    // Otherwise open a new socket
    state = ATGET_CONNECTING;
    at.flushInput();

    String protocol = (port == 443) ? "SSL" : "TCP";
    at.sendCommand("AT+CIPSTART=\"" + protocol + "\",\"" + host + "\"," + String(port));

    if (debugMode) Serial.println("🌐 [ATGet] beginAsync (connecting new socket)");
}

void ATGet::update() {
    String chunk = at.pollSerial();
    if (chunk.length()) {
        accumulated += chunk;
        if (debugMode && chunk.length() < 200) {
            Serial.print("📡 [ATGet] Poll: ");
            Serial.println(chunk);
        }
    }

    switch (state) {
        // ------------------------------------------------
        case ATGET_CONNECTING: {
            if (accumulated.indexOf("ALREADY CONNECTED") >= 0) {
                if (debugMode) Serial.println("🔁 [ATGet] Socket already connected.");
                connected = true;
                accumulated = "";
                state = ATGET_SENDING;
            }
            else if (accumulated.indexOf("CONNECT") >= 0 && accumulated.indexOf("OK") >= 0) {
                if (debugMode) Serial.println("✅ [ATGet] Connection established.");
                connected = true;
                accumulated = "";
                state = ATGET_SENDING;
            }
            else if (accumulated.indexOf("busy p") >= 0 || accumulated.indexOf("ERROR") >= 0) {
                if (debugMode) Serial.println("⚠️ [ATGet] Busy, closing and retrying...");
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
                if (debugMode) Serial.println("❌ [ATGet] Timeout connecting.");
                connected = false;
                state = ATGET_ERROR;
            }
            break;
        }

        // ------------------------------------------------
        case ATGET_SENDING: {
            // Prepare request
            String request = "GET " + endpoint + " HTTP/1.1\r\n";
            request += "Host: " + host + "\r\n";
            request += "User-Agent: Arduino-Async/1.0\r\n";
            request += "Connection: keep-alive\r\n\r\n";

            at.sendCommand("AT+CIPSEND=" + String(request.length()));
            delay(100);
            at.sendCommand(request, false);

            accumulated = "";
            state = ATGET_READING;
            stateStart = millis();
            break;
        }

        // ------------------------------------------------
        case ATGET_READING: {
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

            // Detect server closing
            if (chunk.indexOf("CLOSED") >= 0) {
                if (debugMode) Serial.println("⚠️ [ATGet] Server closed the connection.");
                connected = false;
                state = ATGET_DONE;
                success = lastResponse.indexOf("HTTP/1.1") >= 0;
            }
            // Timeout (but keep connection open)
            else if (millis() - stateStart > readTimeout) {
                success = lastResponse.indexOf("HTTP/1.1") >= 0;
                state = ATGET_DONE;
                if (debugMode) Serial.println("⏱️ [ATGet] Read timeout reached, keeping socket open.");
            }

            if (state == ATGET_DONE && debugMode) {
                Serial.println("📥 [ATGet] Response complete (Keep-Alive):");
                Serial.println(lastResponse);
            }
            break;
        }

        // ------------------------------------------------
        case ATGET_ERROR: {
            at.sendCommand("AT+CIPCLOSE");
            at.expectOK(1000);
            connected = false;
            state = ATGET_DONE;
            success = false;
            break;
        }

        // ------------------------------------------------
        case ATGET_DONE: {
            // Reset to idle so next fetch can run
            state = ATGET_IDLE;
            break;
        }

        default:
            break;
    }
}

bool ATGet::isFinished() const {
    return state == ATGET_DONE || state == ATGET_IDLE;
}

bool ATGet::isSuccess() const {
    return success;
}

String ATGet::getLastResponse() const {
    return lastResponse;
}

bool ATGet::isConnected() const {
    return connected;
}

void ATGet::setConnected(bool state) {
    connected = state;
}

