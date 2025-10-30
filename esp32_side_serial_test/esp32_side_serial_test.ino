#include <HardwareSerial.h>

HardwareSerial ArdSerial(2); // Use UART2

void setup() {
  Serial.begin(115200);
  ArdSerial.begin(115200, SERIAL_8N1, 16, 17); // RX, TX
  Serial.println("ESP32 ready to communicate with Arduino...");
}

void loop() {
  if (ArdSerial.available()) {
    // Read JSON from Arduino
    String msg = ArdSerial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      Serial.println("Received from Arduino: " + msg);

      // Prepare JSON reply
      String reply = "Hello from esp32";
      ArdSerial.println(reply);
    }
  }

  delay(2000);
}
