/* NFC BWAT shared box access and inventory management */


#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>    // Library for Mifare RC522 Devices
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
 
#define relay 4     // Set Relay Pin
#define wipeB 3     // Button pin for WipeMode

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

boolean match = false;          // initialize card match to false
boolean programMode = false;  // initialize programming mode to false

int successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
MOSI: Pin 11 / ICSP-4
  MISO: Pin 12 / ICSP-1
  SCK : Pin 13 / ICSP-3
  SS : Pin 10 (Configurable)
  RST : Pin 9 (Configurable)
/*
  We need to define MFRC522's pins and create instance
  Pin layout should be as follows (on Arduino Uno):
  
  look MFRC522 Library for
  other Arduinos' pin configuration 
 */

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
 
    // Initialize all pixels to 'off'
    delay(1000);
  
    //Arduino Pin Configuration
  
    //pinMode(wipeB, INPUT_PULLUP);   // Enable pin's pull up resistor
    pinMode(relay, OUTPUT);           //Be careful how relay circuit behave on while resetting or power-cycling your Arduino
    digitalWrite(relay, HIGH);        // Make sure door is locked
  
    //***************************Make sure all leds are off!
    //Protocol Configuration
    Serial.begin(9600);    // Initialize serial communications with PC
    SPI.begin();           // MFRC522 Hardware uses SPI protocol
    mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  
    //If you set Antenna Gain to Max it will increase reading distance
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

    matrix.begin(0x70);
  
    Serial.println(F("BWAT Access Control and  Inventory management v0.1"));   // For debugging purposes
    ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

    //Wipe Code if Button Pressed while setup run (powered on) it wipes EEPROM
    if (digitalRead(wipeB) == LOW) {  // when button pressed pin should get low, button connected to ground
        Serial.println(F("Wipe Button Pressed"));
        Serial.println(F("You have 5 seconds to Cancel"));
        Serial.println(F("This will be remove all records and cannot be undone"));
        delay(5000);                        // Give user enough time to cancel operation
        if (digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
            Serial.println(F("Starting Wiping EEPROM"));
            for (int x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
                if (EEPROM.read(x) == 0) {              //If EEPROM address 0
                // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
                }
                else {
                    EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
                }
            }
            Serial.println(F("EEPROM Successfully Wiped"));
            //*****************Visualize a succesfull Wipe
         }
         else {
             Serial.println(F("Wiping Cancelled"));
             //*****************Viz a cancelled Wipe
         }
    }
  
    // Check if master card defined, if not let user choose a master card
    // This also useful to just redefine Master Card
    // You can keep other EEPROM records just write other than 143 to EEPROM address 1
    // EEPROM address 1 should hold magical number which is '143'
    if (EEPROM.read(1) != 143) {      
        Serial.println(F("No Master Card Defined"));
        Serial.println(F("Scan A PICC to Define as Master Card"));
        do {
            successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
            //////////////////////////////////////////////// Visualize Master Card need to be defined
        }
        while (!successRead);                  // Program will not go further while you not get a successful read
        for ( int j = 0; j < 4; j++ ) {        // Loop 4 times
            EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
        }
        EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
        Serial.println(F("Master Card Defined"));
    }
    Serial.println(F("-------------------"));
    Serial.println(F("Master Card's UID"));
    for ( int i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
        masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
        Serial.print(masterCard[i], HEX);
    }
    Serial.println("");
    Serial.println(F("-------------------"));
    Serial.println(F("Everything Ready"));
    Serial.println(F("Waiting PICCs to be scanned"));
    //********************Viz that we are ready to scan cards
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
    do {
        successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
        if (programMode) {
            // ************************Viz for proramming mode
            matrix.clear();
            matrix.drawCircle(3,3, 3, LED_ON);
            matrix.writeDisplay();  // write the changes we just made to the display
        }
        else {
            normalModeOn();
            matrix.clear();
            matrix.drawRect(0,0, 8,8, LED_ON);
            matrix.fillRect(2,2, 4,4, LED_ON);
            matrix.writeDisplay();  // write the changes we just made to the display 
        }
    }
  
    while (!successRead);   //the program will not go further while you not get a successful read
    if (programMode) {
        if ( isMaster(readCard) ) { //If master card scanned again exit program mode
            Serial.println(F("Master Card Scanned"));
            Serial.println(F("Exiting Program Mode"));
            Serial.println(F("-----------------------------"));
            programMode = false;
            return;
        }
        else {
            if ( findID(readCard) ) { // If scanned card is known delete it
                Serial.println(F("I know this PICC, removing..."));
                deleteID(readCard);
                Serial.println("-----------------------------");
            }
            else {                    // If scanned card is not known add it
                Serial.println(F("I do not know this PICC, adding..."));
                writeID(readCard);
                Serial.println(F("-----------------------------"));
            }
        }
    }
    
    else {
        if (isMaster (readCard)) {    // If scanned card's ID matches Master Card's ID enter program mode
            programMode = true;
            Serial.println(F("Hello Master - Entered Program Mode"));
            int count = EEPROM.read(0);   // Read the first Byte of EEPROM that
            Serial.print(F("I have "));     // stores the number of ID's in EEPROM
            Serial.print(count);
            Serial.print(F(" record(s) on EEPROM"));
            Serial.println("");
            Serial.println(F("Scan a PICC to ADD or REMOVE"));
            Serial.println(F("-----------------------------"));
        }
        else {
            if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
                Serial.println(F("Welcome, You shall pass"));
                granted(3000);          // Open the door lock for 300 ms
            }
            else {      // If not, show that the ID was not valid
                Serial.println(F("You shall not pass"));
                denied();
            }
        }
    }
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted (int setDelay) {  
    //******************Viz for opening door
    matrix.clear();
    matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();     
    digitalWrite(relay, LOW);     // Unlock door!
    delay(setDelay);          // Hold door lock open for given seconds
    digitalWrite(relay, HIGH);    // Relock door
    delay(1000);
            // Hold green LED on for a second
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
    ////////////////////////////////////////Denied Viz
    matrix.clear();
    matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(1000);
}


///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
int getID() {
    // Getting ready for Reading PICCs
    if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
        return 0;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
        return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    while(true);  // do not go further
  }
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
//void cycleLeds() {
//   lights(2,YELLOW);//***********************VIz for cycling LEDs, show activity or for programming mode
//}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
    matrix.clear();
    matrix.drawRect(0,0, 8,8, LED_ON);
    matrix.fillRect(2,2, 4,4, LED_ON);
    matrix.writeDisplay();  // write the changes we just made to the display
  
    digitalWrite(relay, HIGH);    // Make sure Door is Locked
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( int number ) {
    int start = (number * 4 ) + 2;    // Figure out starting position
    for ( int i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
        storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
    }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
    if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
        int num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
        int start = ( num * 4 ) + 6;  // Figure out where the next slot starts
        num++;                // Increment the counter by one
        EEPROM.write( 0, num );     // Write the new count to the counter
        for ( int j = 0; j < 4; j++ ) {   // Loop 4 times
            EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
        }
        successWrite();
        Serial.println(F("Succesfully added ID record to EEPROM"));
    }
    else {
        failedWrite();
        Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
    }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
    if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
        failedWrite();      // If not
        Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
    }
    else {
        int num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
        int slot;       // Figure out the slot number of the card
        int start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
        int looping;    // The number of times the loop repeats
        int j;
        int count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
        slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
        start = (slot * 4) + 2;
        looping = ((num - slot) * 4);
        num--;      // Decrement the counter by one
        EEPROM.write( 0, num );   // Write the new count to the counter
        for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
            EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
        }
        for ( int k = 0; k < 4; k++ ) {         // Shifting loop
            EEPROM.write( start + j + k, 0);
        }
        successDelete();
        Serial.println(F("Succesfully removed ID record from EEPROM"));
    }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
    if ( a[0] != NULL ) {     // Make sure there is something in the array first
        match = true;       // Assume they match at first
    }
    for ( int k = 0; k < 4; k++ ) {   // Loop 4 times
        if ( a[k] != b[k] ) {   // IF a != b then set match = false, one fails, all fail
            match = false;
        }
    }
    if ( match ) {      // Check to see if if match is still true
        return true;      // Return true
    }
    else  {
       return false;      // Return false
    }
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
int findIDSLOT( byte find[] ) {
    int count = EEPROM.read(0);       // Read the first Byte of EEPROM that
    for ( int i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
        readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
        if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
        // is the same as the find[] ID card passed
        return i;         // The slot number of the card
        break;          // Stop looking we found it
        }
    }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID( byte find[] ) {
    int count = EEPROM.read(0);     // Read the first Byte of EEPROM that
    for ( int i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
        readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
        if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
            return true;
            break;  // Stop looking we found it
        }
        else {    // If not, return false
        }
    }
    return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
    matrix.clear();
    matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(1000);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
    matrix.clear();
    matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(1000);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
    matrix.clear();
    matrix.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(1000);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
boolean isMaster( byte test[] ) {
    if (checkTwo (test, masterCard )) {
        return true;
    }
    else {
        return false;
    }
}       
