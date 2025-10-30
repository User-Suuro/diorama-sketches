#include "ATPost.h"

ATPost::ATPost(ATSend &at, Stream &serialPort, const String &host, int port)
    : _at(at), _serial(serialPort), _host(host), _port(port) {}

// ---------- Blocking Mode ----------
String ATPost::postRequest(const String &path, const String &jsonBody, bool printDebug) {
    beginAsync(path, jsonBody, printDebug);
    while (isBusy()) updateAsync();
    return getResponse();
}

// ---------- Asynchronous Mode ----------
void ATPost::beginAsync(const String &path, const String &jsonBody, bool printDebug) {
    if (_state != IDLE) return;

    _pendingPath = path;
    _pendingBody = jsonBody;
    _printDebug = printDebug;
    _response = "";
    _sendBuffer = "";
    _state = CONNECTING;
    _lastAction = millis();

    if (_printDebug) Serial.println(F("📡 [ATPost] beginAsync"));
    String startCmd = "AT+CIPSTART=\"SSL\",\"" + _host + "\"," + String(_port);
    _serial.println(startCmd);
}

void ATPost::updateAsync() {
    switch (_state) {
        case IDLE:
            return;

        case CONNECTING: {
            while (_serial.available()) {
                char c = _serial.read();
                _response += c;
                _lastAction = millis();
            }

            if (_response.indexOf("CONNECT") != -1) {
                if (_printDebug) Serial.println(F("✅ SSL CONNECTED"));

                String httpRequest =
                    "POST " + _pendingPath + " HTTP/1.1\r\n" +
                    "Host: " + _host + "\r\n" +
                    "User-Agent: Arduino-ESP32\r\n" +
                    "Content-Type: application/json\r\n" +
                    "Content-Length: " + String(_pendingBody.length()) + "\r\n" +
                    "Connection: close\r\n\r\n" +
                    _pendingBody;

                _sendBuffer = httpRequest;
                _serial.println("AT+CIPSEND=" + String(httpRequest.length()));
                _response = "";
                _state = WAIT_PROMPT;
                _lastAction = millis();
            }

            if (millis() - _lastAction > 8000) {
                if (_printDebug) Serial.println(F("⚠️ CONNECT timeout"));
                _serial.println("AT+CIPCLOSE");
                _state = ERROR;
            }
            break;
        }

        case WAIT_PROMPT: {
            while (_serial.available()) {
                char c = _serial.read();
                _response += c;
                _lastAction = millis();
            }

            if (_response.indexOf('>') != -1) {
                if (_printDebug) Serial.println(F("✉️ Sending POST body..."));
                _serial.print(_sendBuffer);
                _response = "";
                _state = READING;
                _lastAction = millis();
            }

            if (millis() - _lastAction > 8000) {
                if (_printDebug) Serial.println(F("⚠️ No prompt received"));
                _serial.println("AT+CIPCLOSE");
                _state = ERROR;
            }
            break;
        }

        case READING: {
            while (_serial.available()) {
                char c = _serial.read();
                _response += c;
                _lastAction = millis();
            }

            // When no data for 1s, consider done
            if (millis() - _lastAction > 1000 && _response.length() > 0) {
                String body = _extractRawBody(_response);
                _response = body;
                _serial.println("AT+CIPCLOSE");
                _state = DONE;
                if (_printDebug) {
                    Serial.println(F("✅ POST complete"));
                    Serial.println(F("📦 Response:"));
                    Serial.println(body);
                }
            }
            break;
        }

        case ERROR:
            // always ensure socket closed
            _serial.println("AT+CIPCLOSE");
            _state = IDLE;
            break;

        case DONE:
            break;
    }
}

bool ATPost::isBusy() const { return _state != IDLE && _state != DONE && _state != ERROR; }
bool ATPost::hasResponse() const { return _state == DONE; }

String ATPost::getResponse() { return _response; }

void ATPost::clearResponse() {
    _state = IDLE;
    _response = "";
    _sendBuffer = "";
}

String ATPost::_readFullResponse(unsigned long idleTimeout) {
    String response;
    unsigned long lastData = millis();
    while (millis() - lastData < idleTimeout) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            lastData = millis();
        }
    }
    return response;
}

String ATPost::_extractRawBody(const String &raw) {
    int start = raw.indexOf("\r\n\r\n");
    if (start == -1) return raw;
    String body = raw.substring(start + 4);
    int closed = body.indexOf("CLOSED");
    if (closed != -1) body = body.substring(0, closed);
    body.trim();
    return body;
}
