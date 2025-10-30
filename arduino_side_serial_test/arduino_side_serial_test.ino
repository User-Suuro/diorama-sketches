void setup() {
  Serial.begin(9600);      // Communication with PC
  Serial3.begin(9600);     // Communication with ESP32
  Serial.println("Serial bridge ready");
  delay(2000);
  Serial3.println("AT");   // Test AT command
}

void loop() {
  while (Serial3.available()) Serial.write(Serial3.read());
  while (Serial.available()) Serial3.write(Serial.read());
}
