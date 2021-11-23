#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "Arduino.h"
#include "OneButton.h"

//debug libraries

// Match Button constants to pins
const int BIGRED = 5;
const int FWD = 6;
const int PREV = 7;

// Setup for potentiometer to control volume
const int VOLUME = A1;
int volumeCurrent = 0;
int currentVolumeValue = 0;
int volumeValue = 0;

// count state transitions
int transitioncounter = 0;

// Initialize Buttons with OneButton Library
/* See http://www.mathertel.de/License.aspx

Software License Agreement (BSD License)

Copyright (c) 2005-2014 by Matthias Hertel,  http://www.mathertel.de/

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
•Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
•Neither the name of the copyright owners nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
OneButton btnBigRed(BIGRED, false);
OneButton btnFwd(FWD, false);
OneButton btnPrev(PREV, false);

// States definition for Sounduino FSM
typedef enum
{
  notPlaying_State,
  playingSerial_State,
  playingShuffle_State,
  playingCard_State,
  playingStopdance_State,
  progWait_State,
  progDelete_State,
  progPending_State,
  deterPlayingState_State,
  menu_State,
  initializing_State,
  state_count
} SounduinoState;

// Events definition for Sounduino FSM
typedef enum
{
  bigRedSingle_Event,
  bigRedDouble_Event,
  bigRedLong_Event,
  songEnded_Event,
  fwdSingle_Event,
  prevSingle_Event,
  presentCard_Event,
  initialized_Event,
  determinationFailed_Event,
  cardProgrammed_Event,
  no_Event,
  event_count
} SounduinoEvent;

//typedef of function pointer
typedef SounduinoState (*SounduinoEventHandler)(void);

//declaration of transition tables which includes function pointers
SounduinoEventHandler transitionTable[state_count][event_count];

// transition function headers
SounduinoState transNotPlaying(void);
SounduinoState transPlayingSerial(void);
SounduinoState transPlayingShuffle(void);
SounduinoState transPlayingCard(void);
SounduinoState transPlayingStopdance(void);
SounduinoState transProgWait(void);
SounduinoState transProgDelete(void);
SounduinoState transProgPending(void);
SounduinoState transDeterPlayingState(void);
SounduinoState transMenu(void);
SounduinoState transInitializing(void);
SounduinoState doNothing(void);

//SounduinoState (*transitionTable[state_count][event_count])(void);

typedef struct
{
  int test;
  int test2;
} SounduinoStateProperties;

typedef struct
{
  SounduinoState state;
  SounduinoEvent event;
  SounduinoStateProperties stateProperties;
} SounduinoStateMachine;

SounduinoStateMachine fsm = {.state = initializing_State, .event = no_Event, .stateProperties = {.test = 1, .test2 = 2}};

SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

//event function headers
void onBigRedPress();
void onBigRedDoublePress();
void onBigRedLongPress();
void onFwdPress();
void onPrevPress();
void handlePotentiometer();

char *eventToText(SounduinoEvent event);
char *stateToText(SounduinoState state);

void setup()
{
  // initialize serial communication:
  Serial.begin(9600);
  Serial.println("---------- Begin setup ----------");
  transInitializing();

  for (size_t i = 0; i < state_count; i++)
  {
    for (size_t j = 0; j < event_count; j++)
    {
      transitionTable[i][j] = doNothing;
    }
  }

  // Set up state transition table
  transitionTable[notPlaying_State][presentCard_Event] = transPlayingCard;
  transitionTable[notPlaying_State][bigRedSingle_Event] = transDeterPlayingState;
  transitionTable[notPlaying_State][bigRedDouble_Event] = transMenu;

  transitionTable[playingShuffle_State][bigRedSingle_Event] = transNotPlaying;
  transitionTable[playingShuffle_State][fwdSingle_Event] = transPlayingShuffle;
  transitionTable[playingShuffle_State][prevSingle_Event] = transPlayingShuffle;
  transitionTable[playingShuffle_State][bigRedLong_Event] = transProgWait;
  transitionTable[playingShuffle_State][bigRedDouble_Event] = transMenu;

  transitionTable[playingSerial_State][bigRedSingle_Event] = transNotPlaying;
  transitionTable[playingSerial_State][fwdSingle_Event] = transPlayingSerial;
  transitionTable[playingSerial_State][prevSingle_Event] = transPlayingSerial;
  transitionTable[playingSerial_State][bigRedLong_Event] = transProgWait;
  transitionTable[playingSerial_State][bigRedDouble_Event] = transMenu;

  transitionTable[playingStopdance_State][bigRedSingle_Event] = transNotPlaying;
  transitionTable[playingStopdance_State][fwdSingle_Event] = transPlayingStopdance;
  transitionTable[playingStopdance_State][prevSingle_Event] = transPlayingStopdance;
  transitionTable[playingStopdance_State][bigRedLong_Event] = transProgWait;
  transitionTable[playingStopdance_State][bigRedDouble_Event] = transMenu;

  transitionTable[playingCard_State][bigRedSingle_Event] = transNotPlaying;
  transitionTable[playingCard_State][bigRedLong_Event] = transProgDelete;
  transitionTable[playingCard_State][bigRedDouble_Event] = transMenu;

  transitionTable[progWait_State][presentCard_Event] = transProgPending;
  transitionTable[progWait_State][bigRedLong_Event] = transDeterPlayingState;

  transitionTable[progDelete_State][presentCard_Event] = transProgPending;

  transitionTable[progPending_State][cardProgrammed_Event] = transDeterPlayingState;

  transitionTable[menu_State][bigRedSingle_Event] = transPlayingShuffle;
  transitionTable[menu_State][bigRedDouble_Event] = transDeterPlayingState;
  transitionTable[menu_State][fwdSingle_Event] = transPlayingSerial;
  transitionTable[menu_State][prevSingle_Event] = transPlayingStopdance;

  transitionTable[deterPlayingState_State][determinationFailed_Event] = transNotPlaying;

  // initialize possible button actions
  btnBigRed.attachClick(onBigRedPress);
  btnBigRed.attachDoubleClick(onBigRedDoublePress);
  btnBigRed.attachLongPressStop(onBigRedLongPress);

  btnFwd.attachClick(onFwdPress);

  btnPrev.attachClick(onPrevPress);

  fsm.state = transNotPlaying();
  Serial.println("---------- End Setup ----------");
  Serial.println("");
}

void loop()
{

  fsm.event = no_Event;

  // listen to buttonPress
  btnBigRed.tick();
  btnFwd.tick();
  btnPrev.tick();

  //handle potentiometer
  handlePotentiometer();

  //evaluate state
  if (fsm.event != no_Event)
  {
    Serial.println("");
    Serial.print("------ Transition ");
    Serial.print(transitioncounter);
    Serial.println(" ------");
    Serial.print("Before: ");
    Serial.println(stateToText(fsm.state));
    Serial.print("Event: ");
    Serial.println(eventToText(fsm.event));
    Serial.print("Action: ");
    fsm.state = transitionTable[fsm.state][fsm.event]();
    Serial.print("After: ");
    Serial.print(stateToText(fsm.state));
    Serial.println("");
    transitioncounter++;
  }
}

void handleCardReader()
{
  if (false)
    fsm.event = presentCard_Event;
}

void handlePotentiometer()
{
  volumeCurrent = analogRead(VOLUME);
  volumeValue = round(volumeCurrent / 1024.0 * 30);

  if (volumeValue != currentVolumeValue)
  {
    currentVolumeValue = volumeValue;
    String printout = "Volume:";
    printout = printout + currentVolumeValue;
    Serial.println(printout);
  }
}

void onBigRedPress()
{
  fsm.event = bigRedSingle_Event;
}

void onBigRedDoublePress()
{
  fsm.event = bigRedDouble_Event;
}
void onBigRedLongPress()
{
  fsm.event = bigRedLong_Event;
}
void onFwdPress()
{
  fsm.event = fwdSingle_Event;
}
void onPrevPress()
{
  fsm.event = prevSingle_Event;
}

SounduinoState transNotPlaying(void)
{
  Serial.println("Transition to: not Playing");
  return notPlaying_State;
}

SounduinoState transPlayingSerial(void)
{
  Serial.println("Transition to: playingSerial");
  return playingSerial_State;
}
SounduinoState transPlayingShuffle(void)
{
  Serial.println("Transition to: playingShuffle");
  return playingShuffle_State;
}
SounduinoState transPlayingCard(void)
{
  Serial.println("Transition to: playingCard");
  return playingCard_State;
}
SounduinoState transPlayingStopdance(void)
{
  Serial.println("Transition to: playingStopdance");
  return playingStopdance_State;
}
SounduinoState transProgWait(void)
{
  Serial.println("Transition to: progWait");
  return progWait_State;
}
SounduinoState transProgDelete(void)
{
  Serial.println("Transition to: progDelete");
  return progDelete_State;
}
SounduinoState transProgPending(void)
{
  Serial.println("Transition to: progPending");
  return progPending_State;
}
SounduinoState transDeterPlayingState(void)
{
  // determine state to transition to
  SounduinoState stateBefore = playingShuffle_State;
  Serial.print("Transition to: ");
  Serial.print(stateToText(stateBefore));
  Serial.print(" via: ");
  Serial.println("deterPlayingState_State");
  return stateBefore;
}
SounduinoState transMenu(void)
{
  Serial.println("Transition to: menu");
  return menu_State;
}
SounduinoState transInitializing(void)
{
  Serial.println("Transition to: initializing");
  return initializing_State;
}
SounduinoState doNothing(void)
{
  Serial.println("Nothing happens");
  return fsm.state;
}

char *stateToText(SounduinoState state)
{
  char *statetext[] = {
      "notPlaying_State",
      "playingSerial_State",
      "playingShuffle_State",
      "playingCard_State",
      "playingStopdance_State",
      "progWait_State",
      "progDelete_State",
      "progPending_State",
      "deterPlayingState_State",
      "menu_State",
      "initializing_State",
      "state_count"};
  return statetext[state];
}

char *eventToText(SounduinoEvent event)
{
  char *eventtext[] = {
      "bigRedSingle_Event",
      "bigRedDouble_Event",
      "bigRedLong_Event",
      "songEnded_Event",
      "fwdSingle_Event",
      "prevSingle_Event",
      "presentCard_Event",
      "initialized_Event",
      "determinationFailed_Event",
      "cardProgrammed_Event",
      "no_Event",
      "event_count"};
  return eventtext[event];
}
