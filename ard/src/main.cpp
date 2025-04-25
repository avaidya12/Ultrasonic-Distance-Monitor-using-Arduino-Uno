#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Try 0x3F if 0x27 doesn't work

// Ultrasonic sensor pins
const int trigPin = 8;
const int echoPin = 9;

// Buzzer pin
const int buzzerPin = 7;

// Timing
unsigned long lastUpdateTime = 0;
const unsigned long lcdCheckInterval = 5000;  // Check every 5s for "Bye" logic

unsigned long buzzerOnTime = 0;
const unsigned long buzzerDuration = 1000;    // Buzzer ON for 1s

unsigned long lastByeTime = 0;                // When LCD was turned off
const unsigned long lcdWakeDelay = 3000;      // 3 seconds

// State
bool lcdIsOn = true;
bool lcdWaitingToTurnOn = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();

  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  // Turn OFF LCD if object is too far (check every 5 seconds)
  if (currentTime - lastUpdateTime >= lcdCheckInterval) {
    lastUpdateTime = currentTime;

    if (distance > 400 && lcdIsOn) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Byeeee!!!!!!");
      delay(1000);
      lcd.clear();
      lcd.noBacklight();
      lcdIsOn = false;
      lcdWaitingToTurnOn = true;
      lastByeTime = millis();
    }
  }

  // Turn ON LCD after 3 secs if object is back within 4m
  if (!lcdIsOn && lcdWaitingToTurnOn && distance <= 400 && millis() - lastByeTime >= lcdWakeDelay) {
    lcd.backlight();
    lcd.clear();
    lcdIsOn = true;
    lcdWaitingToTurnOn = false;
  }

  // Live display update
  if (lcdIsOn) {
    lcd.setCursor(0, 0);
    lcd.print("Time:");
    unsigned long seconds = currentTime / 1000;
    int hh = seconds / 3600;
    int mm = (seconds % 3600) / 60;
    int ss = seconds % 60;

    if (hh < 10) lcd.print("0");
    lcd.print(hh); lcd.print(":");
    if (mm < 10) lcd.print("0");
    lcd.print(mm); lcd.print(":");
    if (ss < 10) lcd.print("0");
    lcd.print(ss);
    lcd.print(" ");

    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distance, 1);
    lcd.print(" cm   ");
  }

  // Buzzer control
  if (distance > 0 && distance <= 50) {
    if (buzzerOnTime == 0) {
      buzzerOnTime = currentTime;
      digitalWrite(buzzerPin, HIGH);
    }
  }

  if (buzzerOnTime > 0 && currentTime - buzzerOnTime >= buzzerDuration) {
    digitalWrite(buzzerPin, LOW);
    buzzerOnTime = 0;
  }

  if (distance > 50) {
    digitalWrite(buzzerPin, LOW);
    buzzerOnTime = 0;
  }

  // Optional serial debug
  Serial.print("Time ");
  Serial.print(currentTime / 1000); Serial.print("s - ");
  Serial.print("Distance: ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  delay(100); // Smooth display updates
}
