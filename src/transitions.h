#ifndef TRANSITIONS_H
#define TRANSITIONS_H

#include <Arduino.h>
#include <utility.h>
#include <DFPlayerMini_Fast.h>

/***************************************************
 Sounduino Transition functions

 ****************************************************/

//function pointer to store last playing State to transition to

void saveLastPlayState(SounduinoStateMachine *fsm);
void playRandom(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
void changeLastPlayStateTransitionFunction(SounduinoEventHandler transitionFunctionToAssign);
SounduinoState transNotPlaying(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transPlayingSerial(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transPlayingShuffle(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transPlayingCard(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transPlayingStopdance(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transProgWait(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transProgDelete(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transProgPending(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transDeterPlayingState(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transMenu(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);

SounduinoState playHelp(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState transInitializing(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);
SounduinoState doNothing(DFPlayerMini_Fast *mp3player, SounduinoStateMachine *fsm);

#endif
