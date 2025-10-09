#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mp3Serial(10, 11); // RX, TX
DFRobotDFPlayerMini mp3;

void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);

  if (!mp3.begin(mp3Serial)) {  // Start communication with the module
    Serial.println("Unable to start DFPlayer Mini!");
    Serial.println("Check wiring and SD card.");
    while (true);
  }

  mp3.volume(30);  // Set volume (0~30)
  mp3.play(1);     // Play "001.mp3"
  Serial.println("Playing track 001.mp3...");
}

void loop() {
  // Nothing else for now
}
