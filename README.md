# BWAT
## community sharing containers 

NFC controlled access and inventory management
Forked from [Sensorica NFC Doorlock](https://github.com/Sensorica/NFC_doorlock)
Firmware for Arduino Nano

The first prototype was built during the [eco2fest](eco2fest.com)

### Components

* [Arduino Nano 328P](https://www.arduino.cc/en/Main/ArduinoBoardNano)
* [12VDC door striker Note: Must be "fail secure"](https://abra-electronics.com/electromechanical/solenoids/sol-door-12v-12-vdc-cabinet-door-style-solenoid.html)
* [MFRC-522 reader/writer](http://www.nxp.com/documents/data_sheet/MFRC522.pdf)
* [5V relay breakout board](https://abra-electronics.com/electromechanical/relays/relay-modules-shields/rm-1-arduino-relay-module.html)
* [8x8 LED matrix with I2C backpack](https://learn.adafruit.com/adafruit-led-backpack/0-8-8x8-matrix)

### Installation

#### NFC Reader to Arduino Nano diagram

|    NFC  | Arduino             |
|---------|---------------------|
|    3.3v | 3.3v Pin            |
| SDA(SS) | Pin 10 Configurable |
|     SCK | Pin 13              |
|    MOSI | Pin 11              |
|    MISO | Pin 12              |
|     GND | GND Pin             |
|     RST | Pin 9 Configurable  |
	  
#### Relay breakout

|  Relay  | Arduino |
|---------|---------|
|      5v | 5v Pin  |
|     GND | GND Pin |
|  Signal | Pin 4   |
   
   
#### 8x8 LED matrix

|  Matrix  |  Arduino  |
|----------|-----------|
|   5v     | 5v Pin    |
|  GND     | GND Pin   |
|  CLK     | Pin A5    |
|  DAT     | Pin A4    |

  
#### Dependencies 

- [Arduino MFRC-522 device library](https://github.com/miguelbalboa/rfid)
- [Adafruit LED Backpack library](https://github.com/adafruit/Adafruit-LED-Backpack-Library)
- [Adafruit GFX library](https://github.com/adafruit/Adafruit-LED-Backpack-Library)
