#include "PCA9685.h"
#include <Wire.h>

PCA9685 pwmShield;

const int buttonPin = 4;
const int onboardLed = 13;
const uint16_t fullLedBrightness = 4095;
const int redLedPin = 1;
const int blueLedPin = 16;
const int DEFUAULT_DEBOUNCE_MILLI = 50;

struct LEDState
{
  int ledPin;
  float ledBrightnessRatio = 1;
  uint16_t maxLedBrightness = fullLedBrightness;
  PCA9685 pwd;
};

struct ToggleButtonState
{
  bool isToggledOn;
  int buttonPin;
  int currentState;
  int lastRead;
  int debounceDelay = DEFUAULT_DEBOUNCE_MILLI;
  long debounceTime;
};

struct ToggleButtonState toggleButton;
struct LEDState redLed;
struct LEDState blueLed;

void setup()
{
  toggleButton.buttonPin = buttonPin;

  pinMode(toggleButton.buttonPin, INPUT);
  pinMode(onboardLed, OUTPUT);
  initializePWMBoard();

  redLed.ledPin = redLedPin;
  redLed.pwd = pwmShield;

  blueLed.ledPin = blueLedPin;
  blueLed.pwd = pwmShield;

  turnLedOn(blueLed);
  turnLedOn(redLed);
  delay(1000);
  blueLed.ledBrightnessRatio =
      redLed.ledBrightnessRatio = .25;
  turnLedOn(blueLed);
  turnLedOn(redLed);
  delay(2000);
  turnLedOff(blueLed);
  turnLedOff(redLed);
}

void loop()
{
  struct ToggleButtonState toggleButtonState = getToggleButtonState();

  if (toggleButtonState.isToggledOn)
  {
    turnLedOff(redLed);
    turnLedOn(blueLed);
  }
  else
  {
    turnLedOn(redLed);
    turnLedOff(blueLed);
  }
}

struct ToggleButtonState getToggleButtonState()
{
  int buttonState = digitalRead(toggleButton.buttonPin);

  if (buttonState != toggleButton.lastRead)
  {
    toggleButton.debounceTime = millis();
  }

  int debounceReadTime = (millis() - toggleButton.debounceTime);
  if ( debounceReadTime > toggleButton.debounceDelay)
  {
    if (toggleButton.currentState != buttonState)
    {
      toggleButton.currentState = buttonState;
      toggleButton.isToggledOn = 
        buttonState == HIGH ? 
          !toggleButton.isToggledOn : toggleButton.isToggledOn;
    }
    Serial.println(toggleButton.isToggledOn);
  }

  toggleButton.lastRead = buttonState;
  return toggleButton;
}

void turnLedOn(struct LEDState led)
{
  uint16_t offCycle = led.maxLedBrightness - ((1 - led.ledBrightnessRatio) * led.maxLedBrightness);
  led.pwd.setPwm(led.ledPin, offCycle > 0 ? 0 : led.maxLedBrightness, offCycle);
}

void turnLedOff(struct LEDState led)
{
  led.pwd.setPwm(led.ledPin, 0, 0);
}

void initializePWMBoard()
{
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Serial.begin(9600);
  pwmShield.init(0x7f);
  // Set freq to 100Hz, range from 24Hz~1526hz
  pwmShield.setFrequency(1024);

  for (int i = 1; i < 17; i++)
  {
    pwmShield.setPwm(i, 0, 0);
    delay(100);
  }
}