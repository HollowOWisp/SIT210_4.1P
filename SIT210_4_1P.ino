#include <Wire.h>
#include <BH1750.h>


const int LED1_PIN = 10;
const int LED2_PIN = 12;
const int PIR_PIN  = 2;
const int SW_PIN   = 3;


// Light threshold
// Lower lux = darker
// Adjust this after testing in your room
const float DARK_THRESHOLD = 30.0;

BH1750 lightMeter;

volatile bool pirTriggered = false;
volatile bool switchTriggered = false;

bool lightsOn = false;
bool manualMode = false;   // true when slider switch is used
unsigned long lastLuxRead = 0;
float currentLux = 0.0;

void pirISR() {
  pirTriggered = true;
}

void switchISR() {
  switchTriggered = true;
}

void turnLightsOn() {
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  lightsOn = true;
}

void turnLightsOff() {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  lightsOn = false;
}

void printLightStatus() {
  Serial.print("Current light level: ");
  Serial.print(currentLux);
  Serial.println(" lux");
}

bool isDark() {
  return currentLux < DARK_THRESHOLD;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  // Slider switch using internal pull-up
  pinMode(SW_PIN, INPUT_PULLUP);

  turnLightsOff();

  Wire.begin();
  lightMeter.begin();


  currentLux = lightMeter.readLightLevel();


  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirISR, RISING);
  attachInterrupt(digitalPinToInterrupt(SW_PIN), switchISR, CHANGE);

  Serial.println("System started.");
  Serial.println("Interrupt-based lighting system is ready.");
  printLightStatus();
}

void loop() {
  // Periodically update light level
  if (millis() - lastLuxRead >= 1000) {
    lastLuxRead = millis();
    currentLux = lightMeter.readLightLevel();
  }

  if (pirTriggered) {
    noInterrupts();
    pirTriggered = false;
    interrupts();

    Serial.println("Motion detected interrupt received.");
    printLightStatus();

    if (isDark()) {
      turnLightsOn();
      manualMode = false;
      Serial.println("It is dark. LED1 and LED2 turned ON automatically.");
    } else {
      Serial.println("Motion detected, but it is bright. Lights remain OFF.");
    }
  }

  if (switchTriggered) {
    noInterrupts();
    switchTriggered = false;
    interrupts();

    // Because of INPUT_PULLUP:
    // LOW = switch ON / active
    // HIGH = switch OFF / inactive
    int switchState = digitalRead(SW_PIN);

    if (switchState == LOW) {
      turnLightsOn();
      manualMode = true;
      Serial.println("Slider switch interrupt received.");
      Serial.println("Manual override ON. LED1 and LED2 turned ON.");
    } else {
      // only turn off if manual mode was controlling it
      if (manualMode) {
        turnLightsOff();
        manualMode = false;
        Serial.println("Slider switch interrupt received.");
        Serial.println("Manual override OFF. LED1 and LED2 turned OFF.");
      }
    }
  }
}