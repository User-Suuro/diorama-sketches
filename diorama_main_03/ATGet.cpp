#include "ATGet.h"

ATGet::ATGet(ATUtils &util, const String &host, int port)
  : _util(util), _host(host), _port(port),
    _state(IDLE), _timeout(8000), _lastAction(0),
    _tempBuffer(nullptr), _bufferSize(0), _dataLength(0) {}

bool ATGet::beginAsync(const String &endpoint) {
  if (_state != IDLE && _state != DONE && _state != ERROR) return false;

  _endpoint = endpoint;
  _response = "";
  _freeBuffer(); // clear any old buffer
  _state = CONNECTING;
  _lastAction = millis();

  Serial.println("🌐 [ATGet] beginAsync");
  return true;
}

bool ATGet::process() {
  switch (_state) {
    case IDLE:
    case DONE:
    case ERROR:
      return true; // Nothing to process

    case CONNECTING: {
      if (_connect()) {
        _state = REQUESTING;
        _lastAction = millis();
      } else if (millis() - _lastAction > _timeout) {
        Serial.println("❌ Timeout CONNECTING");
        _state = ERROR;
      }
      break;
    }

    case REQUESTING: {
      if (_sendRequest()) {
        _state = READING;
        _lastAction = millis();
      } else if (millis() - _lastAction > _timeout) {
        Serial.println("❌ Timeout REQUESTING");
        _state = ERROR;
      }
      break;
    }

    case READING: {
      _readResponse();
      if (_dataLength > 0 &&
          (_response.indexOf("+IPD") != -1 || _response.indexOf("OK") != -1)) {
        _state = DONE;
        Serial.println("✅ Done — response received");
      } else if (millis() - _lastAction > _timeout) {
        Serial.println("❌ Timeout READING");
        _state = ERROR;
      }
      break;
    }

    default:
      break;
  }
  return _state == DONE || _state == ERROR;
}

bool ATGet::_connect() {
  String cmd = "AT+CIPSTART=\"SSL\",\"" + _host + "\"," + String(_port);
  String resp;
  if (_util.sendCommand(cmd, resp, "OK")) {
    Serial.println("🌐 Connected to host");
    return true;
  }
  if (resp.indexOf("busy p") != -1) {
    Serial.println("⚠️ ESP busy, will retry");
  }
  return false;
}


bool ATGet::_sendRequest() {
  // Construct HTTP GET payload
  String httpPayload =
    "GET " + _endpoint + " HTTP/1.1\r\n" +
    "Host: " + _host + "\r\n" +
    "Connection: close\r\n\r\n";

  // Step 1: Prepare for data transmission
  String cmd = "AT+CIPSEND=" + String(httpPayload.length());
  String resp;

  if (!_util.sendCommand(cmd, resp, ">")) {
    Serial.println("❌ Failed to enter CIPSEND mode");
    return false;
  }

  // Step 2: Send HTTP GET
  if (_util.sendCommand(httpPayload, resp, "SEND OK")) {
    Serial.println("🌐 Async GET start...");
    return true;
  }

  Serial.println("❌ Failed to send GET request");
  return false;
}

void ATGet::_readResponse() {
  String chunk;
  _util.sendCommand("", chunk, "");  // Passive read, non-blocking style

  if (chunk.length() > 0) {
    size_t newData = chunk.length();
    size_t newSize = _dataLength + newData + 1;

    // Allocate or grow buffer dynamically
    char *newBuffer = (char*) realloc(_tempBuffer, newSize);
    if (!newBuffer) {
      Serial.println("❌ Memory allocation failed during read");
      _state = ERROR;
      return;
    }

    _tempBuffer = newBuffer;
    memcpy(_tempBuffer + _dataLength, chunk.c_str(), newData);
    _dataLength += newData;
    _tempBuffer[_dataLength] = '\0';
    _bufferSize = newSize;

    _response = String(_tempBuffer);
  }
}

bool ATGet::isFinished() const {
  return _state == DONE || _state == ERROR;
}

String ATGet::getResponse() const {
  return _response;
}

void ATGet::_freeBuffer() {
  if (_tempBuffer) {
    free(_tempBuffer);
    _tempBuffer = nullptr;
  }
  _bufferSize = 0;
  _dataLength = 0;
}
