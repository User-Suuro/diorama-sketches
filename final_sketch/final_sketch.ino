#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// === Touch Sensor Pins ===
#define touch1 2
#define touch2 3
#define touch3 4
#define touch4 5
#define touchAll 6  // Pang-lima: Toggle all ON/OFF

// === Relay Pins ===
#define relay1 7
#define relay2 8
#define relay3 9
#define relay4 12

// === Sound Sensor Pin ===
#define soundSensor A0
const int threshold = 1000;
const int quietThreshold = 300;
const int maxClapDuration = 200;
unsigned long lastClapTime = 0;

// === Ultrasonic Pins ===
#define trigPin A1
#define echoPin A2

// === DFPlayer Pins ===
#define mp3Rx 10   // Arduino RX (connect to TX of DFPlayer)
#define mp3Tx 11   // Arduino TX (connect to RX of DFPlayer)

// === LED Indicator (optional) ===
#define ledPin 13

SoftwareSerial mp3Serial(mp3Rx, mp3Tx);
DFRobotDFPlayerMini mp3;

// === States ===
bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;
bool isPlaying = false;

// === Ultrasonic Variables ===
long duration;
int distance;

// === SETUP ===
void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);

  // Touch sensors
  pinMode(touch1, INPUT);
  pinMode(touch2, INPUT);
  pinMode(touch3, INPUT);
  pinMode(touch4, INPUT);
  pinMode(touchAll, INPUT);

  // Relays
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Sound sensor
  pinMode(soundSensor, INPUT);

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // DFPlayer setup
  Serial.println("Initializing DFPlayer Mini...");
  if (!mp3.begin(mp3Serial)) {
    Serial.println("Unable to start DFPlayer Mini!");
    Serial.println("Check wiring and SD card.");
    while (true);
  }
  mp3.volume(25);
  Serial.println("DFPlayer Mini ready!");

  // All relays OFF initially
  updateAllRelays();
}

// === LOOP ===
void loop() {
  handleTouch();
  detectSound();
  detectUltrasonic();
}

// === TOUCH SENSOR HANDLING ===
void handleTouch() {
  if (digitalRead(touch1) == HIGH) {
    relayState1 = !relayState1;
    digitalWrite(relay1, relayState1 ? LOW : HIGH);
    delay(300);
  }

  if (digitalRead(touch2) == HIGH) {
    relayState2 = !relayState2;
    digitalWrite(relay2, relayState2 ? LOW : HIGH);
    delay(300);
  }

  if (digitalRead(touch3) == HIGH) {
    relayState3 = !relayState3;
    digitalWrite(relay3, relayState3 ? LOW : HIGH);
    delay(300);
  }

  if (digitalRead(touch4) == HIGH) {
    relayState4 = !relayState4;
    digitalWrite(relay4, relayState4 ? LOW : HIGH);
    delay(300);
  }

  // Touch 5: Toggle ALL relays ON/OFF
  if (digitalRead(touchAll) == HIGH) {
    bool allOn = relayState1 && relayState2 && relayState3 && relayState4;
    relayState1 = !allOn;
    relayState2 = !allOn;
    relayState3 = !allOn;
    relayState4 = !allOn;
    updateAllRelays();
    delay(500);
  }
}

// === SOUND SENSOR HANDLING ===
void detectSound() {
  int soundLevel = analogRead(soundSensor);
  unsigned long currentTime = millis();

  if (soundLevel > threshold && (currentTime - lastClapTime > 500)) {
    unsigned long startTime = millis();

    while (analogRead(soundSensor) > quietThreshold) {
      if (millis() - startTime > maxClapDuration) return;
    }

    lastClapTime = millis();

    bool allOn = relayState1 && relayState2 && relayState3 && relayState4;
    relayState1 = !allOn;
    relayState2 = !allOn;
    relayState3 = !allOn;
    relayState4 = !allOn;
    updateAllRelays();

    Serial.println("Sound detected! Toggled all relays.");
    delay(500);
  }
}

// === ULTRASONIC + DFPLAYER HANDLING ===
void detectUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= 20 && !isPlaying) {
    Serial.println("Person detected! Playing music...");
    digitalWrite(ledPin, HIGH);
    mp3.play(1);
    isPlaying = true;
  } 
  else if (distance > 20 && isPlaying) {
    Serial.println("No person detected. Stopping music...");
    digitalWrite(ledPin, LOW);
    mp3.stop();
    isPlaying = false;
  }

  delay(500);
}

// === RELAY UPDATE FUNCTION ===
void updateAllRelays() {
  digitalWrite(relay1, relayState1 ? LOW : HIGH);
  digitalWrite(relay2, relayState2 ? LOW : HIGH);
  digitalWrite(relay3, relayState3 ? LOW : HIGH);
  digitalWrite(relay4, relayState4 ? LOW : HIGH);
}
