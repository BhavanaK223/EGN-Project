

// Included libraries. SPI is a communication protocol and MFRC522 is for RFID module.
#include <SPI.h>
#include <MFRC522.h>


// Definitions for RFID reader's SS (SPI Slave Select) and RST (Reset) pins.
#define SS_PIN 10
#define RST_PIN 9


// Creating an instance of the MFRC522 class and passing SS_PIN and RST_PIN as parameters
MFRC522 rfid(SS_PIN, RST_PIN);


// MIFARE_Key is a type of key used by MIFARE RFID tags, this is not used in this script but could later be used for authentication
MFRC522::MIFARE_Key key;


// setup function that runs once on a restart or power up
void setup() {
 Serial.begin(9600); // this starts serial communication with baud rate of 9600
 SPI.begin(); // Initializes the SPI bus 
 rfid.PCD_Init(); // Initializes the MFRC522 RFID reader
}


// loop function that runs repeatedly after executing the setup().
void loop() {


 // if there's no new card present on the sensor it skips the rest of the current loop  
 if ( ! rfid.PICC_IsNewCardPresent())
   return;


 // if the new card's NUID (Non-Unique IDentifier) hasn't been read yet, it skips the rest of the current loop 
 if ( ! rfid.PICC_ReadCardSerial())
   return;


 // if both conditions pass, it gets the type of the PICC (proximity integrated circuit card) of the new card
 // PICC type could be MIFARE Mini, MIFARE 1K, MIFARE 4K, MIFARE Ultralight, MIFARE DESFire, etc.
 MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);


 // it then prints the UID of the tag to the serial monitor
 Serial.print(F("RFID Tag UID:"));
 printHex(rfid.uid.uidByte, rfid.uid.size);
 Serial.println("");


 rfid.PICC_HaltA(); // commands the “currently selected” PICC into halt 
}


// Helper function to print an array of bytes as hex values
void printHex(byte *buffer, byte bufferSize) {
 for (byte i = 0; i < bufferSize; i++) {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], HEX);
 }






