#define SOUND_PIN 3

void setup() {
  Serial.begin(9600);
  pinMode(SOUND_PIN, INPUT);  // Configure the pin as input
  delay(1000);
  Serial.println("🔊 Sound sensor ready...");
}

void loop() {
  readSoundSensor();
  delay(100);
}

void readSoundSensor() {
  int soundState = digitalRead(SOUND_PIN); // Read digital signal (0 or 1)

  if (soundState == HIGH) {
    Serial.println("💥 Sound detected!");
  } else {
    Serial.println("🤫 No sound detected");
  }
}
