# BWAT
##BWAT  community sharing containers 

NFC controlled access and inventory management
Forked from [Sensorica NFC Doorlock](https://github.com/Sensorica/NFC_doorlock)
Firmware for Arduino Nano

The first prototype was built during the [eco2fest](eco2fest.com)

###Components

* Arduino Nano 328P
* 12VDC door striker Note: Must be "fail secure"
* MFRC-522 reader/writer 
* 5V relay breakout board
* 8x8 LED matrix with I2C backpack

###Installation:

####NFC Reader to Arduino Nano diagram:
  
     3.3v --- 3.3v Pin
  SDA(SS) --- Pin 10 (Configurable)
      SCK --- Pin 13 / ICSP-3
     MOSI --- Pin 11 / ICSP-4
     MISO --- Pin 12 / ICSP-1
      GND --- GND Pin
      RST --- Pin 9 (Configurable)
	  
#### Relay breakout

       5v --- 5v Pin
      GND --- GND Pin
   Signal --- Pin 4
   
   
#### 8x8 LED matrix:

       5v (red) --- 5v Pin
    GND (black) --- GND Pin
    CLK (white) --- Pin A5
     DAT (gray) --- Pin A4
     
    
  
  
#### Dependencies 

- Mifare RC522 device library
- Adafruit LED Backpack library
