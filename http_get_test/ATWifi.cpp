#include "ATWifi.h"

ATWifi::ATWifi(ATUtils &atUtils) : at(atUtils) {}

// Check WiFi connection
bool ATWifi::isConnected() {
  String resp = at.sendCommand("AT+CWJAP?", 3000, 100);
  resp.toUpperCase();

  if (resp.indexOf("CWJAP:") >= 0) return true;
  if (resp.indexOf("CONNECTED") >= 0) return true;
  return false;
}

// Connect to WiFi (skips if already connected)
bool ATWifi::connect(const String &ssid, const String &password, unsigned long timeoutMs) {
  // 1️⃣ Check if already connected
  if (isConnected()) {
    return true; // already connected, no need to reconnect
  }

  // 2️⃣ Set ESP to station mode
  at.sendCommand("AT+CWMODE=1", 2000, 100);

  // 3️⃣ Join access point
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  String resp = at.sendCommand(cmd, timeoutMs, 200);

  resp.toUpperCase();
  return resp.indexOf("OK") >= 0;
}
