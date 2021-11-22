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

void setup()
{
  // initialize serial communication:
  Serial.begin(9600);
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

  transitionTable[progDelete_State][presentCard_Event] = transProgPending;

  transitionTable[progPending_State][cardProgrammed_Event] = transDeterPlayingState;

  transitionTable[menu_State][bigRedSingle_Event] = transPlayingShuffle;
  transitionTable[menu_State][bigRedDouble_Event] = transDeterPlayingState;
  transitionTable[menu_State][fwdSingle_Event] = transPlayingSerial;
  transitionTable[menu_State][prevSingle_Event] = transPlayingShuffle;

  transitionTable[deterPlayingState_State][determinationFailed_Event] = transNotPlaying;

  // initialize possible button actions
  btnBigRed.attachClick(onBigRedPress);
  btnBigRed.attachDoubleClick(onBigRedDoublePress);
  btnBigRed.attachLongPressStop(onBigRedLongPress);

  btnFwd.attachClick(onFwdPress);

  btnPrev.attachClick(onPrevPress);

  fsm.state = transNotPlaying();
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
    Serial.println("Before:");
    Serial.println(fsm.state);
    Serial.println(fsm.event);
    Serial.println("---");
    Serial.println("After:");
    fsm.state = transitionTable[fsm.state][fsm.event]();
    Serial.println(fsm.state);
    Serial.println(fsm.event);
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
  Serial.println("big red pressed!");
  fsm.event = bigRedSingle_Event;
}

void onBigRedDoublePress()
{
  Serial.println("big red double pressed!");
  fsm.event = bigRedDouble_Event;
}
void onBigRedLongPress()
{
  Serial.println("big red long pressed!");
  fsm.event = bigRedLong_Event;
}
void onFwdPress()
{
  Serial.println("fwd pressed!");
  fsm.event = fwdSingle_Event;
}
void onPrevPress()
{
  Serial.println("prev pressed!");
  fsm.event = prevSingle_Event;
}

SounduinoState transNotPlaying(void)
{
  Serial.println("State: not Playing");
  return notPlaying_State;
}

SounduinoState transPlayingSerial(void)
{
  Serial.println("State: playingSerial");
  return playingSerial_State;
}
SounduinoState transPlayingShuffle(void)
{
  Serial.println("State: playingShuffle");
  return playingShuffle_State;
}
SounduinoState transPlayingCard(void)
{
  Serial.println("State: playingCard");
  return playingCard_State;
}
SounduinoState transPlayingStopdance(void)
{
  Serial.println("State: playingStopdance");
  return playingStopdance_State;
}
SounduinoState transProgWait(void)
{
  Serial.println("State: progWait");
  return progWait_State;
}
SounduinoState transProgDelete(void)
{
  Serial.println("State: progDelete");
  return progDelete_State;
}
SounduinoState transProgPending(void)
{
  Serial.println("State: progPending");
  return progPending_State;
}
SounduinoState transDeterPlayingState(void)
{
  Serial.println("State: deterPlayingState");
  return deterPlayingState_State;
}
SounduinoState transMenu(void)
{
  Serial.println("State: menu");
  return menu_State;
}
SounduinoState transInitializing(void)
{
  Serial.println("State: initializing");
  return initializing_State;
}
SounduinoState doNothing(void)
{
  return fsm.state;
}
