#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
/***************************************************
 Sounduino Potentiometer

 ****************************************************/
const int VOLUME = A1;

/***************************************************
 Sounduino Buttons

 ****************************************************/
const int BIGRED = 5;
const int FWD = 6;
const int PREV = 7;

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
#define RST_PIN 9 // Configurable, see typical pin layout above
#define SS_PIN 10 // Configurable, see typical pin layout above

#endif
