#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <Arduino.h>
#include <utility.h>
#include <MFRC522.h>

void handlePotentiometer(DFPlayerMini_Fast *mp3player, int *volumeCurrent, int *volumeValue, int *currentVolumeValue);
void onBigRedPress(SounduinoStateMachine *fsm);
void onBigRedDoublePress(SounduinoStateMachine *fsm);
void onBigRedLongPress(SounduinoStateMachine *fsm);
void onFwdPress(SounduinoStateMachine *fsm);
void onPrevPress(SounduinoStateMachine *fsm);
void checkForCard(MFRC522 *mfrc522, SounduinoStateMachine *fsm);

#endif
