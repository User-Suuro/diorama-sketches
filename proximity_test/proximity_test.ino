#define TRIG_PIN 50   // Trigger pin
#define ECHO_PIN 51   // Echo pin

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("📏 Ultrasonic Sensor Test Started");
}

void loop() {
  long duration;
  int distance;

  // Send a short 10µs pulse to trigger the sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the time for echo to return
  duration = pulseIn(ECHO_PIN, HIGH);

  // Convert the time into distance (cm)
  distance = duration * 0.034 / 2;

  // Print the result
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500);
}
