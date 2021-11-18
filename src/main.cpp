#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "OneButton.h"

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
  cardProgrammed_Event

} SounduinoEvent;

//typedef of function pointer
typedef SounduinoState (*SounduinoEventHandler)(void);

typedef struct
{
  SounduinoState state;
  SounduinoEvent event;
  SounduinoEventHandler eventHandler;
} SounduinoStateMachine;

// Setup for communication with DFPlayer mini MP3-Player
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

//function headers
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

  // initialize possible button actions
  btnBigRed.attachClick(onBigRedPress);
  btnBigRed.attachDoubleClick(onBigRedDoublePress);
  btnBigRed.attachLongPressStop(onBigRedLongPress);

  btnFwd.attachClick(onFwdPress);

  btnPrev.attachClick(onPrevPress);
}

void loop()
{

  // listen to buttonPress
  btnBigRed.tick();
  btnFwd.tick();
  btnPrev.tick();

  //handle potentiometer
  handlePotentiometer();
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
}

void onBigRedDoublePress()
{
  Serial.println("big red double pressed!");
}
void onBigRedLongPress()
{
  Serial.println("big red long pressed!");
}
void onFwdPress()
{
  Serial.println("fwd pressed!");
}
void onPrevPress()
{
  Serial.println("prev pressed!");
}
