#include "ATGet.h"

ATGet::ATGet(ATUtils &atUtils, ATWifi &wifiModule) : at(atUtils), wifi(wifiModule) {}

// Start TCP/SSL connection with proper blocking
bool ATGet::startConnection(const String &host, int port) {
    if (!wifi.isConnected()) {
        Serial.println("⚠️ WiFi not connected, cannot start TCP/SSL");
        return false;
    }

    // Close previous connection
    Serial.println("📤 AT → AT+CIPCLOSE");
    String resp = at.sendCommand("AT+CIPCLOSE", 2000);
    while (resp.indexOf("OK") == -1 && resp.indexOf("ERROR") == -1) {
        Serial.println("⏳ Waiting for previous connection to close...");
        resp = at.sendCommand("AT+CIPCLOSE", 2000);
    }
    Serial.println("✅ Previous connection closed");

    // Start TCP/SSL connection
    String cmd = "AT+CIPSTART=\"SSL\",\"" + host + "\"," + String(port);
    Serial.println("📤 AT → " + cmd);
    resp = at.sendCommand(cmd, 5000);
    while (resp.indexOf("CONNECT") == -1 && resp.indexOf("ALREADY CONNECT") == -1 && resp.indexOf("ERROR") == -1) {
        Serial.println("⏳ Waiting for TCP/SSL connection...");
        resp = at.sendCommand(cmd, 5000);
    }

    if (resp.indexOf("CONNECT") >= 0 || resp.indexOf("ALREADY CONNECT") >= 0) {
        Serial.println("✅ Connection established");
        return true;
    } else {
        Serial.println("❌ Failed to connect");
        return false;
    }
}

// Send HTTP GET request and return the initial response string
String ATGet::sendHttpRequest(const String &host, const String &path) {
    String http = "GET " + path + " HTTP/1.1\r\n";
    http += "Host: " + host + "\r\n";
    http += "Connection: close\r\n\r\n";

    String cmd = "AT+CIPSEND=" + String(http.length());
    Serial.println("📤 AT → " + cmd);
    String resp = at.sendCommand(cmd, 5000);

    // Wait until ESP is ready for HTTP request
    while (resp.indexOf(">") == -1) {
        Serial.println("⏳ Waiting for ESP to be ready for HTTP request...");
        resp = at.sendCommand(cmd, 5000);
    }

    // Send the HTTP request
    Serial.println("📤 Sending HTTP request...");
    resp = at.sendCommand(http, 10000);  // send request
    Serial.println("📥 HTTP request sent, initial response: " + resp);

    return resp; // return the raw response
}


String ATGet::readResponse(unsigned long timeoutMs) {
  String fullResponse = "";
  unsigned long start = millis();
  unsigned long lastData = millis();

  Serial.println("⏳ Reading response...");

  // Accumulate data until connection closed or timeout
  while (millis() - start < timeoutMs) {
      while (at.stream.available()) {
          char c = at.stream.read();
          fullResponse += c;
          Serial.print(c);  // debug print
          lastData = millis();  // reset inactivity timer
      }

      // Stop if connection closed
      if (fullResponse.indexOf("CLOSED") >= 0) break;

      // Stop if no new data for 500ms but we already have some data
      if ((millis() - lastData > 500) && fullResponse.length() > 0) break;

      delay(1);
  }

  return fullResponse;
}


String ATGet::get(const String &host, const String &path, int port, unsigned long timeoutMs) {
    if (!startConnection(host, port)) return "";

    // Send the HTTP request
    sendHttpRequest(host, path);

    // Block until full response is received or timeout
    String body = "";
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        body = readResponse(timeoutMs);  // keep reading
        if (body.length() > 0) break;    // exit once we have something
        delay(1);                        // avoid busy loop
    }

    return body;
}

