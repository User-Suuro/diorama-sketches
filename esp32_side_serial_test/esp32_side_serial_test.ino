#include <WiFi.h>
#include "ATWiFi.h"
#include "ATPost.h"

HardwareSerial ArdSerial(2); // UART2 (RX=16, TX=17)
ATWiFi atwifi(ArdSerial);
ATPost atpost(ArdSerial, atwifi);

// === HTTPS Configuration ===
const char* ENDPOINT_URL = "https://your-vercel-endpoint.vercel.app/api/data";

// Root CA certificate (replace with your actual certificate)
const char* ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDdzCCAl+gAwIBAgIJAK+2Z0P4PvPaMA0GCSqGSIb3DQEBCwUAMFoxCzAJBgNV
BAYTAkFVMQswCQYDVQQIDAJXQTERMA8GA1UEBwwIS2FtcGVyYTEQMA4GA1UECgwH
RXhhbXBsZTEQMA4GA1UEAwwHZXhhbXBsZTCCASIwDQYJKoZIhvcNAQEBBQADggEP
ADCCAQoCggEBALANZ8Ry9vBZwOAvbKGE6xdNoP4FxvYrt75Noh0hCHz6AqX9O6AQ
kZgU2M6Is/47sJb1cV3AQrA+zAqQULwX9m1c5slG/5kU74+YgiWztEYkAqA4v2fl
K6I9pRdoEl2m8i9B1Q4zH22MtkDg5J8OBrb4dM0SCj8c9eQjGkzF/kkT9PtK9Juf
1e8q3PyZVaxerFqeqx1H7F0+0nslftGgkAz13mO6bgWRf3lSxhPztGiqiR8j9+3R
0ChdtQLlkmXSKiS3rCMXcTE0Z6qMErDPZDNfAiyHuD3OdNY0g+ynK4csTqMIH0RG
7uKUedZ3duBiy46sQzI5z0PKFgkbmYAzMC0CAwEAAaNQME4wHQYDVR0OBBYEFJbV
St7Yx/wZHyRwbE9vE1nIgrB7MB8GA1UdIwQYMBaAFJbVSt7Yx/wZHyRwbE9vE1nI
grB7MAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAE3WZ8YFQUsqR6MZ
ocx/9Z4JjZHP8+8XKDeHuh4eKK7hVfTP2vvC+ZoDuvQGHgqT1kQHcAFMZc5sclp7
cVgbD4E8pslV9cbUuUOQir6JmW2MCFwDp8ShqQcxrjvBvEYEnr1V2d6JKF6dL3AZ
dfKhEOEQYTH9dN5MnQ0eRwYfYI5U5E0H7vW/pc7EZa/KayUXAlTCTYMCnmmFWfqx
hvNQ8qZ5cPpQ6z1KozB5x9S6McQ0eqeA2L8Xg0BtJKv6ZbE2RaFPwjfBYEmMVi8Z
VMDzFeYc6yVN1q6x4Ao1avVRxDJJUb+V1sywK9ZlOxCl8H1b6HjVmnz5ISVWQx7W
pefvn7g=
-----END CERTIFICATE-----
)EOF";

void setup() {
  Serial.begin(115200);
  ArdSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("ESP32 Ready");
  ArdSerial.println("ESP32 Ready");

  atwifi.begin();
  atpost.begin(ROOT_CA, ENDPOINT_URL);
}

void loop() {
  if (ArdSerial.available()) {
    String cmd = ArdSerial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      handleCommand(cmd);
    }
  }
}

// Reroute commands to functions

void handleCommand(String cmd) {
  Serial.println("Processing Command: " + cmd);

  if (cmd.startsWith("+AT=CONNECT_WIFI") || 
      cmd.startsWith("+AT=CHECK_WIFI") || 
      cmd.startsWith("+AT=DISCONNECT_WIFI")) {
    handleWifi(cmd);
  } 

  else if (cmd.startsWith("+AT=POST")) {
    handlePost(cmd);
  } 
 
  else if (cmd.startsWith("+AT=CHECK_HTTP")) {
    atpost.checkConnection();
  } 

  else {
    ArdSerial.println("ERROR: Unknown Command");
  }
}

// Command Handlers

void handleWifi(String cmd) {
  if (cmd.startsWith("+AT=CONNECT_WIFI")) {
    int firstQuote = cmd.indexOf('"');
    int secondQuote = cmd.indexOf('"', firstQuote + 1);
    int thirdQuote = cmd.indexOf('"', secondQuote + 1);
    int fourthQuote = cmd.indexOf('"', thirdQuote + 1);

    if (firstQuote != -1 && secondQuote != -1 && thirdQuote != -1 && fourthQuote != -1) {
      String ssid = cmd.substring(firstQuote + 1, secondQuote);
      String password = cmd.substring(thirdQuote + 1, fourthQuote);
      atwifi.connectToWiFi(ssid, password);
    } else {
      ArdSerial.println("ERROR: Invalid parameters");
    }
  } 
  else if (cmd.startsWith("+AT=CHECK_WIFI")) {
    atwifi.checkWiFiStatus();
  } 
  else if (cmd.startsWith("+AT=DISCONNECT_WIFI")) {
    atwifi.disconnectWiFi();
  }
}


void handlePost(String cmd) {
  int quoteStart = cmd.indexOf('"');
  int quoteEnd = cmd.lastIndexOf('"');

  if (quoteStart != -1 && quoteEnd != -1 && quoteEnd > quoteStart) {
    String payload = cmd.substring(quoteStart + 1, quoteEnd);
    payload.trim();
    if (payload.length() > 0) {
      Serial.println("Sending POST payload: " + payload);
      atpost.sendPost(payload);
    } else {
      ArdSerial.println("ERROR: Empty POST payload");
    }
  } else {
    ArdSerial.println("ERROR: Invalid POST format");
  }
}


