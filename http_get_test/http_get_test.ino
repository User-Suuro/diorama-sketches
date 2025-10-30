#include <SoftwareSerial.h>
#include "ATUtils.h"
#include "ATWifi.h"
#include "ATGet.h"

SoftwareSerial espSerial(2, 3); // RX, TX
ATUtils at(espSerial);
ATWifi wifi(at);
ATGet http(at, wifi);

const char* ssid = "X8b";
const char* password = "12345678";

const char* host = "diorama-endpoint.vercel.app";
const int port = 443;

const char* check_connection_endpoint = "/api/arduino/check-connection";

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  delay(2000);
  Serial.println("Starting AT test...");

  String res = at.sendCommand("AT", 3000);
  Serial.println("Response: " + res);

  if (wifi.connect(ssid, password, 10000)) {
    Serial.println("✅ WiFi is connected!");
  } else {
    Serial.println("❌ Failed to connect.");
  }

  String body = http.get(host, check_connection_endpoint, port, 15000);
  
  if (body.length() > 0) {
    Serial.println("HTTP GET response:");
    Serial.println(body);

    body = extractJson(body);

    Serial.println("JSON: ");
    Serial.println(body);

    Serial.println("Status: ");
    Serial.println(extractStatus(body));
   

  } else {
      Serial.println("Failed to get HTTP response");
  }

  Serial.println("FINISHED");
}

void loop() {}

// Function to extract JSON from full ATGet response
String extractJson(const String &response) {
  int jsonStart = response.indexOf('{'); // first {
  int jsonEnd = response.lastIndexOf('}'); // last }
  
  if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart) {
    return response.substring(jsonStart, jsonEnd + 1);
  }
  
  return ""; // return empty string if no JSON found
}

String extractStatus(String response) {
    // Find the first and last braces to locate the JSON
    int jsonStart = response.indexOf('{');
    int jsonEnd = response.lastIndexOf('}');
    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart) return "";

    // Extract JSON substring
    String json = response.substring(jsonStart, jsonEnd + 1);

    // Find the "status" key
    int keyIndex = json.indexOf("\"status\"");
    if (keyIndex == -1) return "";

    // Find colon and quotes surrounding the value
    int colonIndex = json.indexOf(':', keyIndex);
    int quoteStart = json.indexOf('"', colonIndex);
    int quoteEnd = json.indexOf('"', quoteStart + 1);
    if (colonIndex == -1 || quoteStart == -1 || quoteEnd == -1) return "";

    // Extract the status value
    String status = json.substring(quoteStart + 1, quoteEnd);
    status.trim();
    return status;
}


