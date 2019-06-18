#include "statemachine.h"

StateMachine machine;

#define SYSTEM_OFF 0b00
#define LED_ON 0b01
#define LED_OFF 0b10

struct LED
{
    int pin;
    int output;
    long lastStateChange;
};

struct BUTTON
{
    int pin;
    long lastReadMillis;
    bool isToggled;
    char lastRead = LOW;
    char currentState = LOW;
    long debounceDelay = 50;
};

LED led;
BUTTON button;

void setup()
{
    Serial.begin(9600);

    setupLED(&led);
    setUpButton(&button);
    setUpStateMachine();

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("set up complete");
}

long transitionDelay = 1000;
int nextValue;
int previousValue;
bool isSystemOn;

void loop()
{
    auto currentState = machine.getCurrentStateValue();
    auto isToggled = isButtonToggled(&button);
    if (isToggled && currentState == SYSTEM_OFF)
    {
        nextValue = LED_ON;
    }
    else if(!isToggled)
    {
        nextValue = SYSTEM_OFF;
    }

    if (previousValue != nextValue)
    {
        machine.transitionTo(nextValue);
    }

    if (isSystemOn)
    {
        if (led.lastStateChange + transitionDelay < millis())
        {
            execute(led);
        }
    }
    else
    {
        off(led);
    }
}

void setUpStateMachine()
{
    Serial.println("Setting up machine");
    StateData systemOffState = StateData(SYSTEM_OFF);
    StateData ledOnState = StateData(LED_ON);
    StateData ledOffState = StateData(LED_OFF);

    StateData *offStateTransitions[] = {&ledOnState};
    systemOffState.setAllowedTransitions(offStateTransitions, 1);

    StateData *ledOnStateTransitions[] = {&ledOffState, &systemOffState};
    ledOnState.setAllowedTransitions(ledOnStateTransitions, 2);

    StateData *ledStateOffTransitions[] = {&ledOnState, &systemOffState};
    ledOffState.setAllowedTransitions(ledStateOffTransitions, 2);

    StateData *states[] = {&systemOffState, &ledOnState, &ledOffState};

    nextValue = SYSTEM_OFF;
    previousValue = LED_OFF;
    isSystemOn = false;

    machine = StateMachine(states, 3, systemOffState);
    machine.setOnTransitionCallback(onStateChange);
}

void onStateChange(StateData *oldState, StateData *newState)
{
    auto newValue = newState->getValue();
    previousValue = oldState->getValue();
    isSystemOn = newValue != SYSTEM_OFF;
    led.output = LOW;
    if (isSystemOn)
    {
        led.output = newValue == LED_OFF ? LOW : HIGH;
        led.lastStateChange = millis();
    }
}

void execute(LED led)
{
    digitalWrite(led.pin, led.output);
    nextValue = led.output == LOW ? LED_ON : LED_OFF;
}

void off(LED led)
{
    digitalWrite(led.pin, LOW);
}

void setupLED(LED *led)
{
    led->pin = LED_BUILTIN;
    led->output = LOW;
    pinMode(led->pin, OUTPUT);
    digitalWrite(led->pin, led->output);
}

void setUpButton(BUTTON *button)
{
    button->pin = 4;
    button->isToggled = false;
    pinMode(button->pin, INPUT);
}

bool isButtonToggled(BUTTON *button)
{
    auto read = digitalRead(button->pin);
    if (read != button->lastRead)
    {
        button->lastReadMillis = millis();
        button->lastRead = read;
    }

    if (button->lastReadMillis + button->debounceDelay < millis())
    {
        if (button->currentState != read)
        {
            button->currentState = read;
            // When the button is HIGH (pressed) preserve the previous value, when the
            // the button is LOW (released) then flip the toggle
            if (read == LOW)
            {
                button->isToggled = !button->isToggled;
            }
        }
    }
    return button->isToggled;
}