
#include <Arduino.h>
#include <utility.h>

void handlePotentiometer(DFPlayerMini_Fast mp3player, int *volumeCurrent, int *volumeValue, int *currentVolumeValue);
void onBigRedPress(SounduinoStateMachine fsm);
void onBigRedDoublePress(SounduinoStateMachine fsm);
void onBigRedLongPress(SounduinoStateMachine fsm);
void onFwdPress(SounduinoStateMachine fsm);
void onPrevPress(SounduinoStateMachine fsm);
