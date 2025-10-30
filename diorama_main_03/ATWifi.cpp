#include "ATWifi.h"

ATWifi::ATWifi(ATUtils &util, const String &ssid, const String &password)
  : _util(util), _ssid(ssid), _password(password) {}

bool ATWifi::connect(bool printDebug) {
  String response;

  // Step 1: Set Wi-Fi mode to Station
  if (printDebug) Serial.println(F("📶 Setting WiFi mode (Station)..."));
  if (!_util.sendCommand(F("AT+CWMODE=1"), response)) {
    if (printDebug) Serial.println(F("❌ Failed to set WiFi mode"));
    return false;
  }

  // Step 2: Connect to Wi-Fi network
  if (printDebug) {
    Serial.print(F("🌐 Connecting to WiFi: "));
    Serial.println(_ssid);
  }

  String cmd = "AT+CWJAP=\"" + _ssid + "\",\"" + _password + "\"";
  if (!_util.sendCommand(cmd, response, "WIFI CONNECTED")) {
    if (printDebug) {
      Serial.println(F("❌ Failed to connect to WiFi"));
      Serial.println(response);
    }
    return false;
  }




  return true;
}

bool ATWifi::disconnect(bool printDebug) {
  String response;

  if (printDebug) Serial.println(F("📴 Disconnecting from WiFi..."));
  if (!_util.sendCommand(F("AT+CWQAP"), response, "OK")) {
    if (printDebug) {
      Serial.println(F("❌ Failed to disconnect"));
      Serial.println(response);
    }
    return false;
  }

  if (printDebug) Serial.println(F("✅ Successfully disconnected from WiFi"));
  return true;
}

bool ATWifi::isConnected(bool printDebug) {
  String response;
  if (!_util.sendCommand(F("AT+CWJAP?"), response, "OK")) {
    if (printDebug) Serial.println(F("⚠️ Failed to check WiFi status"));
    return false;
  }

  bool connected = response.indexOf("No AP") == -1;
  if (printDebug) {
    Serial.println(connected ? F("✅ WiFi still connected") : F("❌ WiFi disconnected"));
  }
  return connected;
}
