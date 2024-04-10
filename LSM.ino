/* 

LSM (Livestock Management System) 
    Class: Engineering Design and Society 2020C
    Team members: Jack Rainville
    Table #: 3

Pinouts: 

  RFID:
    Pin 10 -- SDA
    Pin 09 -- RST

  Micro SD:
    Pin 13 -- SCK  (Yellow)
    Pin 12 -- MISO (Blue)
    Pin 11 -- MPSI (Green)
    Pin 08  -- CS (Purple)

  HX711:  
    NA

  Buttons:
    Pin 02 -- High side of button

  LEDs:
    Pin 04 -- Anode of RED LED
    Pin 03 -- Anode of GREEN LED 

*/


 #include <SPI.h>
 #include <SD.h>
 #include <MFRC522.h>

File myFile;

Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 8;

int state;
int lastState;

bool tag;
bool weight;
bool SDWrite;
bool ack;

#define RED 4
#define Green 34
#define Button 2
#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);




void setup() {

  Serial.begin(9600);
  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }






}

void loop() {
  // put your main code here, to run repeatedly:

}
