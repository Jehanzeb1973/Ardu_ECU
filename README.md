# Ardu_ECU- Arduino based ECU for model Jet Engines

Easy to build and use Engine Control Unit (ECU) for model Jet engines :

* Based on Arduino code and running on ESP32 based hardware
* ECU configuration using a simpe web browser
* Engine operating parameters displayed using a web browser interface
* Use off the shelf components for electronics e.g.
        Brushed or brushless Speed controller for starter motor and fuel pump control
        MOSFET module for gas valve solenoid control
        Glow plug driver module for providing power to glow plug.

## Table of Contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
2. [Project Videos](#project-videos)
3. [Getting Started](#getting-started)
	1. [Dependencies](#dependencies)
	1. [Building](#building)
	1. [Installation](#installation)
	1. [Usage](#usage)
4. [Release Process](#release-process)
	1. [Versioning](#versioning)
	1. [How to Get Help](#how-to-get-help)
5. [Contributing](#contributing)
6. [License](#license)
7. [Authors](#authors)
8. [Acknowledgments](#acknowledgements)

## About the Project


* Initial testing done on J66 model jet engine only 
* Currently uses only two parameters from engine 1- Exhaust Gas Temperature and 2- Engine RPM
* EGT is measured using a K-type Thermocouple
* Engine RPM is measured using a magnet in spinner nut and a hall sensor to detect engine RPM 



**[Back to top](#table-of-contents)**

## Project Status

Although based on Arduino framework, current Arduino code uses some libraries which are specific to ESP32 and will not readily compile for other platforms. 

### Release Version:
* Current ECU release version is Rev 11. This version has been tested with gas start engine and is working fine with no known issues.

### Test Version:
* Current test version is ECU Rev12TC10. This is still under test with not all code fully tested. Being released for community to test and release.

####New Features include:
* Kero Start --Tested and working
* Ability to save and load configuration files -- Tested and working
* Realtime Serial data logging -- Tested and working
* Ability to read pressure sensor --Tested and working
* Ability to read load cell --Tested but has errors
* New starting methods included are open loop throttle start and pressure sensor start --not tested



**[Back to top](#table-of-contents)**

## Project Videos
* 1-Project concept 	https://youtu.be/Xl8DUmeqQWw
* 2-Project setup 	https://youtu.be/wCetENek3wA
* 3-Testing Firmware 	https://youtu.be/1cKAqnF0G1I
* 4-New user interface 	https://youtu.be/sQsggVR1LOk
* 5-Multiple Test Runs  https://youtu.be/5c6H9qnOwm4
* 6-High acceleration	https://youtu.be/lYJ67fU-_sc
* 7-User interface	https://youtu.be/T5aCW-EMLBw
* 8-Final Release Test	https://youtu.be/rOXXS8oFhO8
* 9-Thrust Test 	https://youtu.be/VEnOmlrnRFM
* 10-ECU Rev11 Release  https://youtu.be/CBFElKNjws0
### Dependencies

* #include <soc/pcnt_struct.h>
* #include <driver/pcnt.h>
* #include <Wire.h>
* #include <U8g2lib.h>
* #include <Preferences.h>
* #include "RunningAverage.h"
* #include <FS.h>
* #include <SPI.h>
* #include <SD.h>
* #include <MAX31855.h>//url = https://github.com/enjoyneering/MAX31855
* #include <ESP32Servo.h> 
* #include <EasyButton.h>
* #include <AsyncTCP.h>
* #include <ESPAsyncWebServer.h>
* #include <DNSServer.h>
* #include <WiFi.h>
* #include <ArduinoJson.h>

### Getting the Source

ECU-Rev11  is the latest firmware 

### Installation

Upload to ESP32 Devkit V1 using Arduino IDE

### Usage

Each release will refer to hardware and software configuration for usage.
Current code is compiled on Arduino 1.8.19 and is running on ESP32 Devkit V1.
The electronic schemetic is included in repository

**[Back to top](#table-of-contents)**

## Release Process

Release will require testing on an actual engine. 
Test conditions and results are to be shared with the release version

### Versioning

We are using whole number versioning. We will not do a major or minor release
Current Release is Rev 11

## ECU Rev 10 Notes
Added a slider control on Page1 to switch off WebServer. This is helpful in actual flying jets as you dont want a transmitter next to your RC receiver. Only WiFi is switched off, other functiond work as normal
Software corrections-DynamicJsonDoc moved from global to local- Removed OLED Displays, Updated Page 4 for battery voltage and maxLoopTime- Added Page 6 to track engine and ECU lifetime usage

**[Back to top](#table-of-contents)**

## ECU Rev 11 Notes

* Major hardware redesign
* Changed Pin assignments and added 4 wire SD card capability
* SD Card functions running on Core 0 while Main loop is running on Core 1.
* Changed data range to 0-1000
* At each power up two files are written to SD Card. 
        **  1) Settings file with all the settings at power up and
        **  2) Data file with engine operating data from power up till the ECU is powered off
* Added sd loop time and data file name info on Page 5.
* Added buttons on page 5 to reset error,manage files and change ECU modes.
* Added page 7 to be able to download and delete data files
* Added Fuel Solenoid capability. Added Two LED's to Show WiFi State and SD Recording state.SD card pinout as below
          http://3.bp.blogspot.com/_8JZhVVmpICU/TH_Pxa19MHI/AAAAAAAAApg/pgSppwx0gY8/s1600/SD+card+pinout.jpg
* Added ability to change ECU Webserver WiFi SSID and Password on page 2
* Added option for Magnetic (1 Pulse per revolution)  or Optical (2 pulses per revolution)  RPM sensor 
* Implemented tempGradient check to limit sharp rise in exhaust temperature (tempGrad and maxTempGrad variables)
* Implemented lookahead for temperature and stop increasing fuel flow if exhaust temp will increase above maxTemp in 3 sec (tempGrad x 3)



**[Back to top](#table-of-contents)**

## Contact Us

For any help, suggestions, proposals you can send me an email at jehanzeb@digipak.org


## Contributing

All contributions are welcome. If you want to share your version on this repository or share a link to your version, we will be glad to do so. We have limited resources to review other peoples work so any contribution will be taken as is and not reviewed. 

**[Back to top](#table-of-contents)**



## License

Copyright (c) 2022 Jehanzeb Khan

This project is licensed under the GNU AGPLv3 License - see [LICENSE.md](LICENSE.md) file for details.

**[Back to top](#table-of-contents)**

## Authors

* **[Jehanzeb Khan](https://github.com/Jehanzeb1973)** - *Initial work* - [DigiPak](https://digipak.org/)



**[Back to top](#table-of-contents)**

## Acknowledgments

* https://randomnerdtutorials.com/esp32-web-server-gauges/
* https://www.arduinoslovakia.eu/?lang=en
* https://canvas-gauges.com/



**[Back to top](#table-of-contents)**
