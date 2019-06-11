#include "statemachine.h"

StateMachine machine;
StateData state1 = StateData(1);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    StateData state2 = StateData(2);

    StateData *state1Transitions[] = {&state2};
    StateData *state2Transitions[] = {&state1};

    state1.setAllowedTransitions(state1Transitions, 1);
    state2.setAllowedTransitions(state2Transitions, 1);

    StateData *allStates[] = {&state1, &state2};
    machine = StateMachine(allStates, arrayLength(allStates), state1);
    machine.setOnTransitionCallback(onTransition);
    delay(1000);
    pinMode(LED_BUILTIN, LOW);
    delay(500);
    pinMode(LED_BUILTIN, HIGH);
}

long transitionDelay = 1000;
long lastTransition = 0;

void loop()
{
    if (lastTransition == 0 || lastTransition + transitionDelay < millis())
    {
        lastTransition = millis();

        int currentValue = machine.getCurrentStateValue();
        int nextValue = 0;

        switch (currentValue)
        {
        case 1:
            nextValue = 2;
            break;
        case 2:
            nextValue = 1;
            break;
        }

        machine.transitionTo(nextValue);
    }
}

void onTransition(StateData *oldState, StateData *newState)
{
    auto currentValue = newState->getValue();

    switch (currentValue)
    {
    case 1:
        digitalWrite(LED_BUILTIN, LOW);
        break;
    case 2:
        digitalWrite(LED_BUILTIN, HIGH);
        break;
    default:
        break;
    }
}