# Earth Frame Orientation Sensor

This program is a set of two arduino firmwares that allows to vizualise remotely (through ethernet) the orientation of a frame.
The first arduino (the "base") is an arduino MEGA linked to a screen and an ethernet shield, it will receive data from the second arduino.
The second arduino (the "imu") is an arduino UNO linked to an IMU sensor and an ethernet shield, it will send data to the base.

## Prerequisites

### Hardware

* 1x arduino mega
* 1x arduino uno
* 2x arduino ethernet shield 2
* 1x adafruit BNO055 imu
* 1x T6963C screen with 240x128 resolution
* 1x pulled-down switch

### Software

You will need the arduino IDE with the following libraries (they are included in the IDE library manager):

* Adafruit BNO055
* Adafruit Unified Sensor
* Ethernet2
* U8g2

## How-to

* Compile and flash the "base.ino" project into the arduino mega
* Compile and flash the "imu.ino" project into the arduino uno
* Put both ethernet shields on the arduinos
* For the base wiring, see T6963C_240x128.h file and connect the switch output to the pin 49
* For the imu wiring, see [adafruit BNO055 documentation](https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor/pinouts)
* Connect the base and the imu with an ethernet *crossover* cable
* Properly power both arduinos

## Author

* **François Kneib** - *Initial work*

