// Touch sensor pins (5 sensors)

#define relay1 4
#define relay2 5
#define relay3 6
#define relay4 7

#define touch1 8
#define touch2 9
#define touch3 10
#define touch4 11

// Relay states
bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

void setup() {
  Serial.begin(9600);

  // Touch sensors
  pinMode(touch1, INPUT);
  pinMode(touch2, INPUT);
  pinMode(touch3, INPUT);
  pinMode(touch4, INPUT);

  // Relays
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // All relays OFF initially (active LOW)
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void loop() {
  // Touch 1 controls Relay 1
  if (digitalRead(touch1) == HIGH) {
    relayState1 = !relayState1;
    digitalWrite(relay1, relayState1 ? LOW : HIGH);
    delay(300);
  }

  // Touch 2 controls Relay 2
  else if (digitalRead(touch2) == HIGH) {
    relayState2 = !relayState2;
    digitalWrite(relay2, relayState2 ? LOW : HIGH);
    delay(300);
  }

  // Touch 3 controls Relay 3
  else if (digitalRead(touch3) == HIGH) {
    relayState3 = !relayState3;
    digitalWrite(relay3, relayState3 ? LOW : HIGH);
    delay(300);
  }

  // Touch 4 controls Relay 4
  else if (digitalRead(touch4) == HIGH) {
    bool allOn = relayState1 && relayState2 && relayState3 && relayState4;

    // Kapag lahat ay ON → patayin lahat
    // Kapag may naka-OFF → buksan lahat
    relayState1 = !allOn;
    relayState2 = !allOn;
    relayState3 = !allOn;
    relayState4 = !allOn;

    digitalWrite(relay1, relayState1 ? LOW : HIGH);
    digitalWrite(relay2, relayState2 ? LOW : HIGH);
    digitalWrite(relay3, relayState3 ? LOW : HIGH);
    digitalWrite(relay4, relayState4 ? LOW : HIGH);

    delay(500); // small delay para maiwasan double trigger
  }

  // Touch 5: Toggle ALL relays ON/OFF

}
