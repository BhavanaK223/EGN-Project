/*
Engineering Design and Society 2020C Final Project: Livestock Data Collection and System

Team members: Eric (Jack) Rainville,

Table #: 3

Description: This code runs a data collection system for livestock that tracks weight.
The code functions as a state machine, listed below are the states and state functions

Example Sketches and Code Refrences
    1. https://forum.arduino.cc/t/beginners-using-the-switch-case-statement/680177
    2. https://www.arduino.cc/en/Tutorial/LibraryExamples/ReadWrite
    3. SD Library Read Write file and commands




0: Idle State -
        Inputs: RFID Scanner, Reset button
        Outputs: Blue LED, Green LED
        Data Change: The RFID tag # is stored, and bool Tag becomes true
        Description:

1: Weight Measurement -
        Inputs: HX711 Load Cell, Pushbutton
        Outputs: Green LED, Red LED
        Data Change: The Scale reading is stored, and bool Weight becomes true
        Description: Once the pushbutton is pressed, a measurement is taken from the HX711 weight sensor.
        If the measurement is below X pounds and above Y pounds reading is successful and written bool Weight
        becomes true, and state is switched to 3:Micro SD. If it is false, state is sent to Reset.


2: Micro SD -
        Inputs: None
        Outputs: SDWrite to micro SD
        Data Change: If the card is initalized and written to, bool SDWrite becomes true
        Description: If the SD Card is successfully initalized send to state complete.

3: Complete -
        Inputs: Push Button
        Outputs: Red, Green LED
        Data Change: Checks that bool Tag, Weight, SDWrite are true
        Description: If all bools are true, set Green LED on until pushbutton is pressed
        If it is false, send to solid Red LED until pushbutton is pressed
        Once pushbutton is pressed send to state reset


4: Reset -
        Inputs: None
        Outputs: Red
        Data Change: Changes bool Tag, Weight, SDWrite all to false
        Description: Resets the code, and sends to state Idle after acknowladge button is pressed

 */

/*Pinouts

   //Micro SD Pinout
       //Pin 13 -- SCK Micro SD
       //Pin 12 -- MISO Micro SD
       //Pin 11 -- MOSI Micro SD
       //pin 4 -- CS (Chip Select) Micro SD


   //Buttons
       //pin 2 -- Blue Acknowladge Button / High VCC side of button

   //LEDs
       //pin 3 -- High side of RED LED
*/
// Micro SD Initalization
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include "HX711.h"

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

File myFile;

int state;
int laststate;

bool Tag;
bool Weight;
bool SDWrite;

bool Ack; // Bool for whether the acknoladge button is high or low

const int chipSelect = 10;
int weight = 10;

#define BLUE 7
#define ACK 2       // Digital pin 2 is ACK, ACK stands for "Acknowladge button push"
#define RED 3       // Digital pin 3 is now called RED
#define GREEN 4     // Digital pin 4 is now called GREEN
#define SS_PIN 9    //CLOCK PIN
#define RST_PIN 8   //RFID PIN

// Creating an instance of the MFRC522 class and passing SS_PIN and RST_PIN as parameters
MFRC522 rfid(SS_PIN, RST_PIN);

// MIFARE_Key is a type of key used by MIFARE RFID tags, this is not used in this script but could later be used for authentication
MFRC522::MIFARE_Key key;

HX711 scale;

uint8_t dataPin = 5;
uint8_t clockPin = 6;
float w1, w2, previous = 0;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Setup!");
    pinMode(RED, OUTPUT);
    pinMode(Ack, INPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);

    digitalWrite(BLUE, LOW);
    

    // weight
    scale.begin(dataPin, clockPin);
    scale.set_scale(355.914154); // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare();

    state = 0;
    laststate = 5;

    SPI.begin();     // Initializes the SPI bus
    rfid.PCD_Init(); // Initializes the MFRC522 RFID reader
    Serial.print("8 Seconds -- Plug in MicroSD Card");
    delay(8000);

    Serial.print("Initializing SD card...");

    if (!SD.begin(10))  // Initializes the SD card and the file system
    {
        Serial.println("initialization failed!");
    }
    Serial.println("initialization done.");
}

void loop()
{
    Serial.print("begin: ");
    Serial.println(state);
    delay(5);
    if (state == 0)                   // WEIGHT
    {                                 
        Serial.println("in state 0"); // Prints State
        Ack = digitalRead(ACK);       // Reads ACK sees if high or low
        
        if (Ack == HIGH)
        { // If ACK is high, stay in state 4 and stay, also blink LED
            Serial.println("Place your weight, Push button to continue:");
            digitalWrite(BLUE, HIGH);
            delay(50);
            digitalWrite(BLUE, LOW);
            delay(50);
        }
        else
        {
            w1 = scale.get_units(10);
            delay(100);
            w2 = scale.get_units();
            while (abs(w1 - w2) > 10)//Stablizes weight value
            {
                w1 = w2;
                w2 = scale.get_units();
                delay(100);
            }
            Serial.print("UNITS: ");
            Serial.print(w1);
            if (w1 == 0)
            {
                Serial.println();
            }
            else                    //Measures True Weight Value
            {
                Serial.print("\t\tDELTA: ");
                Serial.println(w1 - previous);
                previous = w1;
            }
            delay(100);
            digitalWrite(BLUE, HIGH);
            laststate = state;
            state = state + 1;
        }
    }
    else if (state == 1)              //Reading RFID
    {                                 
        Serial.println("in state 1"); // Prints State
        // if there's no new card present on the sensor it skips the rest of the current loop
        if (!rfid.PICC_IsNewCardPresent())
            return;

        // if the new card's NUID (Non-Unique IDentifier) hasn't been read yet, it skips the rest of the current loop
        if (!rfid.PICC_ReadCardSerial())
            return;

        // if both conditions pass, it gets the type of the PICC (proximity integrated circuit card) of the new card
        // PICC type could be MIFARE Mini, MIFARE 1K, MIFARE 4K, MIFARE Ultralight, MIFARE DESFire, etc.
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

        // it then prints the UID of the tag to the serial monitor
        Serial.print(F("RFID Tag UID:"));
        printHex(rfid.uid.uidByte, rfid.uid.size);
        myFile.close();
        delay(500);

        rfid.PICC_HaltA(); // commands the “currently selected” PICC into halt
        delay(50);
        laststate = state;
        state = 2;
        Serial.println(state);
    }

    else if (state == 2)              //Wrties to SD Card
    {                                
        Serial.println("in state 2"); // Prints State
        laststate = state;
        delay(1000);
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        myFile = SD.open("test.txt", FILE_WRITE);

        // if the file opened okay, write to it:
        if (myFile)
        {
            // close the file:
            myFile.close();
            Serial.println("done.");
            state = 3;
        }
        else
        {
            // if the file didn't open, print an error:
            Serial.println("error opening test.txt");
            state = 4;
        }

        // re-open the file for reading:
        myFile = SD.open("test.txt");
        Serial.println("Break");
    }

    else if (state == 3)// Blink GREEN aft scan
    {   
        digitalWrite(BLUE, LOW);                        
        Serial.println("in state 3"); // Prints State
        Ack = digitalRead(ACK); 
        if (Ack == HIGH)
        { // If ACK is high, stay in state 4 and stay, also blink LED
            digitalWrite(GREEN, HIGH);
            delay(50);
            digitalWrite(GREEN, LOW);
            delay(50);
        }
        else
        {
            laststate = state;
            state = state + 1;
        }
    }

    else if (state == 4) //RESET
    {                              
        Serial.println("in state 4"); // Prints State
        Ack = digitalRead(ACK);       // Reads ACK sees if high or low
        if (Ack == HIGH)
        { // If ACK is high, stay in state 4 and stay, also blink LED
            digitalWrite(RED, HIGH);
            delay(50);
            digitalWrite(RED, LOW);
            delay(50);
        }

        else
        { // If ack is false leave the state and go to state 4
            Serial.println("Leave");
            Tag = false;
            Weight = false;
            SDWrite = false;
            delay(500);
            laststate = state;
            state = 0;
        }
    }

    else
    { // This means we are not in any state 0-5
        Serial.println("Error! Currently in no state see bottom of switch statement in code");
        delay(1000);
        /*Hi if you are here you managed to leave states 0-4, and exist in some weird in between state. defualt is
        where you you arent in any state, which is here*/
    }
}

// Helper function to print an array of bytes as hex values
void printHex(byte *buffer, byte bufferSize)
{

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    myFile = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile)
    { // Print RFID to SD Card
        for (byte i = 0; i < bufferSize; i++)
        {
            Serial.print(buffer[i] < 0x10 ? " 0" : " ");
            myFile.print(buffer[i] < 0x10 ? " 0" : " ");
            myFile.print(buffer[i], HEX);
            Serial.print(buffer[i], HEX);
        }
        myFile.print(";");
        myFile.print(w1);
        myFile.println();
    }
    else
    {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
}
