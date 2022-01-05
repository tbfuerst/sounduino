#include <Arduino.h>
#include <utility.h>
#include <pins.cpp>

void handlePotentiometer(DFPlayerMini_Fast mp3player, int *volumeCurrent, int *volumeValue, int *currentVolumeValue)
{
    *volumeCurrent = analogRead(VOLUME);
    *volumeValue = round(*volumeCurrent / 1024.0 * 30);

    if (*volumeValue != *currentVolumeValue)
    {
        currentVolumeValue = volumeValue;
        Serial.print("Volume:");
        Serial.println(*currentVolumeValue);
        mp3player.volume(*currentVolumeValue);
    }
}

void onBigRedPress(SounduinoStateMachine fsm)
{
    fsm.event = bigRedSingle_Event;
}

void onBigRedDoublePress(SounduinoStateMachine fsm)
{
    fsm.event = bigRedDouble_Event;
}
void onBigRedLongPress(SounduinoStateMachine fsm)
{
    fsm.event = bigRedLong_Event;
}
void onFwdPress(SounduinoStateMachine fsm)
{
    fsm.event = fwdSingle_Event;
}
void onPrevPress(SounduinoStateMachine fsm)
{
    fsm.event = prevSingle_Event;
}
