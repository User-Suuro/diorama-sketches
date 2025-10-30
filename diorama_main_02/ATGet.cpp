#include "ATGet.h"

ATGet::ATGet(ATSend &at, Stream &serialPort, const String &host, int port)
    : _at(at), _serial(serialPort), _host(host), _port(port) {}

// ---------- Blocking Mode ----------
String ATGet::getRequest(const String &path, bool printDebug) {
    beginAsync(path, printDebug);
    while (isBusy()) updateAsync();
    return getResponse();
}

// ---------- Asynchronous Mode ----------
void ATGet::beginAsync(const String &path, bool printDebug) {
    if (_state != IDLE) return;
    _pendingPath = path;
    _printDebug = printDebug;
    _response = "";
    _state = CONNECTING;
    _lastAction = millis();
    if (_printDebug) Serial.println(F("🌐 [ATGet] beginAsync"));
    String startCmd = "AT+CIPSTART=\"SSL\",\"" + _host + "\"," + String(_port);
    _serial.println(startCmd);
}

void ATGet::updateAsync() {
    switch (_state) {
        case IDLE: return;

        case CONNECTING: {
            if (_serial.available()) {
                _response += _serial.readString();
                if (_response.indexOf("CONNECT") != -1) {
                    if (_printDebug) Serial.println(F("✅ SSL CONNECTED"));
                    String httpRequest = "GET " + _pendingPath + " HTTP/1.1\r\nHost: " + _host + "\r\nConnection: close\r\n\r\n";
                    _sendBuffer = httpRequest;
                    _serial.println("AT+CIPSEND=" + String(httpRequest.length()));
                    _response = "";
                    _state = WAIT_PROMPT;
                    _lastAction = millis();
                }
            }
            if (millis() - _lastAction > 8000) { _state = ERROR; }
            break;
        }

        case WAIT_PROMPT: {
            if (_serial.available()) {
                _response += _serial.readString();
                if (_response.indexOf('>') != -1) {
                    _serial.print(_sendBuffer);
                    _response = "";
                    _state = READING;
                    _lastAction = millis();
                }
            }
            if (millis() - _lastAction > 8000) _state = ERROR;
            break;
        }

        case READING: {
            while (_serial.available()) {
                char c = _serial.read();
                _response += c;
                _lastAction = millis();
            }
            if (millis() - _lastAction > 1000) {
                String body = _extractRawBody(_response);
                _response = body;
                _serial.println("AT+CIPCLOSE");
                _state = DONE;
            }
            break;
        }

        case DONE:
        case ERROR:
            break;
    }
}

bool ATGet::isBusy() const { return _state != IDLE && _state != DONE && _state != ERROR; }
bool ATGet::hasResponse() const { return _state == DONE; }

String ATGet::getResponse() { return _response; }
void ATGet::clearResponse() { _state = IDLE; _response = ""; }

String ATGet::_readFullResponse(unsigned long idleTimeout) {
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

String ATGet::_extractRawBody(const String &raw) {
    int start = raw.indexOf("\r\n\r\n");
    if (start == -1) return raw;
    String body = raw.substring(start + 4);
    int closed = body.indexOf("CLOSED");
    if (closed != -1) body = body.substring(0, closed);
    body.trim();
    return body;
}
