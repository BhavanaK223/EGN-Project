//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>

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
        Description: This is the default state for the system. It waits for an RFID tag to be scanned before moving to case 1

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
        ***Possible addition of sending information to an ESP32 for AI processing***

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

      Micro SD Pinout
        //Pin 13 -- SCK Micro SD
        //Pin 12 -- MISO Micro SD
        //Pin 11 -- MOSI Micro SD
        //pin 8 -- CS (Chip Select) Micro SD

        //THE MICRO SD NEEDS 5 VOLTS MY GOD!!!!!


      Buttons
        //pin 2 -- Blue Acknowladge Button / High VCC side of button

      LEDs
        //pin 4 -- High side of RED LED
        //pin 3 -- High side of GREEN LED

      RFID 
        //Pin 10 -- SDA 
        //pin 9 -- RST


*/
    //Micro SD Initalization
      #include <SPI.h>
      #include <SD.h>

      #include <MFRC522.h> //RFID NEW  
 

      // set up variables using the SD utility library functions:
File myFile;



int state;
int laststate;

bool Tag;
bool Weight;
bool SDWrite;
bool Lemon;


bool Ack; //Bool for whether the acknoladge button is high or low

#define RED 4 //Digital pin 4 is now called RED - All caps
#define GREEN 3 //Digital pin 3 is now called RED - All caps
#define ACK 2 //Digital pin 2 is ACK, ACK stands for "Acknowladge button push"

#define SS_PIN 10 //RFID NEW
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(RED, OUTPUT);
pinMode(Ack, INPUT);

state = 0;
laststate =5;



 SPI.begin(); // Initializes the SPI bus 
 rfid.PCD_Init(); // Initializes the MFRC522 RFID reader

//SetUp SD Card
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 8;




  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } 
  else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}


void loop() {

//if(1==1)
  //{ 
   // Serial.print("Before Switch");
   //delay(500);    
      switch(state)
      {
        case 0: //Idle state -- Not finished
           // Serial.println(state);
            //delay(125);
          
          if ( ! rfid.PICC_IsNewCardPresent()){}
          
            // if the new card's NUID (Non-Unique IDentifier) hasn't been read yet, it skips the rest of the current loop 
          if ( ! rfid.PICC_ReadCardSerial()) {}
         
              // if both conditions pass, it gets the type of the PICC (proximity integrated circuit card) of the new card
          else
            { // PICC type could be MIFARE Mini, MIFARE 1K, MIFARE 4K, MIFARE Ultralight, MIFARE DESFire, etc.
              MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
              // it then prints the UID of the tag to the serial monitor
              Serial.print(F("RFID Tag UID:"));
              cowInfoWrite(rfid.uid.uidByte, rfid.uid.size, myFile);
              Serial.println("");
              laststate = state;
              state = state + 1;
              //Serial.print("Before Break");
              Serial.println(state);
            }
          break; //Go to next state 
            

        case 1: //Weight Measurement --Not finished
            Serial.println(state);
            laststate = state;
            state = state + 1;
            delay(6000);
            Serial.println("3Seconds");
            delay(3000);
        break;

        case 2://Micro SD
            Serial.println(state);
            laststate = state;
            delay(1000);
                  while (!Serial) {
                    ; // wait for serial port to connect. Needed for native USB port only
                  }


                  Serial.print("Initializing SD card...");

                  if (!SD.begin(8)) {
                    Serial.println("initialization failed!");
                    state = 4;
                  }
                  Serial.println("initialization done.");

                  // open the file. note that only one file can be open at a time,
                  // so you have to close this one before opening another.
                  myFile = SD.open("test.txt", FILE_WRITE);
                  // if the file opened okay, write to it:
                  if (myFile) {
                    Serial.print("Writing to test.txt...");
                    myFile.println("testing 1, 2, 3."); //Information that is written on the SD card as a string
                    myFile.println(state); 
                    cowInfoWrite(rfid.uid.uidByte, rfid.uid.size, myFile);
                    // close the file:
                    myFile.close();
                    Serial.println("done.");
                    
                    state = 3;
                  } else {
                    // if the file didn't open, print an error:
                    Serial.println("error opening test.txt");
                    state = 4;
                  }

                  // re-open the file for reading:
                  myFile = SD.open("test.txt");
                  
                           
        break;//Go to next state

        case 3://Complete-- Not finished, but will check if all actions for object being weighed have been done
            Serial.println(state);
            laststate = state;
            state = state + 1;
            delay(1000);
            break; //Go to next state


        case 4://Reset
            Serial.println(state); //Prints State
            Ack = digitalRead(ACK);//Reads ACK sees if high or low
            if(Ack == HIGH)
              { //If ACK is high, stay in state 4 and stay, also blink LED
                Serial.println("Stay");
                  digitalWrite(RED, HIGH);
                  delay(50);
                  digitalWrite(RED, LOW);
                  delay(50); 
              }

            else
              {//If ack is false leave the state and go to state 0, Resets bools for next measurement
                Serial.println("Leave");
                  Tag = false;
                  Weight = false;
                  SDWrite = false;               
                delay(500);
                laststate = state;
                state = 0; 
              }

        break;  //Go to next state

        default: //This means we are not in any state 0-5
            Serial.println("Error! Currently in no state see bottom of switch statement in code");
            delay (1000);
            /*Hi if you are here you managed to leave states 0-4, and exist in some weird in between state. defualt is 
            where you you arent in any state, which is here*/
         
        break; //Go to next state
        
      }
     // jackFunction();
    
    
  //}
//Serial.print("How did i get here");
}


void cowInfoWrite(byte *buffer, byte bufferSize, File& file) {
 for (byte i = 0; i < bufferSize; i++) {
   file.print(buffer[i] < 0x10 ? " 0" : " ");
   file.print(buffer[i], HEX);
  file.print(";");


 }




}




void jackFunction()
{
delay(1000);
Ack = digitalRead(ACK);//Reads ACK sees if high or low
if (Ack == HIGH) {
Serial.print("Hi");

}
else
{ Serial.print("low");}
}



