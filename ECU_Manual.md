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


* Model Jet engines are complex mechanical machines which require high precision and specific material properties to operate under extreme temperature and RPM.
Electronically however they are very simple machines. Primarily two sensors tell the state of the engine i.e. 
1- RPM Sensor and 
2- Temperature sensor 

Similarly, for controlling a model Jet engine only a few control parameters are required
1- Fuel Flow control
2- Gas flow for starting the engine on gas (propane, butane or a mix)
3- Glow plug on/off control to ignite starting gas
4- Starter motor control to provide initial RPM to engine until it reaches self sustaining RPM.
* This electronic simplicity makes it easy to develop an ECU (Engine Control Unit) using commonly available microcontrollers such as ESP32 and a few sensors. Development can further be simplified by using Arduino environment which is much more user friendly.
* ESP32 Dev Kit V1 is initially used for development. The electronics schematic is available on github
* Initial testing done on J66 model jet engine only 
* Currently uses only two parameters from engine 1- Exhaust Gas Temperature and 2- Engine RPM
* EGT is measured using a K-type Thermocouple
* Engine RPM is measured using a magnet in spinner nut and a hall sensor to detect engine RPM 



**[Back to top](#table-of-contents)**

## Project Status

Although based on Arduino framework, current Arduino code uses some libraries which are specific to ESP32 and will not readily compile for other platforms.
A WiFi webserver is now added to ECU firmware. It starts on power up and provides four pages output. 
WiFi SSID is "Ardu_ECU" and password is "Admin123"
Pages can be loaded by typing IP address in web browser (192.168.4.1)
Also, a DNS is builtin and you can use any web address as long as it is not a persistent IP. e.g. you can use y.com , x.com or any thing similar and it will take you to Page1 of web server
Page 1 is used to set Engine Parameters
Page 2-4 display various control and state parameters and act as a data terminal



**[Back to top](#table-of-contents)**

## Getting Started

Before compiling Arduino code, make sure tht you have latest updates of all the libraries especially for ESP32.

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

ECU has three modes which can be selected by double clicking the button. On each doubleclick, the next mode will be selected. The three modes selectable are 
1- Trial mode Starter control
2- Trial mode Fuel pump control and
3- Normal mode

Details of these modes are as below; 

1- Trial mode
    Starter Control
    Fuel Pump Control
2- Normal mode
    Waiting
        Starter control
        Fuel Pump control
    Starting
        StartStage0
        StartStage1
    Idle
    Operating
    Cooldown

## Trial mode
### Trial mode starter control (most dangerous mode- use with caution)
In this mode the throttle stick will control the starter RPM. The mode switch will allow to switch on/off the gas flow. Glow plug will be switched on as per settings (glowOnRPM).
Fuel flow will be as per trialModeFuelOnRPM and trialModeFuelFlow . Fuel will automatically start at FuelOnRPM and will be increased by ECU to a maximum value of trialModeFuelFlow.
This mode is designed to identify self sustaining RPM therefore trial mode fuel flow limit should be kept as low as possible. Once the RPM reaches self sustaining, the ECU will keep increasing fuel flow until it reaches trialModeFuelFlow. This is what makes this mode dangerous as throttle is only controlling starter. Once the engine is self sustaining, throttle has no control over engine RPM and engine can accelerate to dangerously high RPM. The engine can however be shut down using mode switch. 

### Trial mode fuel pump control 
In this mode the throttle stick will control the fuel pump.Fuel will start at FuelOnRPM and will be increased to a maximum value of trialModeFuelFlow at 100% throttle. The mode switch will start the test sequence which will be as follows. 
RPM will cycle between ignitionRPMLow and ignitionRPMHigh while Glow plug and Gas Flow will be switched on as per settings.
Once the ignition is detected (exhaust temperature above limit 100degC ) Starter will increase RPM once temperature starts going above startingTemperature (700degC defult). In effect starter will operator in temperature chase mode i.e. starter will try to keep temperature at startingTemperature value. If temperature falls below this value, starter will slow down to reduce cooling effect. If exhaust temperature increases above this value due to gas flow or adding more fuel flow, the starter will accelerate thus increasing RPM. This keeps on going untill starter reaches 100% RPM. This has proven to be reliable starting method, the fuel can easily be ignited in combustion chamber as the temperature is not allowed to drop below starting temperature.
Once at 100% starter will remain at full power until engine RPM reaches preset value of starterOffRPM which should be set well above self sustain RPM. Once RPM is above starterOffRPM, starter will switch off and bendix clutch should disengage. Engine is now running on fuel only or fuel plus Gas depending where gasOffRPM is set.

## Normal mode
### Waiting
Mode switch in this mode will start the startup sequence or abort operation
Abort will switch off gas, fuel, starter and glow plug in effect shutting down the engine immediately.
 #### Waiting- Starter Control
 Default mode at startup. With mode switch in off(Abort) position, throttle control will control starter. This is setup so that throttle can be used to clear engine of any excess fuel, gasses or just test the free running of engine
Switching the mode switch to on position will start the startup sequence
 #### Waiting- Fuel Pump Control
 Sub mode of waiting mode.With mode switch in off(Abort) position, throttle control will control fuel pump. This mode is used to prime the fuel pump
 Switching the mode switch to on position will start the startup sequence

## Starting
Starting mode is selected from mode switch signal. It consists of two sub stages which are automatically selected starting with StartStage0. This stage ends when engine has reached idleRPM. During starting, throttle stick should be kept at low or minimum position. This is one of the conditions of starting. After starting the control will be handed over to throttle with idle RPM as the running value at zero throttle. 

### StartStage0
In this stage starter motor is run for a given time to purge the engine of any residual gases or fuel. The purgeTime and purgeThrottle define the two values used during this stage
### StartStage1
RPM will cycle between ignitionRPMLow and ignitionRPMHigh while Glow plug and Gas Flow will be switched on as per settings.
Once the ignition is detected (exhaust temperature above limit 100degC ) Starter will increase RPM once temperature starts going above startingTemperature (700degC defult). In effect starter will operator in temperature chase mode i.e. starter will try to keep temperature at startingTemperature value. If temperature falls below this value, starter will slow down to reduce cooling effect. If exhaust temperature increases above this value due to gas flow or adding more fuel flow, the starter will accelerate thus increasing RPM. This keeps on going untill starter reaches 100% RPM. This has proven to be reliable starting method, the fuel can easily be ignited in combustion chamber as the temperature is not allowed to drop below starting temperature.
at 100% starter power the fuel flow is now increased to reach idle RPM. fuel flow will stop increasing if exhaust temperature goes above max Temperature limit.This condition can happen if starter at 100% power is not able to bring engine to self sustaining RPM. 
Once at 100% starter will remain at full power until engine RPM reaches preset value of starterOffRPM which should be set well above self sustain RPM. Once RPM is above starterOffRPM, starter will switch off and bendix clutch should disengage. Engine is now running on fuel only or fuel plus Gas depending where gasOffRPM is set.
The fuel flow will keep increasing if temperature stays below maxTemp limit. until the RPM reaches idle RPM. Once idle RPM is reached ECU will switch to idle mode and maintain RPM at idle RPM value.
## Brushless Vs Brushed DC motor Starting
Initially we started using a brushed DC motor for starting. The starter control logic increases the engine RPM while increasing the fuel flow AND keeping the temperature below Starting Temperature setpoint Or MaxTemp Setpoint( if starter has maxed out) .
Starter is switched off at StarterOffRPM and the engine accelerates with fuel to idle RPM. For KJ 66 engine the self sustaining RPM is above 20000 RPM with good bearings and can be higher if the bearings are not running smooth. This means that the starter motor has to push the RPM upto 20000 or higher before engine can accelerate on fuel only. 
During testing we burned quiet a few DC motors and it became apparent that to get a DC motor in a 380 frame which can accelerate the engine to 20000 RPM is not easy. There were many motors available but they did not last many cycles, specially when engine is in testing and does not start right away. Either the motor would burn out or the bush were wearing out at high RPM's very quickly. After 3-4 failed startup attempts the motor had to be changed. This should be less of  problem with an engine where parameters are properly setup and engine starts within a few seconds and motor disengages.


Our next attempt was to try a brushless motor for startup.
Looking at very popular 2212 brushless motor, the motor shaft is very sturdy  3.17mm vs 2 mm for brushed motors. The motor has ball bearings vs bushes in brushed dc motors And the biggest difference of all i.e. in a small frame a 2700kv motor should deliver 27000 RPM with a 10 volt battery. Well above self sustaining RPM. However, as soon as we started testing, we found that our RPM measurement was very erratic and thus based on RPM criteria the motor was shutting down too early.
i.e. RPM sensor was reading higher than actual RPM. We theorised that due to rotation of magnets in the outrunner brushless motor, the RPM signal was being corrupted so we are not able to get rid of this interference. Instead of shutting the motor down on reference RPM it was decided to shut it down on reference throttle e.g. we gradually increase the throttle to 50% while increasing fuel flow and as soon as 50% throttle is reached we shut down the starter and accelerate on fuel flow . This required a hit and trial method to see at what throttle should we shut down the motor as we didnt want motor to run more than what is absolutely required but we also needed engine RPM to be above self sustaining RPM.As fuel flow is increased, the actual power required by motor is reduced drastically. 
In our test we have been able to achieve self sustaining RPM with only 20% throttle.
This logic was implemented using StarterLimit variable. If StarterLimit has a value greater than zero then startup logic will shut down the starter motor at StartupLimit and StarterOffRPM will be ignored, if StartupLimit is zero, the brushless motor will shutdown at StarterOffRPM. This also means that both of these methods can be used for brushed or brushless motor if other users donot face the same problem.

## Idling
In this mode ECU will maintain engine at idleRPM. Throttle stick at zero will maintain idleRPM. Increasing throttle will increase RPM. Once RPM increases above 3000 + idleRPM, the mode changes to operating mode. At idle RPM if mode switch is switched off, ECU starts the process of shutting down the engine by entering cooldown mode 

## Operating
Once engine RPM is above 3000+idleRPM, the ECU enters operating mode. The throttle stick controls the RPM upto a limit of maxRPM or maxTemp whichever comes first

## Cooldown
Cooldown mode is entered from idle mode by switching off the mode switch. In cool down mode, the fuel is turned off and once RPM reaches below 12000 the engine is run by starter until temperature drps below ignition temperature.


**[Back to top](#table-of-contents)**

## Problems encountered during development (Gotchas)

### Fuel Pump Priming
## The fuel pump has to be primed so that there is fuel line from fuel tank to the fuel pump is full of fuel and all air is expelled.
The startup of the engine requires a careful balance of fuel flow and starter RPM so engine can accelerate to self sustaining RPM. Proper priming of fuel pump is absolutely essential to achieve a good startup and in avoiding potential damage to engine. The firmware makes no assumption about the type of fuel pump, the delivery rate of fuel and the battery of fuel pump etc. The firmware detects the pumping of fuel through an increase in exhaust temperature and if temperature is not increasing, it keeps increasing the speed of fuel pump until it reaches maximum value.
## Scenario 1 (Fuel line is empty and pump has not been primed)
Firmware will keep increasing the fuel pump speed  and will reach a scenario where as soon as fuel ines are filled and fuel reaches the fuel pump. It will dump massive amount of fuel into the engine. This will result in sudden acceleration and temperature rise which will be uncontrollable as the fuel is already in the engine and shutting down throttle will not stop the engine
## Scenario 2 (During Priming the pump excess fuel has been pumped into the engine)
In this scenario as soon as the fuel is ignited, it will accelerate the engine uncontrollably. Temperature and/or RPM can rise above engine limit and engine can get damaged.
## Bottom Line is that if excess fuel has reached the engine and has been ignited an uncontrolled runaway will happen untill all the fuel is burned and/or engine is damaged
### Engine mounting
Most of the engines are constructed so that the casing cylinder forms seal at compressor diffuser and nozzle guide vane ring. When the engine runs, there is an increase in pressure inside the engine. This increased pressure is required to run the engine. These engine casing cylinders are usually light weight metal cans.
If a generic clamp type engine mounting is used and tightened excessively around the engine body, it can distort the engine casing and cause a leakage through gas seal.If the leakage happens, engine will either sustain at higher RPM or fail to run entirely. 
Depending on engine design a generic clamp (if used) should be placed so that an increase in OD is restricting the engine movement out of clamp as well.
### Starter Motor
Starter motor needs to be able to drive engine to self sustaining RPM.For J66 engine  Speed 400 or equivalent motor with minimum capability to drive engine to around 20000 RPM in cold condition is required. With this type of motor and fuel and/or gas burning, the engine will be able to reach self sustaining RPM. If the motor cannot deliver such RPM, engine will not start and may even get damaged. 
For other engines a different type of motor may be required however the principle is the same. If the motor cannot bring engine to self sustaining RPM, the engine will not start
 ### Glow plug Driver for gas ignition
 A remote glow plug driver usually operates on high PWM frequency to deliver correct voltage to glow plug. This high frequency can travel through engine body and affect thermocouple which is also in contact with engine body thus causing erratic reading.
 The easiest way in a test setup was to mount the thermocouple aft of exhaust nozzle without touching.

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
