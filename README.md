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
1. [Getting Started](#getting-started)
	1. [Dependencies](#dependencies)
	1. [Building](#building)
	1. [Installation](#installation)
	1. [Usage](#usage)
1. [Release Process](#release-process)
	1. [Versioning](#versioning)
	1. [How to Get Help](#how-to-get-help)
1. [Contributing](#contributing)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgments](#acknowledgements)

## About the Project


* Initial testing done on J66 model jet engine only 
* Currently uses only two parameters from engine 1- Exhaust Gas Temperature and 2- Engine RPM
* EGT is measured using a K-type Thermocouple
* Engine RPM is measured using a magnet in spinner nut and a hall sensor to detect engine RPM 



**[Back to top](#table-of-contents)**

## Project Status

Although based on Arduino framework, current Arduino code uses some libraries which are specific to ESP32 and will not readily compile for other platforms. 


**[Back to top](#table-of-contents)**

## Getting Started

This section should provide instructions for other developers to

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Dependencies

#include <soc/pcnt_struct.h>
#include <driver/pcnt.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include "RunningAverage.h"
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <MAX31855.h>
#include <ESP32Servo.h> 
#include <EasyButton.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>

### Getting the Source

Still in progress. A working version will be shared soon

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
Current Release is Rev 7


**[Back to top](#table-of-contents)**

## How to Get Help

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

https://randomnerdtutorials.com/esp32-web-server-gauges/
https://www.arduinoslovakia.eu/?lang=en
https://canvas-gauges.com/



**[Back to top](#table-of-contents)**
