#include "Arduino.h"

/***************************************************
 Sounduino Absolute Globals

 ****************************************************/

#define TRACK_COUNT 3

/***************************************************
 Sounduino Imports

 ****************************************************/
#include <pins.h>
#include <transitions.h>
#include <event_handler.h>
#include <rfid.h>

/***************************************************
 Sounduino Potentiometer

 ****************************************************/

int volumeCurrent = 0;
int currentVolumeValue = 0;
int volumeValue = 0;

/***************************************************
 Sounduino Helper

 ****************************************************/
int transitioncounter = 0;
long lastMillis = 0;

#include "OneButton.h"
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

/***************************************************
 MFRC522 RFID Card Module Setup

 ****************************************************/

#include <SPI.h>
#include <MFRC522.h>
#include <stdint.h>

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

/***************************************************
 DFPlayer - A Mini MP3 Player For Arduino
 <https://www.dfrobot.com/product-1121.html>

 ***************************************************
 This example shows the all the function of library for DFPlayer.

 Created 2016-12-07
 By [Angelo qiao](Angelo.qiao@dfrobot.com)

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
<https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include <DFPlayerMini_Fast.h>

#if !defined(UBRR1H)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#endif

DFPlayerMini_Fast mp3player;
int lastmp3PlayerAvailable;
const int BUSY = 4;

//declaration of a transition table which includes function pointers to the respective transition function
SounduinoEventHandler transitionTable[state_count][event_count];

/***************************************************
 Sounduino State Machine

 ***************************************************
 This state machine uses a Lookup table to make State transitions

 ****************************************************/

SounduinoStateMachine fsm = {.state = initializing_State, .event = no_Event, .stateProperties = {.paused = false, .currentRandomTrack = 0}};

void initializeTransitionTable()
{
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
  transitionTable[progPending_State][cardProgrammingFailed_Event] = transProgWait;

  transitionTable[menu_State][bigRedSingle_Event] = transPlayingShuffle;
  transitionTable[menu_State][bigRedDouble_Event] = transDeterPlayingState;
  transitionTable[menu_State][bigRedLong_Event] = playHelp;
  transitionTable[menu_State][fwdSingle_Event] = transPlayingSerial;
  transitionTable[menu_State][prevSingle_Event] = transPlayingStopdance;

  transitionTable[deterPlayingState_State][determinationFailed_Event] = transNotPlaying;
}

/***************************************************
 Sounduino Utility Function headers

 ****************************************************/
void initializeTransitionTable();
void initializeDFPlayerMini();

/***************************************************
 Sounduino Program Setup

 ****************************************************/

void setup()
{
  // initialize serial communication:
  Serial.begin(9600);
  Serial.setTimeout(20000);
  Serial.println("---------- Begin setup ----------");

  // Initialize DFPlayerMini
  initializeDFPlayerMini();

  //initialize last-state function pointer to fall back to shuffle
  changeLastPlayStateTransitionFunction(transPlayingShuffle);

  transInitializing(&mp3player, &fsm);

  initializeTransitionTable();

  // initialize possible button actions
  btnBigRed.attachClick(onBigRedPress, &fsm);
  btnBigRed.attachDoubleClick(onBigRedDoublePress, &fsm);
  btnBigRed.attachLongPressStop(onBigRedLongPress, &fsm);

  btnFwd.attachClick(onFwdPress, &fsm);

  btnPrev.attachClick(onPrevPress, &fsm);

  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
  Serial.print(F("Using key (for A and B):"));
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));

  fsm.state = transNotPlaying(&mp3player, &fsm);
  lastMillis = millis();
  Serial.println("---------- End Setup ----------");
  Serial.println("");
}

/***************************************************
 Sounduino Program Loop

 ****************************************************/

void loop()
{

  fsm.event = no_Event;

  // listen to buttonPress
  btnBigRed.tick();
  btnFwd.tick();
  btnPrev.tick();

  //handle potentiometer
  handlePotentiometer(&mp3player, &volumeCurrent, &volumeValue, &currentVolumeValue);

  //check for card
  checkForCard(&mfrc522, &fsm);

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
    fsm.state = transitionTable[fsm.state][fsm.event](&mp3player, &fsm);
    Serial.print("After: ");
    Serial.print(stateToText(fsm.state));
    Serial.println("");
    transitioncounter++;
    lastMillis = millis();
  }
}

/***************************************************
 Sounduino DFPlayerMini Functions

 ****************************************************/
void initializeDFPlayerMini()
{

  pinMode(BUSY, INPUT);
#if !defined(UBRR1H)
  mySerial.begin(9600);
  mp3player.begin(mySerial, true);
  delay(1000);
#else
  Serial1.begin(9600);
  mp3player.begin(Serial1, true);
#endif
  mp3player.volume(0);
  mp3player.EQSelect(0);
}
