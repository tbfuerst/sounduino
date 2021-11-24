#include "Arduino.h"

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

#include <SPI.h>
#include <MFRC522.h>
#include <stdint.h>
/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 *
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 *
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 *
  * -----------------------------------------
  * Pin layout
  * -----------------------------------------
  * MFRC522      Arduino
  * Reader       Nano
  * Pin          Pin
  * -----------------------------------------
  * RST          D9
  * SDA(SS)      D10
  * MOSI         D11
  * MISO         D12
  * SCK          D13
  * NC(IRQ)      not used
  * 3.3V         3.3V
  * GND          GND
  * --------------------------------------------------------------------------
  */
#define RST_PIN 9                 // Configurable, see typical pin layout above
#define SS_PIN 10                 // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

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

SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

/***************************************************
 Sounduino States and Events

 ****************************************************/
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
  cardProgrammingFailed_Event,
  cardProgrammed_Event,
  no_Event,
  event_count
} SounduinoEvent;

/***************************************************
 Sounduino State Machine Transition Table

 ****************************************************/

//typedef of a function pointer to use in the following transition table
typedef SounduinoState (*SounduinoEventHandler)(void);

//declaration of a transition table which includes function pointers to the respective transition function
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
  transitionTable[menu_State][fwdSingle_Event] = transPlayingSerial;
  transitionTable[menu_State][prevSingle_Event] = transPlayingStopdance;

  transitionTable[deterPlayingState_State][determinationFailed_Event] = transNotPlaying;
}

//function pointer to store last playing State to transition to
SounduinoEventHandler transToLastPlayState;
void saveLastPlayState();

/***************************************************
 Sounduino State Machine

 ***************************************************
 This state machine uses a Lookup table to make State transitions

 ****************************************************/

typedef struct
{
  bool paused;
} SounduinoStateProperties;

typedef struct
{
  SounduinoState state;
  SounduinoEvent event;
  SounduinoStateProperties stateProperties;
} SounduinoStateMachine;

SounduinoStateMachine fsm = {.state = initializing_State, .event = no_Event, .stateProperties = {.paused = false}};

/***************************************************
 Sounduino Utility Function headers

 ****************************************************/
void onBigRedPress();
void onBigRedDoublePress();
void onBigRedLongPress();
void onFwdPress();
void onPrevPress();
void handlePotentiometer();
void handleCard();
void checkForCard();
void dump_byte_array(byte *buffer, byte bufferSize);
void initializeTransitionTable();
void initializeDFPlayerMini();

char *eventToText(SounduinoEvent event);
char *stateToText(SounduinoState state);

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
  transToLastPlayState = transPlayingShuffle;

  transInitializing();

  initializeTransitionTable();

  // initialize possible button actions
  btnBigRed.attachClick(onBigRedPress);
  btnBigRed.attachDoubleClick(onBigRedDoublePress);
  btnBigRed.attachLongPressStop(onBigRedLongPress);

  btnFwd.attachClick(onFwdPress);

  btnPrev.attachClick(onPrevPress);

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

  fsm.state = transNotPlaying();
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
  handlePotentiometer();

  //check for card
  checkForCard();

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

/***************************************************
 Sounduino Event Functions

 ****************************************************/

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
    myDFPlayer.volume(currentVolumeValue);
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

/***************************************************
 Sounduino Transition functions

 ****************************************************/

SounduinoState transNotPlaying(void)
{
  saveLastPlayState();
  myDFPlayer.pause();
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
  if (fsm.stateProperties.paused)
    myDFPlayer.start();
  else
    myDFPlayer.randomAll();
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
  saveLastPlayState();
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
  Serial.print("via deterPlayingState_State, ");
  return transToLastPlayState();
}
SounduinoState transMenu(void)
{
  saveLastPlayState();
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

/***************************************************
 Sounduino Utility Functions

 ****************************************************/

void saveLastPlayState()
{
  fsm.stateProperties.paused = true;
  switch (fsm.state)
  {
  case playingShuffle_State:
    transToLastPlayState = transPlayingShuffle;
    break;
  case playingSerial_State:
    transToLastPlayState = transPlayingSerial;
    break;
  case notPlaying_State:
    transToLastPlayState = transNotPlaying;
    break;
  case playingCard_State:
    transToLastPlayState = transPlayingCard;
    break;
  case playingStopdance_State:
    transToLastPlayState = transPlayingStopdance;
    break;
  default:
    fsm.stateProperties.paused = false;
    transToLastPlayState = transPlayingShuffle;
    break;
  }
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
      "cardProgrammingFailed_Event"
      "cardProgrammed_Event",
      "no_Event",
      "event_count"};
  return eventtext[event];
}

/***************************************************
 Sounduino RFID Reader Functions

 ****************************************************/

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void checkForCard()
{
  if (mfrc522.PICC_IsNewCardPresent())

    fsm.event = presentCard_Event;
}

void handleCard()
{

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  // In this sample we use the second sector,
  // that is: sector #1, covering block #4 up to and including block #7
  byte sector = 1;
  byte blockAddr = 4;
  byte dataBlock[] = {
      0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
      0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
      0x09, 0x0a, 0xff, 0x0b, //  9, 10, 255, 11,
      0x0c, 0x0d, 0x0e, 0x0f  // 12, 13, 14, 15
  };
  byte trailerBlock = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block "));
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block "));
  Serial.print(blockAddr);
  Serial.println(F(":"));
  dump_byte_array(buffer, 16);
  Serial.println();
  Serial.println();

  // Authenticate using key B
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write data to the block
  Serial.print(F("Writing data into block "));
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(dataBlock, 16);
  Serial.println();
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

  // Read data from the block (again, should now be what we have written)
  Serial.print(F("Reading data from block "));
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block "));
  Serial.print(blockAddr);
  Serial.println(F(":"));
  dump_byte_array(buffer, 16);
  Serial.println();

  // Check that data in block is what we have written
  // by counting the number of bytes that are equal
  Serial.println(F("Checking result..."));
  byte count = 0;
  for (byte i = 0; i < 16; i++)
  {
    // Compare buffer (= what we've read) with dataBlock (= what we've written)
    if (buffer[i] == dataBlock[i])
      count++;
  }
  Serial.print(F("Number of bytes that match = "));
  Serial.println(count);
  if (count == 16)
  {
    Serial.println(F("Success :-)"));
  }
  else
  {
    Serial.println(F("Failure, no match :-("));
    Serial.println(F("  perhaps the write didn't work properly..."));
  }
  Serial.println();

  // Dump the sector data
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

/***************************************************
 Sounduino DFPlayerMini Functions

 ****************************************************/
void initializeDFPlayerMini()
{

  mySoftwareSerial.begin(9600);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial))
  { //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true)
      ;
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.volume(0);
}
