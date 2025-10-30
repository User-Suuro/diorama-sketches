#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16x2 LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();           // Initialize the LCD
  lcd.backlight();      // Turn on the backlight
  lcd.setCursor(0, 0);  // Set cursor to first column, first row
  lcd.print("Hello, World!");
}

void loop() {
  static int counter = 0;
  lcd.setCursor(0, 1);  // Move to the second line
  lcd.print("Count: ");
  lcd.print(counter++);
  delay(1000);
}
