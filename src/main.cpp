#include <SoftwareSerial.h>

#include <DFRobotDFPlayerMini.h>

#include <AceButton.h>
using namespace ace_button;

/***************************************************
DFPlayer - A Mini MP3 Player For Arduino
 <https://www.dfrobot.com/index.php?route=product/product&product_id=1121>

 ***************************************************
 This example shows the basic function of library for DFPlayer.

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

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// this constant won't change:
const int BIGRED = 5;   // the pin that the pushbutton is attached to
const int FWD = 6;
const int PREV = 7;

const int VOLUME = A1;
int volumeCurrent = 0;
int currentVolumeValue = 0;
int volumeValue = 0;

SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

AceButton btnBigRed(BIGRED);
AceButton btnFwd(FWD);
AceButton btnPrev(PREV);

//void printDetail(uint8_t type, int value);

// header function
void handleBigRed(AceButton*, uint8_t, uint8_t);
void handleFwd(AceButton*, uint8_t, uint8_t);
void handlePrev(AceButton*, uint8_t, uint8_t);

void setup()
{
   // initialize serial communication:
  Serial.begin(9600);

  // initialize pins
  pinMode(BIGRED, INPUT);
  pinMode(FWD, INPUT);
  pinMode(PREV, INPUT);

  // configure big red button
  ButtonConfig *btnBigRedConfig = btnBigRed.getButtonConfig();
  btnBigRedConfig->setEventHandler(handleBigRed);
  btnBigRedConfig->setFeature(ButtonConfig::kFeatureClick);
  btnBigRedConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  btnBigRedConfig->setFeature(ButtonConfig::kFeatureLongPress);

  // configure fwd button
  ButtonConfig *btnFwdConfig = btnFwd.getButtonConfig();
  btnFwdConfig->setEventHandler(handleFwd);
  btnFwdConfig->setFeature(ButtonConfig::kFeatureClick);

  // configure prev button
  ButtonConfig *btnPrevConfig = btnPrev.getButtonConfig();
  btnPrevConfig->setEventHandler(handlePrev);
  btnPrevConfig->setFeature(ButtonConfig::kFeatureClick);
}

void loop()
{
  volumeCurrent = analogRead(VOLUME);
  volumeValue = round(volumeCurrent / 1024 * 30);

  if (volumeValue != currentVolumeValue) {
    currentVolumeValue = volumeValue;
    Serial.println("Volume: "+ currentVolumeValue);
  }

  btnBigRed.check();
  btnFwd.check();
  btnFwd.check();

}

void handleBigRed(AceButton*, uint8_t eventType, uint8_t){
  switch (eventType) {
    case AceButton::kEventClicked:
      Serial.println("Big Red single click");
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Big Red double click");
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Big Red long press");
      break;
    }
}
void handleFwd(AceButton*, uint8_t eventType, uint8_t){
  switch (eventType) {
    case AceButton::kEventClicked:
      Serial.println("Fwd single click");
      break;
  }
}
void handlePrev(AceButton*, uint8_t eventType, uint8_t){
  switch (eventType) {
    case AceButton::kEventClicked:
      Serial.println("Prev single click");
      break;
  }
}
