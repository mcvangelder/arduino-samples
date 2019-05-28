#include "PCA9685.h"
#include <Wire.h>

const byte buttonPin = 4;
const byte DEFUAULT_DEBOUNCE_MILLI = 50;

const byte servoPin = 1;
const byte servoRotationMax = 180;
const byte servoRotationMin = 0;

const byte rotEncoderAnalogPin = 3;
const int rotEncoderValueMax = 1023;
const byte rotEncoderValueMin = 0;

struct ServoState
{
  byte pin;
  ServoDriver servo;
  byte position;
  byte maxAngle = servoRotationMax;
  byte minAngle = servoRotationMin;
};

struct ToggleButtonState
{
  bool isToggledOn;
  byte buttonPin;
  byte currentState;
  byte lastRead;
  byte debounceDelay = DEFUAULT_DEBOUNCE_MILLI;
  long debounceTime;
};

struct RotationEncoderState
{
  byte pin;
  int encodedAngle;
  int maxAngle = rotEncoderValueMax;
  byte minAngle = rotEncoderValueMin;
};

struct ToggleButtonState toggleButton;
struct ServoState servo;
struct RotationEncoderState rotEncoder;

void setup()
{
  toggleButton.buttonPin = buttonPin;

  pinMode(toggleButton.buttonPin, INPUT);
  initializeServo(&servo);
  initializeRotationEncoder(&rotEncoder);
}

void loop()
{
  struct ToggleButtonState toggleButtonState = getToggleButtonState();
  struct RotationEncoderState inputAngle = getRotationalEncoderState();

  if (toggleButtonState.isToggledOn)
  {
    int requestedServoAngle = convertToServoAngle(rotEncoder, servo);
    if (servo.position != requestedServoAngle)
    {
      servo.position = requestedServoAngle;
      servo.servo.setAngle(servo.pin, requestedServoAngle);
    }
  }
  else if (!toggleButtonState.isToggledOn && servo.position != 0)
  {
    servo.position = 0;
    servo.servo.setAngle(servo.pin, 0);
  }
}

struct ToggleButtonState getToggleButtonState()
{
  byte buttonState = digitalRead(toggleButton.buttonPin);

  if (buttonState != toggleButton.lastRead)
  {
    toggleButton.debounceTime = millis();
  }

  int debounceReadTime = (millis() - toggleButton.debounceTime);
  if (debounceReadTime > toggleButton.debounceDelay)
  {
    if (toggleButton.currentState != buttonState)
    {
      toggleButton.currentState = buttonState;
      toggleButton.isToggledOn =
          buttonState == HIGH ? !toggleButton.isToggledOn : toggleButton.isToggledOn;
    }
  }

  toggleButton.lastRead = buttonState;
  return toggleButton;
}

struct RotationEncoderState getRotationalEncoderState()
{
  float inputAngle = analogRead(rotEncoder.pin);
  rotEncoder.encodedAngle = inputAngle;
  return rotEncoder;
}

byte convertToServoAngle(RotationEncoderState rotEncoder, ServoState servo)
{
  return map(rotEncoder.encodedAngle, rotEncoder.minAngle, rotEncoder.maxAngle, servo.minAngle, servo.maxAngle);
}

void initializeServo(struct ServoState *servoState)
{
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Serial.begin(9600);
  servoState->servo.init(0x7f);
  servoState->pin = servoPin;

  servoState->servo.setAngle(servoState->pin, servoRotationMax);
  delay(1000);
  servoState->servo.setAngle(servoState->pin, 0);
}

void initializeRotationEncoder(struct RotationEncoderState *rotEncoder)
{
  rotEncoder->pin = rotEncoderAnalogPin;
}