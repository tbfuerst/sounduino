
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <stdint.h>

void dump_byte_array(byte *buffer, byte bufferSize);
void handleCard(MFRC522 *mfrc522, MFRC522::MIFARE_Key key);
