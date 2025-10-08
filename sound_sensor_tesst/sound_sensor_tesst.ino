int sensor = A0;      // KY-038 analog output
int led = 13;         // LED pin
bool is_on = false;

const int threshold = 1000;     // Base loudness threshold
const int quietThreshold = 300; // What counts as "quiet" after a clap
const int maxClapDuration = 200; // ms - how long a clap spike can last
unsigned long lastClapTime = 0;

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  Serial.begin(9600);
}

void loop() {
  int soundLevel = analogRead(sensor);
  unsigned long currentTime = millis();

  // Detect sharp, sudden sound spike (clap)
  if (soundLevel > threshold && (currentTime - lastClapTime > 500)) {
    unsigned long startTime = millis();

    // Wait a short moment to confirm it's a quick spike, not long noise
    while (analogRead(sensor) > quietThreshold) {
      if (millis() - startTime > maxClapDuration) {
        // Too long -> likely noise
        return;
      }
    }

    // Valid clap detected
    lastClapTime = millis();
    is_on = !is_on;
    digitalWrite(led, is_on ? HIGH : LOW);
    Serial.println("Clap detected!");
  }
}
