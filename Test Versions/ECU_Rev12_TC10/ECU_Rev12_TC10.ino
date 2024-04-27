/*
//Rev3 - Added Trial modes trialModeStarter and trialModeFuelPump to determine the best settings for Final Setup
//Rev4 - use pulse counter module intead or interrupt to measure turbine RPM
//Rev5 - pulse counter code updated
//Rev6 - Webserver based data terminal functionality added
//Rev7 - Cleaned up the code and added 2 more data screens in webserver (Page3&4)
//Rev8 - changed output control levels to (outMin-outMax) instead of (0-100) previously.Added data screen 5 for abort parameters
//Rev9 - Added Temperature tolerance for Trial mode Fuel Pump Control- Added support for brushless motor by introducing starterLimit
          when "starterLimit" > 0,  "starterLimit" will be used to determine when the starter motor stops and when gas valve is closed
          when "starterLimit"==0, starter motor and gas valve cutoff will be according to defined RPM in the settings
          This is done because we found that magnetic RPM sensing is not correct due to brushless motor EM noise.
          The issue is not present when a DC brushed motor is used
//Rev10 - DynamicJsonDoc moved from global to local- Removed OLED Displays, Updated Page 4 for battery voltage and maxLoopTime 
//Rev11 - Changed Pin assignments and added 4 wire SD card capability, SD Card functions running on Core 0 while Main loop is running on Core 1.
          Changed data range to 0-1000
          At each power up two files are written to SD Card. 
          1) Settings file with all the settings at power up and
          2) Data file with engine operating data from power up till the ECU is powered off
          Added sd loop time and data file name info on Page 4.
          Added buttons on page 4 to reset error,manage files and change ECU modes. Added page 7 to be able to download and delete data files
          Added Fuel Solenoid capability. Added Two LED's to Show WiFi State and SD Recording state.SD card pinout as below
          http://3.bp.blogspot.com/_8JZhVVmpICU/TH_Pxa19MHI/AAAAAAAAApg/pgSppwx0gY8/s1600/SD+card+pinout.jpg
          Added ability to change ECU Webserver WiFi SSID and Password on page 1
          Added option for Magnetic (1 Pulse per revolution)  or Optical (2 pulses per revolution)  RPM sensor 
          implemented tempGradient check to limit sharp rise in exhaust temperature (tempGrad and maxTempGrad variables)
          Also implemented lookahead for temperature and stop increasing fuel flow if exhaust temp will increase above maxTemp in 3 sec (tempGrad x 3)

 //Rev12 - Changes
          1. Change the html button color based on what page you are on.
          2. Load parameters from File option on page 9
          3. Kero start imlemenred and is working. thanks to @dstromeng for testing
          4. Verify engine run time value
          5. Measure pressure and pressure sensor calibration
          6. Trial Modes have been removed
          7. Re organized the menu pages to be more logical
          8. Push Button functionality removed. All options can be selected from web interface
          9. LED2 functionality removed as this pin is now used for load cell
          10. Control methods have been divided into RPM control, Pressure control and Throttle Control. The Temperature sensor is essential 
              RPM and pressure sensor are now optional although if these are not present, fault checks will not be available.Engine can now be
              started and operated only on open loop fuel pump/Throttle control Or RPM feedback based control Or Pressure sensor based feedback control
              If RPM sensor is not available or not used, maxRPM cannot be detected and engine can exceed its rated RPM
              Similarly if pressure sensor is not used, maxPressure will not be detected. In model jet engines maxRPM is a bigger concern as the engine 
              pressure ratio is not high and damage due to exceeding pressure is not a major concern
          11. html buttons for testing Gas Solenoid, Fuel Solenoid and Glow plug
          12. Load Cell measurement capability to measure thrust
          13. Added preheat delay. This is the time delay after ignition has been achieved. This allows the combustion chamber to heat up for successfull ignition once the fuel is pumped in
        
 Ardu-ECU
 ESP32 based ECU for model jet engines
/////////////////////////////////////////////////////////////////////////////////////////////////////////
 Disclaimer:
 Jet engine is not a toy and ECU control of a jet engine presents inherent risk.Any programming error, selection 
 of wrong parameters or component failure can cause damage to jet engine or other equipment and serious injury or
 death to people . I take no responsibility for any consequences for using this software. 
 Any person using this software uses it at their own risk and is responsible for all consequences intentional or otherwise.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
 Jehanzeb Khan
 Jehanzeb@digipak.org

 //Fault Handling
 Startup sequence - No ignition after gas on for given time--Handled with AbortAll()
 Startup sequence- No RPM increase with fuel pump running--handled with AbortAll()
 RPMAvg> Max RPM - Handled by reducing Fuel Flow
 exTemp>maxTemp - Handled by reducing Fuel Flow
 Flameout during operation - Handled with AbortAll()
 No RC Signal - Handled with AbortAll()
 No RPM Signal Handled with AbortAll()

//Startup Sequence

Note: All settings in this sequence are changeable through web interface
1- Purge for x seconds at xx% Throttle to clear any combustible gases prior to starting 
2- Switch on Glow Plug and cycle between Ignition Low and High RPM  with gas valve open until ignition is achieved or timeout is reached
3- Confirm ignition if exhaust temperature crosses a limit(100 deg C default)
4- After ignition increase starter motor power as well as Fuel Flow. Chase the starting temperature(500 deg C default)  by increasing fuel flow and starter RPM
5- Switch off Gas, starter motor and glow plug as certain RPM are reached with fuel ( All programmable)
6- Keep increasing RPM with fuel flow until reach idle RPM.
7- Switch to idle mode and follow Throttle Input( maintain Idle RPM if throttle is zero)
8- Switch to operation mode and follow Throttle input

*/

// Libraries
#include <soc/pcnt_struct.h>
#include <driver/pcnt.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Preferences.h>//EEPROM library
#include "RunningAverage.h"
#include <FS.h>
#include <SD_MMC.h>
#include "driver/sdmmc_types.h"
#include <MAX31855.h>
#include <ESP32Servo.h> 
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HX711.h> //load cell
#include "page1.h"// html code for parameters screen
#include "page2.h"// html code for parameters screen
#include "page3.h"// html code for parameters screen
#include "page4.h"//html code for realtime data terminal screen 1
#include "page5.h"//html code for realtime data terminal screen 2
#include "page6.h"//html code for realtime data terminal screen 3
#include "page7.h"//Readings at abort time screen
#include "page8.h"//Engine and ECU Lifetime usage
#include "page9.h"//Data file download page




// pins for PCB design with MicroSD


#define LEDPin_1               0
//#define SD-D0                2 //SD pins are used but no need to define them
//#define SD-D1                4 
#define MAXCS                  5 //SPI Thermocouple CS 
//#define SD-D2                12 //SD card 4 wire interface has fixed GPIO on ESP32 . No need to define these
//#define SD-D3                13 //Also when using 4 wire interface, these pins cannot be used for any other purpose
//#define SD-CLK               14 
//#define SD_CMD               15
#define LOADCELL_DOUT_PIN      16 
#define RC_Mode_Pin           17 
#define ThermoSCK            18
#define ThermoSD             19
#define StarterPin           23  
#define LOADCELL_SCK_PIN     25
#define GlowPin              26
#define FuelPumpPin          27
#define Fuel_Solenoid_Pin    32
#define GasPin               33
#define voltsensorPin        34
#define RPMsensorPin         35
#define PressurePin          36  
#define RC_Throttle_Pin      39 



//for storing data to SD card

TaskHandle_t sdTask; //SD write Task to run on core 0 to avoid delay in main code
//SD card File
File file;
int fileseq=0;//Sequence in filename
char filename[32]; //Filename for data storage
char settingsfilename[32];//filename for settings data
char newsettingsfilename[32];//filename for settings data
//downloading and deleting files
String   webpage, MessageLine;
 bool dirpresent=false;//if default data directory is present or not
bool sdavailable=false;
bool writefile=true; //only write file if this variable is true
bool serialheaderprinted=false;
String Data="";//Data string to be logged to SD card
String headerData="";//Header Data string to be logged to SD card
String serialData=""; //Data to be sent by serial port
String newData=""; //Data to be added to sd buffer
String sysMsg="";//system Message


// HX711 
HX711 scale;
float loadcellvalue=0;
bool loadCellEnable=true;

// Calibration constants stored in EEPROM
bool taredone=false;
bool scalingdone=false;
int tareconst=0;
float scaleconst=1;

//counter for RPM
#define PCNT_FREQ_UNIT      PCNT_UNIT_0                      // select ESP32 pulse counter unit 0 (out of 0 to 7 indipendent counting units)
  pcnt_isr_handle_t user_isr_handle = NULL;                    // interrupt handler - not used
  hw_timer_t * timer = NULL;                                   // Instancia do timer                                                              // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/pcnt.html
 
  int16_t PulseCounter =     0;                                // pulse counter, max. value is 65536
  int OverflowCounter =      0;                                // pulse counter overflow counter
  int PCNT_H_LIM_VAL =       10000;                            // upper limit of counting  max. 32767, write +1 to overflow counter, when reached 
  uint16_t PCNT_FILTER_VAL=  1000;                             // filter (damping, inertia) value for avoiding glitches in the count, max. 1023


bool pumpPrime=false;//variable to determine if throttle spins the starter motor or primes the pump in modeWaiting
//DNS Server
DNSServer dnsServer;
//Web server to change settings
AsyncWebServer server(80);
// Create an Event Source on /events using SSE(Server Sent Events) to update data in Data Terminal
AsyncEventSource events("/events");

bool startServer=true;//WiFi server should be started at powerup
bool serverStarted=false;// WiFi server has not started yet
bool eventServer=false;
bool startSerial=true; //Start Serial Transmission

 String ssid     = "Ardu_ECU";
 String password = "admin123";//Pass phrase has to be 8 or more characters for ssid to work
 char* PARAM_ServerState = "state";
 char* PARAM_maxTemp = "maxTemp";                           //Maximum exhaust temperature in degC. Engine will cut throttle if this temperature is exceeded. It will not shut down
 char* PARAM_maxRPM = "maxRPM";                             //Maximum RPM. Engine will cut throttle if this is exceeded, it will not shut down the engine
 char* PARAM_cpr = "cpr";                                     //Counts per revolution based on sensor type
 char* PARAM_idleRPM = "idleRPM";                           //RPM while running at zero throttle. ECU will try to maintain this RPM at zero throttle
 char* PARAM_rpmTolerance = "rpmTolerance";                      //RPM tolerance range, target RPMs will be RPM+ within rpmTolerance range
 char* PARAM_glowOnRPM = "glowOnRPM";                       //RPM at which glow plug will be switched on at startup
 char* PARAM_glowOffRPM = "glowOffRPM";                     //RPM exceeding which the glow plug will be turned off
 char* PARAM_ignitionRPMHigh = "ignitionRPMHigh";           //At start up ECU will run engine between ignitionRPMHIGH and ignitionRPMLOW with gas and glow plug energized to create ignition
 char* PARAM_ignitionRPMLow = "ignitionRPMLow";             //At start up ECU will run engine between ignitionRPMHIGH and ignitionRPMLOW with gas and glow plug energized to create ignition
 char* PARAM_gasOnRPM= "gasOnRPM";                          //RPM at which gas valve will be turned On
 char* PARAM_gasOffRPM = "gasOffRPM";                       //RPM at which gas valve will be turned off
 char* PARAM_starterOffRPM = "starterOffRPM";               //RPM at which starter motor will be turned off
 char* PARAM_fuelOnRPM = "fuelOnRPM";                       //RPM at which fuel pump will be switched on
 char* PARAM_pumpOnValue = "pumpOnValue";                  //Initial value of fuel pump as fuel pumps donot start pumping until a certain input value
 char* PARAM_purgeTime = "purgeTime";                       //Time for which engine will run at purge throttle without fuel or gas to clear the engine at begining
 char* PARAM_purgeThrottle = "purgeThrottle";               //Throttle for purge operation in begining
 char* PARAM_starterIncDelay = "starterIncDelay";           //Delay built into starter control to limit the rate of change of starter RPM
 char* PARAM_accelerationDelay = "accelerationDelay";       //Delay built in to limit fuel flow increase to avoid dumping too much fuel into engine
 char* PARAM_decelerationDelay = "decelerationDelay";       //Delay built in to limit fuel flow decrease to avoid flameout by starving the engine
 char* PARAM_lowDecelDelay = "lowDecelDelay";       //Delay built in to limit fuel flow decrease to avoid flameout by starving the engine
 char* PARAM_lowAccelDelay = "lowAccelDelay";       //Delay built in to limit fuel flow decrease to avoid flameout by starving the engine
 char* PARAM_ignitionThreshold = "ignitionThreshold";       //Temperture above which ignition will be detected
 char* PARAM_noIgnitionThreshold="noIgnitionThreshold";     //Time in milliseconds after which startup sequence will be aborted due to no ignition
 char* PARAM_fuelFlowDetectionTime="fuelFlowDetectionTime"; //Time in which engine should reach a certain RPM after starting fuel pump (Startup only)
 char* PARAM_fuelFlowDetectionRPM="fuelFlowDetectionRPM";   //RPM to reach to confirm that engine is running on fuel and not on gas only
 char* PARAM_trialModeFuelOnRPM="trialModeFuelOnRPM";   //RPM to reach to when fuel will be switched on for lubricating the bearings
 char* PARAM_trialModeFuelFlow="trialModeFuelFlow";     //Fuel Flow to pump in Starter Only test mode to lubricate bearings
 char* PARAM_startingTemp="startingTemp";                  //maximum temperature for starting phase
 char* PARAM_maxTempGrad="maxTempGrad";                  //maximum temperature for starting phase
 char* PARAM_maxMotorVolt="maxMotorVolt";                  //maximum motor voltage. this allows motors with less voltage limit to be used with bigger battery packs
 char* PARAM_maxPumpVolt="maxPumpVolt";                  //maximum fuel pump voltage. this allows fuel pumps with less voltage limit to be used with bigger battery packs
 char* PARAM_MIN_MICROS = "MIN_MICROS";              //minimum microseconds for servo control
 char* PARAM_MAX_MICROS = "MAX_MICROS";                //maximum microseconds for servo control
 char* PARAM_voltIn = "voltIn";                //Input voltage
 char* PARAM_casePress = "casePress";                //Engine Case Pressure
 char* PARAM_WiFiSSID = "WiFiSSID";                //Wifi Name/SSID
 char* PARAM_WiFiPWD = "WiFiPWD";                //WiFi Password
 char* PARAM_keroPower = "keroPower";                //kerostart plug power
 char* PARAM_keroTime = "keroTime";                //kero plug heating time before pumping fuel
 char* PARAM_keroValve = "keroValve";                //kerostart solenoid valve duty cycle
 char* PARAM_keroFreq = "keroFreq";                //kerostart solenoid valve Frequency 
 char* PARAM_keroPump = "keroPump";                //kerostart Fuel pump setting
 char* PARAM_idlePress = "idlePress";                //Pressure measurment for switching to idle mode
 char* PARAM_maxPress = "maxPress";                   //maximum Pressure
 char* PARAM_pressTolerance = "pressTolerance";       //maximum Pressure
 char* PARAM_ignitionThrottle = "ignitionThrottle";                //Throttle value to cycle for ignition without RPM sensor
 char* PARAM_throttleHighTime = "throttleHighTime";                //time delay to cycle throttle without RPM
 char* PARAM_throttleLowTime = "throttleLowTime";                //time delay to cycle throttle without RPM
 char* PARAM_idleFuel= "idleFuel";                  //fuel flow at idle in throttle control mode
 char* PARAM_maxFuel="maxFuel";                     //maximum fuel flow in throttle control mode
 char* PARAM_fuelTolerance="fuelTolerance";         //maximum fuel flow in throttle control mode
 char* PARAM_calWeight="calWeight";                 //calibration load for load cell
 char* PARAM_calPress0="calPress0";                 //calibration Pressure point 0
 char* PARAM_calPress1="calPress1";                 //calibration Pressure point 1
 char* PARAM_calVolt0="calVolt0";                 //calibration Pressure Volt 0
 char* PARAM_calVolt1="calVolt1";                 //calibration Pressure Volt 1
 char* PARAM_preheatDelay="preheatDelay";         //Preheat delay

 String inputmaxTemp;
 String inputmaxRPM;
 String inputcpr;
 String inputidleRPM;
 String inputrpmTolerance;
 String inputglowOnRPM ;
 String inputglowOffRPM;
 String inputignitionRPMHigh ;
 String inputignitionRPMLow ;
 String inputgasOnRPM;
 String inputgasOffRPM ;
 String inputstarterOffRPM ;
 String inputfuelOnRPM ;
 String inputpumpOnValue ;
 String inputpurgeTime ;
 String inputpurgeThrottle;
 String inputstarterIncDelay;
 String inputaccelerationDelay;
 String inputdecelerationDelay;
 String inputlowAccelDelay;
 String inputlowDecelDelay;
 String inputignitionThreshold;
 String inputnoIgnitionThreshold;
 String inputfuelFlowDetectionTime;
 String inputfuelFlowDetectionRPM;
 String inputtrialModeFuelOnRPM;
 String inputtrialModeFuelFlow;
 String inputstartingTemp;
 String inputmaxTempGrad;
 String inputmaxMotorVolt;
 String inputmaxPumpVolt;
 String inputMIN_MICROS ;
 String inputMAX_MICROS ;
 String inputvoltIn ;
 String inputcasePress ;
 String inputWiFiSSID ;
 String inputWiFiPWD ;
 String inputkeroPower ;
 String inputkeroTime ;
 String inputkeroFreq ;
 String inputkeroValve ;
 String inputkeroPump ;
 String inputidlePress ;
 String inputmaxPress ;
 String inputpressTolerance ;
 String inputignitionThrottle ;
 String inputthrottleHighTime ;
 String inputthrottleLowTime ;
 String inputidleFuel ;
 String inputmaxFuel ;
 String inputfuelTolerance ;
 String inputcalWeight;
 String inputcalPress0;
 String inputcalPress1;
 String inputcalVolt0;
 String inputcalVolt1;
 String inputpreheatDelay;





//Output Servo PPM Signal min and max times in microseconds
int MIN_MICROS=      1000;  
int MAX_MICROS=      2450;

// minimum and maximum values of output used in calculations. This will be scaled to servo min and max
int outMax=1000;
int outMin=0;
//output signal is identical to rc servo control signal and is generated using servo library
Servo glowServo;
Servo fuelServo;
Servo startServo;

MAX31855 myMAX31855(MAXCS); //chip select pin

int exTemp=0;//Exhaust Temperature degC
int ambTemp=0;//Ambient Temperature degC
int tempGrad=0; //Temp change measured over 1 sec
int tempGradAvg=0; //Temp change averaged measured over 1 sec
int previousTemp=0;//previous value for measuring temperature gradient
long tempTimeOld=0;//to store previous measurement time
int preheatDelay=0; //delay between ignition and Rampup
bool preheatCompleted=false; //Flag to keep track of preheat

//engine modes during normal operation
#define modeWaiting       0  //No RPM //AutoStart
#define modeStarting      1  //RPM 0-Idle RPM  //Initiated through mode switch
#define modeIdling        2  //RPM IdleRPM or idle Pressure
#define modeOperating     3  //RPM IdleRPM-MaxRPM/ idlePress-maxPress
#define modeCooldown      4  //RPM IdleRPM-0 /idlePress

                              
//startup stages during modeStarting
#define startStage0   0  //Purge cycle to purge engine of any residual gases or fuel
#define startStage1   1  //Between Ignition and idle

int startStage=startStage0;  //At startup start with engine purge


bool RPMIncrease=false;//variable to cause starter to oscillate betweem ignitionRPMHigh and ignitionRPMLow until ignition 
#define serverLoopTime  300// update web server data terminal every 300ms
#define sdLoopTime   200 //snapshot of current data is written to SD card every 200 ms
#define serialLoopTime   50 //snapshot of current data is written to serial port every 50 ms
#define rpmLoopTime   100  //RPM is calculated every 100ms
#define tempLoopTime  100  //Temp is measured every 100ms as that is time for max31855 to acquire one reading
#define rcLoopTime   20  //RC Signal is calculated every 20ms
#define voltageLoopTime 500 //battery voltage is measured every half second
#define casePressLoopTime 100 //pressure is measured every 100 millisecond
#define loadcellLoopTime  100// update loadcell data every 100 milliseconds
#define saveUsageTime 20000 //Save engine usage data 20 seconds
#define sdMaxTime 35  //if SD write takes more than 35ms for sdMaxCount in a row stop writing to SD card
long loadcellLoopTimeOld=0;//to store last time serial data was updated
long serialLoopTimeOld=0;//to store last time serial data was updated
long sdLoopTimeOld=0; //to store last time SD card was updated
long casePressLoopTimeOld=0; //To store pressure measurement time
long voltageLoopTimeOld=0; //to store last time battery voltage was updated
long startStageTimeOld=0; //To keep track of time elapsed during start stage
long outputMillisOld=0;// variable to store millis value for calculating accel/deccel delay
long serverLoopTimeOld=0;//variable to keep track of web server data terminal update
long keepalivecounter=0; //variable to keep track of time to send live data to html dataterminal
long engineUsageTimeOld=0; //variable to keep track of engine running time
long preheatDelayTimeOld=0; //variable to keep track of preheat Delay time
int engineMode=modeWaiting;  //Start with engine mode waiting
int startCount=0;   // ECU Restart count
int engineStartCount=0; //Engine start cycles
int engineRunTime = 0; //Engine Run Time in seconds



//RC Signal Variables
unsigned long rcThrottleSignal=0; //Throttle control
unsigned long rcModeSignal=0;  //Used for startup and cool down



// Engine Parameters
    // RPM Set points
int glowOnRPM=           1000;    //Glow plug is switched on as RPM increase above this point
int gasOnRPM=            2000;  //Switch on Gas when this RPM is exceeded
int ignitionRPMLow=      2200;   //ignition cycle low RPM
int ignitionRPMHigh=     6000;  //ignition cycle high RPM
int rpmTolerance=        5000;   //rpm Tolerance for target RPM (applicable at RPM higher than idle RPM only)
int fuelOnRPM=           6000;  //should be much lower than gasOffRPM to ensure continuous combustion
int pumpOnValue=         50;  //This is the value at which pump will start pumping fuel and is usually higher than zero
int glowOffRPM=          26000;  //Glow Plug switched off above this RPM
int gasOffRPM=           26000; //Gas is switched off and engine is accelerating on Fuel only
int starterOffRPM=       24000; //Started is switched of and engine is accelerating on Fuel and/or Gas only
int starterOffTime=      3000; //Starter will turn off if RPM remains above starterOffRPM for starterOffTime
int idleRPM =            32000; //Idle RPM at zero Throttle
int maxRPM=              110000;//Never exceed RPM
int maxTemp=             800;   //Never exceed temperature
int maxTempGrad=         200;  //maximum temperature gradient (degC/Sec)
int startingTemp=        500;   //exhaust temperature maintained during startup
int tempTolerance=       50;   //temperature tolerance for fuel pump control.Not applicable at maxTemp
int noIgnitionThreshold= 6000; //6000ms to abort due to no ignition
int ignitionThreshold=   100; //if exTemp is higher than this value ignition is true otherwise ignition is false;
int purgeTime=           3000; //purge for 3 seconds
int purgeThrottle=       (0.15*outMax); //purge Throttle is 20% of max
int starterIncDelay=     5; //5ms between each iteration of starter motor control
int starterTransition=   (0.1*outMax);//After this value starter delay will apply (outMin-outMax)
int accelerationDelay=   60 ;  //delay time in millisecond for each iteration of output fuelpump control (While Accelerating)
int decelerationDelay=   80; //delay time in millisecond for each iteration of output fuelpump control (While Decelerating)
int lowAccelDelay=       600;  //delay time in millisecond for each iteration of output fuelpump control (While Accelerating)
int lowDecelDelay=       500; //delay time in millisecond for each iteration of output fuelpump control (While Decelerating)
int fuelFlowDetectionTime=10000;  //time in millisecond after switching on fuel pump
int fuelFlowDetectionRPM=18000;  //RPM to confirm engine is running on Fuel
float maxMotorVolt=      12.0;        //default max motor voltage is for 3 cell Lipo
float maxPumpVolt=       12.0;        //default max fuel pump voltage is for 3 cell Lipo
bool keroStart=          false;
int keroPower=           810; //0-1000.   810 to provide 6v with 7.4 v Lipo battery
int keroTime=            5; //heating time for glow plug prior to pumping fuel-In second default 5 seconds 0-60 seconds
int keroValve=           100; //0-1000- On time of kero valve PWM
int keroFreq=            10; // kero valve frequency 0-60
int keroPump=            100; //0-1000
unsigned long keroStartTime=0; //variable to store time at which glow plug was switched on
uint8_t glowpwmchannel=  6;
uint8_t valvepwmchannel= 0;
int ignitionThrottle=    110; //throttle for ignition without RPM sensor
int throttleHighTime=       1000;   //delay for throttle cycling without RPM sensor (milliseconds)
int throttleLowTime=       2500;   //delay for throttle cycling without RPM sensor (milliseconds)
int idleFuel=            130; //Fuel flow to switch to idle in throttle control mode
int maxFuel=             300; //maximum fuel flow in throttle control mode
int fuelTolerance=       3;


//RPM Variables
int          cpr=                   1; //counts per revolution. For Hall type sensors it is 1 and for through hole optical sensors it is 2 counts per revolution
volatile int revs=                 0;
volatile int RPM=                  0;
volatile int RPMAvg=               0;

int rpmGrad    =          0   ; //measured RPM gradient in RPM/Sec
int rpmGradAvg=0; //rpm change averaged measured over 1 sec
int previousRPM=0;//previous value for measuring RPM gradient


//constants
int RPMHoldGrad=         500; //RPM control will react if rpmGrad is higher than this value
int RPMRiseGrad=         20000;  //RPM Rise rate of 20000 RPM/Sec
int RPMDropGrad=         -10000;  //RPM Drop rate of 10000 RPM/Sec




// Other variables

volatile long measuredLoopTime=0;//main loop time measurement
volatile long sdWriteTime=0;
unsigned long maxLoopTime=0;
int sdMaxCount=0; // no of times sdWrite time has exceeded max limit
volatile byte errorCode=0; //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
bool ignitionState=false;//default state



RunningAverage RPMBuff(4);
RunningAverage RPMGradBuff(4);
RunningAverage tempGradBuff(4);

unsigned long RPMLoopTimeOld=0; //to store previous RPM calculation  time
unsigned long RCLoopTimeOld=0; //to start RC Signal measurement after this interval
long mainLoopTimeOld=0;//for measuring main loop time
unsigned long starterOffTimeOld=0;//for measuring starter time at max RPM
volatile unsigned long gasOnTime=0;//Time when gas valve was opened
volatile unsigned long RCModemicrosold=0;  //for calculating RC mode signal
volatile unsigned long RCThrottlemicrosold=0;//for calculating RC throttle signal
volatile unsigned long RCModemicros=0;
volatile unsigned long RCThrottlemicros=0;
volatile long RCSignalmillis=0;//To store time when RC signal was measured so RC signal failure can be detected
int RCSignalMaxTime=1000; //there should be RC signal within 1000 milliseconds
int rpmDiff=0; //RPM difference between current RPM and IDLE RPM

//Output Servo signal variables
int fuelFlowTarget=outMin; //Target value for fuel pump  
int fuelFlowNow=outMin; //current value of output signal for fuel pump  
long fuelFlowTimeOld=0; //Time of previous update for fuel pump 
int startMotorTarget=outMin;// target value for starter motor 
int startMotorNow=outMin;// current value of output signal for starter motor 
long startMotorTimeOld=0;// Time of previous update for starter motor 
int idleFuelFlow=outMin; //pump throttle at idle RPM  
bool gasFlow=false; //Solenoid gas valve for starting
bool fuelFlow=false;//Solenoid Fuel Valve
int glowPower=outMin; //output signal for glow plug  
long throttleTimerHigh=0; //keeping track of throttle based cycling
long throttleTimerLow=0; //keeping track of throttle based cycling

//Test variables
volatile bool testGlowPlug=false;
volatile bool testFuelSolenoid=false;
volatile bool testGasSolenoid=false;
volatile long testmillis=0;


//Voltage Sensor variables
   
int voltsensorValue = 0;  // variable to store the value coming from the sensor
float resistorRatio=7.1/1.5; //use actual values of resistor divider network as measured by multimeter to avoid error from resistor tolerance
float voltCorrection=1; //correction factor applied to resistance ratio to get accurate voltage measurement
float voltage=0;//Calculated voltage value
//Variables to store data at Abort condition

//Pressure Sensor Variables
float casePress=0;
float idlePress=0;
float maxPress=0;
float pressOffset=0;
float pressTolerance=2.0;
bool  useCustomCal=false;
float calPress0=0;
float calPress1=0;
float calVolt0=0;
float calVolt1=0;


//ControlMethod
bool startRpmControl=true; //startup is based on rpm feedback
bool startPressureControl=false;//startup is based on pressure feedback
bool startThrottleControl=false;//startup is through throttle control and is open loop
bool runRpmControl=true; //Operation from idle onwards is based on rpm feedback
bool runPressureControl=false;//Operation from idle onwards is based on pressure feedback
bool runThrottleControl=false;//operation from idle onwards is through throttle control and is open loop

 int abRPMAvg=0;
 int abexTemp=0;
 int abfuelFlowTarget=0; //output signal for fuel pump  0-1000
 int abfuelFlowNow=0; //output signal for fuel pump  0-1000
 int abstartMotorTarget=0;// output signal for starter motor 0-1000
 int abstartMotorNow=0;// output signal for starter motor 0-1000
 bool abgasFlow=false; //gas valve for starting
 bool abfuelFlow=false; //gas valve for starting
 bool abignitionState=false;
 int abrcModeSignal=0;
 int abrcThrottleSignal=0;
 int abglowPower=0;//Stop Glow plug if On
 int abengineMode=0;
  int abstartStage=0;
float  abvoltage=voltage;
byte  aberrorCode=errorCode;

//RPM Counter
 void IRAM_ATTR CounterOverflow(void *arg) {                  // Interrupt for overflow of pulse counter
    OverflowCounter = OverflowCounter + 1;                     // increase overflow counter
    PCNT.int_clr.val = BIT(PCNT_FREQ_UNIT);                    // clean overflow flag
    pcnt_counter_clear(PCNT_FREQ_UNIT);                        // zero and reset of pulse counter unit
  }

void IRAM_ATTR isr_2() {  //RC Mode Signal Measurement
 
 if (digitalRead(RC_Mode_Pin)==HIGH) RCModemicrosold=micros();
 else  
 {
  if((micros()-RCModemicrosold)>800)RCModemicros=(micros()-RCModemicrosold);//ignore values below 800 microseconds
 }
  }

  void IRAM_ATTR isr_3() {  //RC Throttle Signal Measurement

 if (digitalRead(RC_Throttle_Pin)==HIGH) RCThrottlemicrosold=micros();
 else  
 {
  if((micros()-RCThrottlemicrosold)>800) RCThrottlemicros=(micros()-RCThrottlemicrosold);//ignore values below 800 microseconds
  RCSignalmillis=millis();//Updated at falling edge of RC Signal pulse
 }

 }



//  constants stored in EEPROM
Preferences preferences;


 
void setup(void) {
  
Serial.begin(115200);
ReadSettings();//read constants from EEPROM if available
// Initialize GPIO Pins
ledcSetup(valvepwmchannel,20,10);
ledcSetup(glowpwmchannel,5000,10);

pinMode(LEDPin_1,OUTPUT);  
pinMode(Fuel_Solenoid_Pin,OUTPUT);
pinMode(GasPin,OUTPUT);
if (keroStart) 
{
  ledcAttachPin(GasPin,valvepwmchannel);// Solenoid valve will be used with keroFreq to meter fuel in keroplug fuel supply
 ledcWrite(valvepwmchannel, 0);
//   valvepwm.attachPin(GasPin, keroFreq,10);// Solenoid valve will be used with keroFreq to meter fuel in keroplug fuel supply
  //valvepwm.write(0);
}
digitalWrite(LEDPin_1,HIGH);
digitalWrite(Fuel_Solenoid_Pin,LOW);
digitalWrite(GasPin,LOW);

pinMode(RC_Mode_Pin,INPUT_PULLDOWN);
pinMode(RC_Throttle_Pin,INPUT_PULLDOWN);
pinMode(voltsensorPin,INPUT);
pinMode(PressurePin,INPUT);

if(loadCellEnable){
scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
 
if (scale.wait_ready_timeout(1000))
{
      tareconst= scale.read_average(5);
      Serial.println(tareconst);
      scale.tare();
      scale.set_offset(tareconst);
    
 /*  if (taredone) scale.set_offset(tareconst);
  else scale.set_offset(0);*/
  if (scalingdone)  scale.set_scale(scaleconst);
  else scale.set_scale(1.0);
  }
}

//RPM Setup
 pinMode(RPMsensorPin, INPUT);
initPulseCounter ();
 attachInterrupt(RC_Mode_Pin, isr_2, CHANGE);
 attachInterrupt(RC_Throttle_Pin, isr_3, CHANGE);

 RPMBuff.clear(); //clear the buffer used to average RPM values

myMAX31855.begin();


// Allow allocation of all timers
  //ESP32PWM::allocateTimer(0);//Timer 0 now used for pulse counter
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);


 if (!keroStart) glowServo.setPeriodHertz(50);// Standard 50hz servo- initialize if kerostart is false
  fuelServo.setPeriodHertz(50);// Standard 50hz servo
  startServo.setPeriodHertz(50);// Standard 50hz servo
  
   if (keroStart)
   {
     ledcAttachPin(GlowPin,glowpwmchannel);// Solenoid valve will be used with keroFreq to meter fuel in keroplug fuel supply
 ledcWrite(glowpwmchannel, 0);
 
   }
  else glowServo.attach(GlowPin, MIN_MICROS, MAX_MICROS);  //only if kerostart is false
  fuelServo.attach(FuelPumpPin, MIN_MICROS, MAX_MICROS);  
  startServo.attach(StarterPin, MIN_MICROS, MAX_MICROS);
  
  fuelServo.write(0);
  startServo.write(0);
  glowServo.write(0); //0-180 
  delay(3000);//let the servo signals stabilize
  
  
  //save ECU Start Count in EEPROM

preferences.begin("settings",false);
preferences.getInt("startCount",startCount);

startCount++;

preferences.putInt("startCount",startCount);
preferences.end();
Serial.print("ECU Powerup cycles  ");
Serial.println(startCount);   
  
 
//Initialize all timers/counters

  mainLoopTimeOld=loadcellLoopTimeOld=serialLoopTimeOld=serverLoopTimeOld=casePressLoopTimeOld=sdLoopTimeOld=RPMLoopTimeOld=RCLoopTimeOld=voltageLoopTimeOld=tempTimeOld=RCSignalmillis=millis();
//Create Task on core 0 to run SD recording loop
xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "sdTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &sdTask,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  


}

void loop(void) {
  
 if((startServer)&&(!serverStarted)&&(millis()>5000))//5 seconds before WiFi startup is given to allow power to stabilize
 {
   Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
  WebServerFunction();//Start webserver if 5 seconds have passed and WiFi server is not started
  initSD();//Start SD recording
 }
 
 if(serverStarted) dnsServer.processNextRequest();//process DNS request
   measuredLoopTime=millis()-mainLoopTimeOld;//calculate execution time of the program
   if (measuredLoopTime>maxLoopTime) maxLoopTime=measuredLoopTime;
   mainLoopTimeOld=millis();
  CheckFailure();
  ReadRC();
  ReadRPM();
  ReadTemperature(); 
  ReadPressure();
  ReadVoltage();

  switch(engineMode)
    {
      case modeWaiting:
      WaitingFunction();
      break;
      
      case modeStarting:
      StartingFunction();
      break;
      
      case modeIdling:
      IdlingFunction();
      break;
      
      case modeOperating:
      OperatingFunction();
      break;
      
      case modeCooldown:
      CooldownFunction();
      break;

        
      default:
      WaitingFunction();
      break; 
}
  ControlOutput();

  UpdateWebData();// Update web server with new data
    if (ignitionState) UpdateUsageData();
  
 
}

//Task1code: This is like a second loop functioning running on core0. By default main loop runs on Core 1 of ESP32
void Task1code( void * pvParameters ){
//  Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());
//This function will run forever, like a second loop function
  for(;;){      
   if(sdavailable && writefile)  SaveData();
   if (startSerial&& (millis()>10000)) SendSerialData();
   if(loadCellEnable)   readScale();
      if(engineMode==modeWaiting){
        if (testGasSolenoid||testFuelSolenoid||testGlowPlug) testSensors();
      }
        vTaskDelay(10 / portTICK_PERIOD_MS);
  } 
}
void testSensors()
{
  if ((testGasSolenoid)&&(millis()>(testmillis+2000))&&(testmillis>0)) 
  {
    digitalWrite(GasPin,LOW);
    testmillis=0;
    testGasSolenoid=false;
  }
  else if(testGasSolenoid)
  {
    digitalWrite(GasPin,HIGH);
  
  }
  else if ((testFuelSolenoid)&&(millis()>(testmillis+2000))&&(testmillis>0)) 
  {
    digitalWrite(Fuel_Solenoid_Pin,LOW);
    testmillis=0;
    testFuelSolenoid=false;
  }
  else if(testFuelSolenoid)
  {
    digitalWrite(Fuel_Solenoid_Pin,HIGH);
  
  }
   else if ((testGlowPlug)&&(millis()>(testmillis+7000))&&(testmillis>0)) 
  {
   if(!keroStart)  glowServo.write(map(outMin,outMin,outMax,MIN_MICROS,MAX_MICROS)); 
else ledcWrite(glowpwmchannel, outMin); 
    testmillis=0;
    testGlowPlug=false;
  }
  else if((testGlowPlug))
  {
    if(!keroStart)  glowServo.write(map(outMax,outMin,outMax,MIN_MICROS,MAX_MICROS)); 
else ledcWrite(glowpwmchannel, outMax); 
    
  }
  else
  {
    testGasSolenoid=false;
    testFuelSolenoid=false;
    testGlowPlug=false;
  }

      
}
void readScale()
{
  if((millis()-loadcellLoopTimeOld)>loadcellLoopTime)
  {
  if (scale.wait_ready_timeout(2))  loadcellvalue = scale.get_units(5);
  }
}
void SendSerialData()
{
  if((millis()-serialLoopTimeOld)>serialLoopTime)
  {
  if (!serialheaderprinted)
  {
    PrintDataHeader();
    serialheaderprinted=true;
  }

   updateSerialData();
   int writesize=Serial.availableForWrite();
   Serial.print(serialData.substring(0,writesize));
   serialData.remove(0,writesize);
   serialLoopTimeOld=millis();
}

}
void UpdateWebData()
{
  
  if ((millis()-keepalivecounter)<5000)//Webdata is updated if a client is connected and counter is reset every 5 seconds
  {
  if((millis()-serverLoopTimeOld)>serverLoopTime)//If a client is connected , data is sent every 200 milli seconds 
{
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    serverLoopTimeOld = millis();
}
}
}

void ReadVoltage()
{ 
  if((millis()-voltageLoopTimeOld)>voltageLoopTime)
{
  // read the value from the sensor:
   voltsensorValue = analogRead(voltsensorPin);
   voltage= resistorRatio*voltCorrection*3.3*(float(voltsensorValue)/4095);//voltage factor is the actual ratio of voltage divider used to reduce voltage from battery to ESP32 input range of maximum 3.3v
   voltageLoopTimeOld=millis();
}
}
void ReadPressure()
{ 
  if((millis()-casePressLoopTimeOld)>casePressLoopTime)
{
  if(useCustomCal)
  {
    int  casePressValue = analogRead(PressurePin);
       float Vpress= 3.3*(float(casePressValue)/4095.0);
        float Vdiff=Vpress-calVolt0;         
    if((calVolt1-calVolt0)==0) casePress=0;
    else{
      casePress=calPress0+((Vdiff/(calVolt1-calVolt0))*(calPress1-calPress0));
    }
    
  }
  else
 { 
  // read the value from the sensor:
 int  casePressValue = analogRead(PressurePin);
 casePress= ((3.3*(float(casePressValue)/4095.0))-0.5)/0.02;
 
}
casePress=casePress-pressOffset;
 casePressLoopTimeOld=millis();
}
}

void ReadRPM()
{

if ((millis()-RPMLoopTimeOld)>=rpmLoopTime)
{
  int timeElapsed=millis()-RPMLoopTimeOld;
Read_Reset_PCNT();
RPMLoopTimeOld=millis();
 revs=PulseCounter;
 revs=revs/cpr; //normalizing revs according to sensor type

  // read the value from the sensor:
   
     RPM=(revs*60*1000)/(timeElapsed);//As pulse counter is operating on rising edge
     RPMBuff.addValue(RPM);
     RPMAvg= RPMBuff.getAverage();//we use average to smooth out RPM

     rpmGrad =  ((RPM-previousRPM)*1000)/timeElapsed; 
     RPMGradBuff.addValue(rpmGrad);
    rpmGradAvg=RPMGradBuff.getAverage(); 
    previousRPM=RPM;//previous value for measuring RPM gradient
  
}
}
void ReadRC()//values are updated in ISR and this function processes the result
{
  if ((millis()-RCLoopTimeOld)>=rcLoopTime)
  {
//saving previous values
unsigned long rcThrottleSignalold=rcThrottleSignal;
unsigned long rcModeSignalold=rcModeSignal;
rcThrottleSignal=RCThrottlemicros;
if((rcThrottleSignal<1000)) rcThrottleSignal=1000;
rcThrottleSignal=(rcThrottleSignal-1000)/10;//RC signal in %
//removing signal glithes bigger than 30
if (rcThrottleSignal>(rcThrottleSignalold+30))rcThrottleSignal=rcThrottleSignalold+30;
if ((rcThrottleSignalold>30)&&(rcThrottleSignal<(rcThrottleSignalold-30)))rcThrottleSignal=rcThrottleSignalold-30;

rcModeSignal=RCModemicros;
if((rcModeSignal<1000)||(rcModeSignal>2300)) rcModeSignal=1000;
rcModeSignal=(rcModeSignal-1000)/10;//RC signal in %

//removing signal glithes bigger than 30
if (rcModeSignal>(rcModeSignalold+30))rcModeSignal=rcModeSignalold+30;
if ((rcModeSignalold>30)&&(rcModeSignal<(rcModeSignalold-30)))rcModeSignal=rcModeSignalold-30;
RCLoopTimeOld=millis();
 
  }
}


void ReadTemperature()
{
  if ((millis()-tempTimeOld)>tempLoopTime)
  {
    digitalWrite(MAXCS,LOW);
   uint32_t rawData = myMAX31855.readRawData();
   exTemp = myMAX31855.getTemperature(rawData);
   ambTemp=myMAX31855.getColdJunctionTemperature(rawData);
   if (isnan(exTemp)) exTemp=0;
   int elapsedtime=millis()-tempTimeOld;
   tempGrad=(exTemp-previousTemp)*1000/elapsedtime;
   tempGradBuff.addValue(tempGrad);
   tempGradAvg= tempGradBuff.getAverage();//we use average to smooth out temperature gradient
   previousTemp=exTemp;
   tempTimeOld=millis();
 if ((ignitionState) &&(exTemp<ignitionThreshold))//detect flameout condition when after ignition temperature drops below threshold
   {
    if (fuelFlowNow>0){
     
    bitSet(errorCode,4);//1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
    AbortAll();
    }
    else{
      AbortAll();
         }
    
   }
  if (exTemp>=ignitionThreshold) ignitionState=true;
  if (exTemp<ignitionThreshold) ignitionState=false;
  digitalWrite(MAXCS,HIGH);
  }
 
}

  void ReadSettings()//Read settings from Flash memory
{  //name of the key in key-value pair should be maximum 15 characters
   preferences.begin("settings",true);
   maxRPM=preferences.getInt("maxRPM",maxRPM);
   maxTemp=preferences.getInt("maxTemp",maxTemp);
   idleRPM=preferences.getInt("idleRPM",idleRPM);
   rpmTolerance=preferences.getInt("rpmTolerance",rpmTolerance);
   glowOnRPM=preferences.getInt("glowOnRPM",glowOnRPM);
   glowOffRPM=preferences.getInt("glowOffRPM",glowOffRPM);
   ignitionRPMHigh=preferences.getInt("ignitionRPMHigh",ignitionRPMHigh);
   ignitionRPMLow=preferences.getInt("ignitionRPMLow",ignitionRPMLow);
   gasOnRPM=preferences.getInt("gasOnRPM",gasOnRPM);
   gasOffRPM=preferences.getInt("gasOffRPM",gasOffRPM);
   starterOffRPM=preferences.getInt("starterOffRPM",starterOffRPM);
   fuelOnRPM=preferences.getInt("fuelOnRPM",fuelOnRPM);
   pumpOnValue=preferences.getInt("pumpOnValue",pumpOnValue);
   purgeThrottle=preferences.getInt("purgeThrottle",purgeThrottle);
   purgeTime=preferences.getInt("purgeTime",purgeTime);
   starterIncDelay=preferences.getInt("starterIDelay",starterIncDelay);
   noIgnitionThreshold=preferences.getInt("noIgnThreshold",noIgnitionThreshold);//Time after which it is decided that starting gas has not ignited
   ignitionThreshold=preferences.getInt("ignThreshold",ignitionThreshold);//temperature at which ignition is detected
   accelerationDelay=preferences.getInt("accelDelay",accelerationDelay);
   decelerationDelay=preferences.getInt("decelDelay",decelerationDelay);
   fuelFlowDetectionTime=preferences.getInt("fuelDetTime",fuelFlowDetectionTime);
   fuelFlowDetectionRPM=preferences.getInt("fuelDetRPM",fuelFlowDetectionRPM);
   startingTemp=preferences.getInt("startingTemp",startingTemp);
   maxTempGrad=preferences.getInt("maxTempGrad",maxTempGrad);
   maxMotorVolt=preferences.getFloat("maxMotorVolt",maxMotorVolt);
   maxPumpVolt=preferences.getFloat("maxPumpVolt",maxPumpVolt);
   MIN_MICROS=preferences.getInt("MIN_MICROS",MIN_MICROS);//Servo PPM Signal min and max times in microseconds
   MAX_MICROS=preferences.getInt("MAX_MICROS",MAX_MICROS);
   RPMRiseGrad=preferences.getInt("RPMRiseGrad",RPMRiseGrad);
   RPMDropGrad=preferences.getInt("RPMDropGrad",RPMDropGrad);
   lowAccelDelay=preferences.getInt("lowADelay",lowAccelDelay);
   lowDecelDelay=preferences.getInt("lowDDelay",lowDecelDelay);
   cpr=preferences.getInt("cpr",cpr);//counts per revolution for RPM sensor
   keroStart=preferences.getBool("kero",keroStart); // Kero start or gas start. If this is true engine will start on keroscene 
   keroTime=preferences.getInt("keroTime",keroTime); // Kero plug heating time in milliseconds
   keroPower=preferences.getInt("keroPower",keroPower); // Kero plug heating power 0-1000
   keroValve=preferences.getInt("keroValve",keroValve); // Kero fuel flow valve settings 0-1000
   keroFreq=preferences.getInt("keroFreq",keroFreq); // Kero valve frequency
   keroPump=preferences.getInt("keroPump",keroPump); // Kero fuel flow pump settings 0-1000
   startCount=preferences.getInt("startCount",startCount);
   engineStartCount=preferences.getInt("engineStarts",engineStartCount);
   engineRunTime=preferences.getInt("runTime",engineRunTime);
   voltCorrection=preferences.getFloat("voltCorr",voltCorrection);
   ssid=preferences.getString("SSID",ssid);
   password=preferences.getString("PWD",password);
  idlePress=preferences.getFloat("idlePress",idlePress); // Pressure limit to switch to idle mode
   maxPress=preferences.getFloat("maxPress",maxPress); // maximum Pressure limit
   pressTolerance=preferences.getFloat("pressTol",pressTolerance); // Pressure Tolerance
   idleFuel=preferences.getInt("idleFuel",idleFuel); // Fuel Flow to switch to idle mode
   maxFuel=preferences.getInt("maxFuel",maxFuel); // maximum Fuel Flow limit
   fuelTolerance=preferences.getInt("fuelTol",fuelTolerance); // Fuel control tolerance limit
   pressOffset=preferences.getFloat("pressOffset",pressOffset); // zero Offset
   runPressureControl=preferences.getBool("rpressControl",runPressureControl);
   runRpmControl=preferences.getBool("rrpmControl",runRpmControl);
   runThrottleControl=preferences.getBool("rthrControl",runThrottleControl);
   startPressureControl=preferences.getBool("spressControl",startPressureControl);
   startRpmControl=preferences.getBool("srpmControl",startRpmControl);
   startThrottleControl=preferences.getBool("sthrControl",startThrottleControl);
   startSerial=preferences.getBool("startSerial",startSerial);
   ignitionThrottle=preferences.getInt("ignThrottle",ignitionThrottle);
   throttleHighTime=preferences.getInt("thrDelay",throttleHighTime);
   throttleLowTime=preferences.getInt("thrDelay2",throttleLowTime);
   taredone=preferences.getBool("TareDone",taredone);
   scalingdone=preferences.getBool("ScalingDone",scalingdone);
   tareconst=preferences.getInt("TareConst",tareconst);
   scaleconst=preferences.getFloat("ScaleConst",scaleconst);
   useCustomCal=preferences.getBool("customCal",useCustomCal);
   calPress0=preferences.getFloat("calPress0",calPress0);
   calPress1=preferences.getFloat("calPress1",calPress1);
   calVolt0=preferences.getFloat("calVolt0",calVolt0);
   calVolt1=preferences.getFloat("calVolt1",calVolt1);
   loadCellEnable=preferences.getBool("loadEnable",loadCellEnable);
   preheatDelay=preferences.getInt("phDelay",preheatDelay);
 preferences.end();

}   
  void WriteSettings()//Write settings from Flash memory
{  //name of the key in key-value pair should be maximum 15 characters
   preferences.begin("settings",false);
   preferences.putInt("maxRPM",maxRPM);
   preferences.putInt("maxTemp",maxTemp);
   preferences.putInt("idleRPM",idleRPM);
   preferences.putInt("rpmTolerance",rpmTolerance);
   preferences.putInt("glowOnRPM",glowOnRPM);
   preferences.putInt("glowOffRPM",glowOffRPM);
   preferences.putInt("ignitionRPMHigh",ignitionRPMHigh);
   preferences.putInt("ignitionRPMLow",ignitionRPMLow);
   preferences.putInt("gasOnRPM",gasOnRPM);
   preferences.putInt("gasOffRPM",gasOffRPM);
   preferences.putInt("starterOffRPM",starterOffRPM);
   preferences.putInt("fuelOnRPM",fuelOnRPM);
   preferences.putInt("pumpOnValue",pumpOnValue);
   preferences.putInt("purgeThrottle",purgeThrottle);
   preferences.putInt("purgeTime",purgeTime);
   preferences.putInt("starterIDelay",starterIncDelay);
   preferences.putInt("noIgnThreshold",noIgnitionThreshold);//Time after which it is decided that starting gas has not ignited
   preferences.putInt("ignThreshold",ignitionThreshold);//temperature at which ignition is detected
   preferences.putInt("accelDelay",accelerationDelay);
   preferences.putInt("decelDelay",decelerationDelay);
   preferences.putInt("fuelDetTime",fuelFlowDetectionTime);
   preferences.putInt("fuelDetRPM",fuelFlowDetectionRPM);
   preferences.putInt("startingTemp",startingTemp);
   preferences.putInt("maxTempGrad",maxTempGrad);
   preferences.putFloat("maxMotorVolt",maxMotorVolt);
   preferences.putFloat("maxPumpVolt",maxPumpVolt);
   preferences.putInt("MIN_MICROS",MIN_MICROS);//Servo PPM Signal min and max times in microseconds
   preferences.putInt("MAX_MICROS",MAX_MICROS);
   preferences.putInt("RPMRiseGrad",RPMRiseGrad);
   preferences.putInt("RPMDropGrad",RPMDropGrad);
   preferences.putInt("lowADelay",lowAccelDelay);
   preferences.putInt("lowDDelay",lowDecelDelay);
   preferences.putInt("cpr",cpr);//counts per revolution for RPM sensor
   preferences.putBool("kero",keroStart); // Kero start or gas start. If this is true engine will start on keroscene 
   preferences.putInt("keroTime",keroTime); // Kero plug heating time in milliseconds
   preferences.putInt("keroPower",keroPower); // Kero plug heating power 0-1000
   preferences.putInt("keroValve",keroValve); // Kero fuel flow valve settings 0-1000
   preferences.putInt("keroFreq",keroFreq); // Kero valve frequency
   preferences.putInt("keroPump",keroPump); // Kero fuel flow pump settings 0-1000
   preferences.putFloat("idlePress",idlePress); // Pressure limit to switch to idle mode
   preferences.putFloat("maxPress",maxPress); // maximum Pressure limit
   preferences.putFloat("pressTol",pressTolerance); // pressure control tolerance
   preferences.putInt("idleFuel",idleFuel); // Fuel Flow to switch to idle mode
   preferences.putInt("maxFuel",maxFuel); // maximum Fuel Flow limit
   preferences.putInt("fuelTol",fuelTolerance); // Fuel Flow control tolerance limit
   preferences.putFloat("pressOffset",pressOffset); // zero Offset
   preferences.putBool("spressControl",startPressureControl);
   preferences.putBool("srpmControl",startRpmControl);
   preferences.putBool("sthrControl",startThrottleControl);
   preferences.putBool("rpressControl",runPressureControl);
   preferences.putBool("rrpmControl",runRpmControl);
   preferences.putBool("rthrControl",runThrottleControl);
   preferences.putBool("startSerial",startSerial);
   preferences.putInt("ignThrottle",ignitionThrottle);
   preferences.putInt("thrDelay",throttleHighTime);
   preferences.putInt("thrDelay2",throttleLowTime);
   preferences.putBool("TareDone",taredone);
   preferences.putBool("ScalingDone",scalingdone);
   preferences.putInt("TareConst",tareconst);
   preferences.putFloat("ScaleConst",scaleconst);
   preferences.putBool("customCal",useCustomCal);
   preferences.putFloat("calPress0",calPress0);
   preferences.putFloat("calPress1",calPress1);
   preferences.putFloat("calVolt0",calVolt0);
   preferences.putFloat("calVolt1",calVolt1);
   preferences.putBool("loadEnable",loadCellEnable);
   preferences.putInt("phDelay",preheatDelay);
   preferences.end();

}   

void WaitingFunction()
{
if (errorCode>0)
{
  delay(3000);
  errorCode=0; //reset error code
}
 startStage=startStage0;
  startStageTimeOld=0;
  gasOnTime=0;
  keroStartTime=0;
  glowPower=outMin;
  gasFlow=false;
  fuelFlow=false;
  preheatCompleted=false;
  fuelFlowTarget=outMin;//This will change in below command if system is in pumpPrime mode
  
 if(!pumpPrime) startMotorTarget=map(rcThrottleSignal,0,100,outMin,outMax);        //starter will follow throttle control
else fuelFlowTarget=map(rcThrottleSignal,0,100,outMin,outMax);        //follow throttle control
if ((rcModeSignal>80)&&(rcThrottleSignal<10)&&(exTemp<ignitionThreshold))//Start only when throttle is below 10%
{
  pumpPrime=false;//reset throttle control at startup to starter control
  engineMode=modeStarting;
  startStage=startStage0;
  startStageTimeOld=0;
}

}
void StartingFunction()
{
 if(startRpmControl) RPMStartingFunction();
 else if(startPressureControl) NoRPMStartingFunction();
 else if(startThrottleControl) NoRPMStartingFunction();
}
void NoRPMStartingFunction()
{
  if (rcModeSignal<20) AbortAll();
  else
  {
 
    if (startStage==startStage0)  //Purge cycle
    {
      if (startStageTimeOld==0) 
      {
        startStageTimeOld=millis();//initialize the purge Timer
        throttleTimerLow=0;
      }
     else if ((startStageTimeOld>0)&&(throttleTimerLow==0)&&((millis()-startStageTimeOld)>purgeTime))//determine if end of Purge cycle reached
      {
        startMotorTarget=outMin;
       throttleTimerLow=millis();
        
      }
       else if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)<purgeTime)) startMotorTarget=purgeThrottle;//set Start Motor at purge Throttle
     
      else if ((startStageTimeOld>0)&&(throttleTimerLow>0)&&((millis()-throttleTimerLow)>throttleLowTime))//determine if end of Purge cycle reached
      {
       
        startStageTimeOld=0;
        throttleTimerLow=0;
        throttleTimerHigh=0;
        startStage=startStage1;//move to next stage (ignition)
       
      }
    
    }
    else    // if (startStage==startStage1)  //ignition cycle
    { 
      if (!keroStart)
      {
     //glow plug operation
   glowPower=outMax;
    

    //gas flow operation
    if ((glowPower==outMax) &&(gasFlow==false)) 
    {
      gasFlow=true;
      gasOnTime=millis();
    }
    if ((gasOnTime>0)&&((millis()-gasOnTime)>noIgnitionThreshold)&&(!ignitionState) )
    {
      //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
   
      bitSet(errorCode,0);//No Ignition
      gasOnTime=0;
      AbortAll();
       
    }
         }
    else NoRPMKeroStartFunction();
    
    //ignition cycle -oscillate between two RPM's untill ignition is established
    if(!ignitionState)
    {
     if(RPMIncrease) startMotorTarget=ignitionThrottle;
     else 
     {
      startMotorTarget=outMin;//Using a target of zero has worked better as compared to gradually reducing RPM
     startMotorNow=outMin;
     }
      
         if((RPMIncrease) &&((millis()-throttleTimerHigh)>throttleHighTime))
         {
          RPMIncrease=false;//using ignitionThrottleHigh as a throttle control
          throttleTimerLow=millis();
         }
     else if((!RPMIncrease)&&((millis()-throttleTimerLow)>throttleLowTime))
     {
      RPMIncrease=true;//At ignitionRPMLow start increasing starter RPM
      throttleTimerHigh=millis();
     }
        }
   
 
else//ignition is established
{
 engineUsageTimeOld=millis(); //initialize engine use counter
if(startMotorNow<ignitionThrottle) startMotorTarget++ ;  //first get to ignitionThrottleHigh
else 
if (!preheatCompleted) preheatDelayFunction();
else
{
  if (startMotorTarget<outMax)
      {
        //Ramp up starter RPM while keeping Temp at Starting Temp
        if(exTemp>startingTemp) 
        {
          startMotorTarget++;
          if ((exTemp+(2*tempGrad))>maxTemp) fuelFlowTarget--;    //2 sec look ahead
        }
        else if (tempGrad<=0)fuelFlowTarget++;
      }
    else //  if start motor is maxed out
       { 
        //Keep increasing fuel flow but keep temperature in check. 
        //If fuel flow increase temperature above max temp and RPM is below self sustaining RPM and starter motor is already at 100% 
        // then starter motor is not giving the RPM to reach self sustaining mode.Engine will not accelerate further and 
        //will not reach self sustaining RPM
        
        if (exTemp<maxTemp)fuelFlowTarget++;
             else fuelFlowTarget--;
      }
}
if ((startThrottleControl)&&(fuelFlowNow>idleFuel)&&(exTemp<maxTemp)) 
{
  fuelFlowTarget++;//Give a little kick as starter will shut off and now it will switch to idling mode
   startMotorTarget=outMin;
   startMotorNow=outMin;
    engineMode=modeIdling; 
    engineStartCount++;  //Increment engine usage counter
}
else if ((startPressureControl)&&(casePress>idlePress)&&(exTemp<maxTemp)) 
{
  fuelFlowTarget++;//Give a little kick as starter will shut off and now it will switch to idling mode
   startMotorTarget=outMin;
   startMotorNow=outMin;
    engineMode=modeIdling; 
    engineStartCount++;  //Increment engine usage counter
}

       }
      }
     }
    }
    
void RPMStartingFunction()
{
  if (rcModeSignal<20) AbortAll();
  else
  {
 
    if (startStage==startStage0)  //Purge cycle
    {
      if (startStageTimeOld==0) startStageTimeOld=millis();//initialize the purge Timer
      if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)>purgeTime))//determine if end of Purge cycle reached
      {
        startMotorTarget=outMin;
        
      }
       if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)>purgeTime)&& (RPMAvg<ignitionRPMLow))//determine if end of Purge cycle reached
      {
       
        startStageTimeOld=0;
        startStage=startStage1;//move to next stage (ignition)
       
      }
      if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)<purgeTime)) startMotorTarget=purgeThrottle;//set Start Motor at purge Throttle
     
    }
    else    // if (startStage==startStage1)  //ignition cycle
    { 
      if (!keroStart)
      {
     //glow plug operation
    if ((RPMAvg>=glowOnRPM)&&(RPMAvg<glowOffRPM))glowPower=outMax;
    else  glowPower=outMin;

    //gas flow operation
    if ((glowPower==outMax) &&(gasFlow==false)&&(RPMAvg>gasOnRPM)&&(RPMAvg<gasOffRPM)) 
    {
      gasFlow=true;
      gasOnTime=millis();
    }
    if ((gasOnTime>0)&&((millis()-gasOnTime)>noIgnitionThreshold)&&(!ignitionState) )
    {
       gasOnTime=0;
       bitSet(errorCode,0);//No Ignition
      AbortAll();
       //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
     
     
    }
   
    if ((RPMAvg>gasOffRPM)|| (RPMAvg<gasOnRPM))
    {
      gasFlow=false;
      gasOnTime=0;
      
    }
    }
    else KeroStartFunction();
    
    //ignition cycle -oscillate between two RPM's untill ignition is established
    if(!ignitionState)
    {
     if(RPMIncrease) startMotorTarget++;
     else 
     {
      startMotorTarget=outMin;//Using a target of zero has worked better as compared to gradually reducing RPM
     startMotorNow=outMin;
     }
     if((RPMAvg>ignitionRPMHigh)) RPMIncrease=false;//At ignitionRPMHigh start reducing starter RPM
     else if((RPMAvg<ignitionRPMLow)) RPMIncrease=true;//At ignitionRPMLow start increasing starter RPM     
    
    }
    
        //Ramp up to idle- starter Motor and fuel flow during ramp up Opertion
 
else//ignition is established
{
 engineUsageTimeOld=millis(); //initialize engine use counter
if(RPMAvg<ignitionRPMHigh) startMotorTarget++ ;  //first get to ignitionRPMHigh
else if (!preheatCompleted) preheatDelayFunction();
else if (RPMAvg<starterOffRPM)//ignition is established and starter is not yet off 
{

  if (startMotorTarget<outMax)
      {
        //Ramp up starter RPM while keeping Temp at Starting Temp
        if(exTemp>startingTemp) 
        {
          startMotorTarget++;
          if ((exTemp+(2*tempGrad))>maxTemp) fuelFlowTarget--;    //2 sec look ahead
        }
        else if (tempGrad<=0)fuelFlowTarget++;
      }
    else //  if start motor is maxed out
       { 
        //Keep increasing fuel flow but keep temperature in check. 
        //If fuel flow increase temperature above max temp and RPM is below self sustaining RPM and starter motor is already at 100% 
        // then starter motor is not giving the RPM to reach self sustaining mode.Engine will not accelerate further and 
        //will not reach self sustaining RPM
        
        if (exTemp<maxTemp)fuelFlowTarget++;
             else fuelFlowTarget--;
      }
}
else if (RPMAvg>starterOffRPM)
      
{
    fuelFlowTarget++;//Give a little kick as starter will shut off and now it will switch to idling mode
    startMotorTarget=outMin;
    startMotorNow=outMin;
    engineMode=modeIdling; 
    engineStartCount++;  //Increment engine usage counter
}

       }
      }
     }
    }
    
     void NoRPMKeroStartFunction()
  {
     //glow plug operation
      if (glowPower==outMin) 
      {
        keroStartTime=millis();//Start the kero plug heating Timer
        glowPower=keroPower;
      }
    

    //Fuel flow operation --gas flow function is used as this is proven code and gas is used to describe fuel through glow plug
    if ((glowPower==keroPower) &&(gasFlow==false)&&(keroStartTime>0)&&((millis()-keroStartTime)>(keroTime*1000))) 
    {
      gasFlow=true; //start the kero valve at given settings
      gasOnTime=millis();
      fuelFlowTarget=keroPump;//Start the fuel pump at given settings
      
    }
    if ((gasFlow)&&((millis()-gasOnTime)>noIgnitionThreshold)&&(!ignitionState) )
    {
      AbortAll();
       //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
     bitSet(errorCode,0);//No Ignition
      gasOnTime=0;//Start time of fuel flow which is by default 5 seconds after kero plug is switched on
      keroStartTime=0; //Start time of Kero Plug heating
    }
        }
 void KeroStartFunction()
  {
     //glow plug operation
    if ((RPMAvg>=glowOnRPM)&&(RPMAvg<glowOffRPM))
    {
      if (glowPower==outMin) 
      {
        keroStartTime=millis();//Start the kero plug heating Timer
        glowPower=keroPower;
      }
    }
    else  glowPower=outMin;

    //Fuel flow operation --gas flow function is used as this is proven code and gas is used to describe fuel through glow plug
    if ((glowPower==keroPower) &&(gasFlow==false)&&(RPMAvg>gasOnRPM)&&(RPMAvg<gasOffRPM)&&(keroStartTime>0)&&((millis()-keroStartTime)>(keroTime*1000))) 
    {
      gasFlow=true; //start the kero valve at given settings
      gasOnTime=millis();
      fuelFlowTarget=keroPump;//Start the fuel pump at given settings
     
    }
    if ((gasFlow)&&((millis()-gasOnTime)>noIgnitionThreshold)&&(!ignitionState) )
    {
      AbortAll();
       //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
     bitSet(errorCode,0);//No Ignition
      gasOnTime=0;//Start time of fuel flow which is by default 5 seconds after kero plug is switched on
      keroStartTime=0; //Start time of Kero Plug heating
    }
   
    if ((RPMAvg>gasOffRPM)|| (RPMAvg<gasOnRPM))
    {
      gasFlow=false;
      keroStartTime=0;
      gasOnTime=0;
    }
      }

      
void preheatDelayFunction()
{
 if(preheatDelayTimeOld==0) preheatDelayTimeOld=millis();
 else if((millis()-preheatDelayTimeOld)>preheatDelay)
 {
  preheatCompleted=true;
  preheatDelayTimeOld=0;
 }
}


      void IdlingFunction()
      {
         if(startRpmControl) RPMIdlingFunction();
 else if(startPressureControl) NoRPMIdlingFunction();
 else if(startThrottleControl) NoRPMIdlingFunction();
      }
  void NoRPMIdlingFunction(){

  //for brushless motor we dont depend on RPM to shut down gas and glow plug in starting functions
  gasFlow=false;
  gasOnTime=0;
  keroStartTime=0;
  glowPower=outMin;
  //Also switch off starter motor just in case
  startMotorTarget=outMin;
  startMotorNow=outMin;
   
  if (rcModeSignal<20) engineMode=modeCooldown;
  else{
    if(startPressureControl){
float targetPress=idlePress+ int(round(((maxPress-idlePress)*rcThrottleSignal)/100));
if((casePress<maxPress)&&(casePress<targetPress)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if pressure conditions and exhaust temp<max Temp
else if ((casePress>(targetPress+pressTolerance))||(casePress>maxPress)||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (casePress>(idlePress+(2*pressTolerance))) engineMode=modeOperating;
}
else if (startThrottleControl){
  int targetThrottle=idleFuel+ int(round(((maxFuel-idleFuel)*rcThrottleSignal)/100));
if((fuelFlowTarget<maxFuel)&&(fuelFlowTarget<targetThrottle)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if fuel Flow and exhaust temp<max Temp
else if ((fuelFlowTarget>(targetThrottle+fuelTolerance))||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (fuelFlowTarget>(idleFuel+(2*fuelTolerance))) engineMode=modeOperating;
}
}
  }
  

void RPMIdlingFunction(){

  //for brushless motor we dont depend on RPM to shut down gas and glow plug in starting functions
  gasFlow=false;
  gasOnTime=0;
  keroStartTime=0;
  glowPower=outMin;
  //Also switch off starter motor just in case
  startMotorTarget=outMin;
  startMotorNow=outMin;
   
  if (rcModeSignal<20) engineMode=modeCooldown;
  else{
int targetRPM=idleRPM+ int(round(((maxRPM-idleRPM)*rcThrottleSignal)/100));
if((RPMAvg<maxRPM)&&(RPMAvg<targetRPM)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if Average RPM< max RPM and exhaust temp<max Temp
else if ((RPMAvg>(targetRPM+rpmTolerance))||(RPMAvg>maxRPM)||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (RPMAvg>(idleRPM+3000)) engineMode=modeOperating;
}
}
void OperatingFunction(){
  if(runRpmControl) RPMOperatingFunction();
 else if(runPressureControl) NoRPMOperatingFunction();
 else if(runThrottleControl) NoRPMOperatingFunction();
  
}
void NoRPMOperatingFunction(){
if (rcModeSignal<20) AbortAll();
else
{    if(startPressureControl){
float targetPress=idlePress+ int(round(((maxPress-idlePress)*rcThrottleSignal)/100));
if((casePress<maxPress)&&(casePress<targetPress)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if pressure conditions and exhaust temp<max Temp
else if ((casePress>(targetPress+pressTolerance))||(casePress>maxPress)||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (casePress<(idlePress+(2*pressTolerance))) engineMode=modeIdling;
}
else if (startThrottleControl){
  int targetThrottle=idleFuel+ int(round(((maxFuel-idleFuel)*rcThrottleSignal)/100));
if((fuelFlowTarget<maxFuel)&&(fuelFlowTarget<targetThrottle)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if fuel Flow and exhaust temp<max Temp
else if ((fuelFlowTarget>(targetThrottle+fuelTolerance))||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (fuelFlowTarget<(idleFuel+(2*fuelTolerance))) engineMode=modeIdling;
}
}
}


void RPMOperatingFunction(){
if (rcModeSignal<20) AbortAll();
else
{
int targetRPM=idleRPM+ int(round(((maxRPM-idleRPM)*rcThrottleSignal)/100));
if((RPMAvg<maxRPM)&&(RPMAvg<targetRPM)&&(exTemp<maxTemp)) fuelFlowTarget++;//only increase fuel flow if Average RPM< max RPM and exhaust temp<max Temp
else if ((RPMAvg>(targetRPM+rpmTolerance))||(RPMAvg>maxRPM)||(exTemp>maxTemp)) fuelFlowTarget--;
else fuelFlowTarget=fuelFlowNow;
if (RPMAvg<(idleRPM+3000)) engineMode=modeIdling;
}
}
void CooldownFunction(){
  if(startRpmControl) RPMCooldownFunction();
 else if(startPressureControl) NoRPMCooldownFunction();
 else if(startThrottleControl) NoRPMCooldownFunction();
}
void NoRPMCooldownFunction(){
 if(fuelFlowTarget>outMin) fuelFlowTarget=outMin;//Shut down fuel flow

 if ((startPressureControl)&&(casePress<idlePress)&&(exTemp>ignitionThreshold))//Run starter motor till temperature is below ignition temperature
 {
//   fuelFlowTarget=outMin;
   startMotorTarget=purgeThrottle;
 }
 else if ((startThrottleControl)&&(tempGrad>(-50))&&(exTemp>ignitionThreshold))//Run starter motor till temperature is below ignition temperature
 {
//   fuelFlowTarget=outMin;
   startMotorTarget=purgeThrottle;
 }

if (exTemp<ignitionThreshold)startStageTimeOld=millis();//cool down till ignition detection temperature and initialize the purge Timer
       if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)>purgeTime))//determine if end of Purge cycle reached
 
      {
        startStageTimeOld=0;//Reset timer
       startMotorTarget=outMin;
        fuelFlowTarget=outMin;
       fuelFlowNow=outMin;
       engineMode=modeWaiting;
       
      }

}

void RPMCooldownFunction(){
 if(fuelFlowTarget>outMin) fuelFlowTarget=outMin;//Shut down fuel flow

 if ((RPMAvg<12000)&&(exTemp>ignitionThreshold))//Run starter motor till temperature is below ignition temperature
 {
//   fuelFlowTarget=outMin;
   startMotorTarget=purgeThrottle;
 }

if (exTemp<ignitionThreshold)startStageTimeOld=millis();//cool down till ignition detection temperature and initialize the purge Timer
       if ((startStageTimeOld>0)&&((millis()-startStageTimeOld)>purgeTime))//determine if end of Purge cycle reached
 
      {
        startStageTimeOld=0;//Reset timer
       startMotorTarget=outMin;
       fuelFlowTarget=outMin;
       fuelFlowNow=outMin;
       engineMode=modeWaiting;
       
      }

}
void CheckFailure()
{
  
 //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
 
 if  (exTemp>maxTemp) 
 { 
  bitSet(errorCode,1);//Max Temp Exceeded
    if (rcModeSignal<20) AbortAll();
else  if (fuelFlowTarget>5)  fuelFlowTarget=fuelFlowTarget-5;
else{
  fuelFlow=false;
  fuelFlowTarget=outMin;
}
 }
 else bitClear(errorCode,1);//1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
 if ((runRpmControl||startRpmControl)&&(RPMAvg>maxRPM)) 
 {
  bitSet(errorCode,2);//Max RPM Exceeded
  if (rcModeSignal<20) AbortAll();
 else if (fuelFlowTarget>5) fuelFlowTarget=fuelFlowTarget-5;
 else {
  fuelFlow=false;
  fuelFlowTarget=outMin;
 }
 }
  else bitClear(errorCode,2);

   //if no RC Signal within RCSignalMaxTime
 if ((millis()>RCSignalmillis)&&((millis()-RCSignalmillis)>RCSignalMaxTime) )
 {
  if (ignitionState){
     //1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
  bitSet(errorCode,3);//No RC Signal
  AbortAll();}
 }
 else 
 bitClear(errorCode,3);
 //RPM Sensor Failure
 if (pumpOnValue>outMin)
 {
if((!pumpPrime)&&(runRpmControl||startRpmControl)&&(RPMAvg==0)&&(fuelFlowTarget>pumpOnValue))
{
   bitSet(errorCode,5);//RPM Failure
  AbortAll();
}
 }
 else
 {
if((!pumpPrime)&&(runRpmControl||startRpmControl)&&(RPMAvg==0)&&(fuelFlowTarget>outMin))
{
   bitSet(errorCode,5);//RPM Failure
  AbortAll();
}
 }

if((!pumpPrime)&&(runRpmControl||startRpmControl)&&(RPMAvg==0)&&(startMotorNow>((outMax-outMin)/4)))//Throttle is higher than 25% of scale and RPM is zero
{
   bitSet(errorCode,5);//RPM Failure
  AbortAll();
}
if((engineMode==modeOperating)&&(runRpmControl||startRpmControl)&&(RPMAvg<idleRPM))//RPM incorrect as per mode
{
   bitSet(errorCode,5);//RPM Failure
  AbortAll();
}
if((engineMode==modeIdling)&&(runRpmControl||startRpmControl)&&(RPMAvg<(starterOffRPM*0.7)))//RPM incorrect as per mode--setting limit at 30% RPM drop
{
   bitSet(errorCode,5);//RPM Failure
  AbortAll();
}
if((engineMode==modeIdling)&&(startPressureControl)&&(fuelFlowNow>idleFuel)&&(casePress<(idlePress/2)))//Pressure incorrect as per mode
{
   bitSet(errorCode,7);
  AbortAll();
}
 
 }


void AbortAll()
{ 
  sysMsg="";
  sysMsg+="Error Code, ";
  sysMsg+=errorCode;
 // sysMsg+='\n'; 
  //take snapshot of critical data for analysis
  abRPMAvg=RPMAvg;
  abexTemp=exTemp;
  abfuelFlowTarget=fuelFlowTarget; //output signal for fuel pump  0-1000
  abfuelFlowNow=fuelFlowNow; //output signal for fuel pump  0-1000
  abstartMotorTarget=startMotorTarget;// output signal for starter motor 0-1000
  abstartMotorNow=startMotorNow;// output signal for starter motor 0-1000
  abgasFlow=gasFlow; //gas valve for starting
  abfuelFlow=fuelFlow; //fuel valve for starting
  abignitionState=ignitionState;
  abrcModeSignal=rcModeSignal;
  abrcThrottleSignal=rcThrottleSignal;
  abglowPower=glowPower;//Stop Glow plug if On
  abengineMode=engineMode;
  abstartStage=startStage;
  abvoltage=voltage;
  aberrorCode=errorCode;

  
  fuelFlowTarget=outMin; //output signal for fuel pump  0-1000
  fuelFlowNow=outMin; //output signal for fuel pump  0-1000
  startMotorTarget=outMin;// output signal for starter motor 0-1000
  startMotorNow=outMin;// output signal for starter motor 0-1000
  gasFlow=false; //gas valve for starting
  fuelFlow=false;//Solenoid Fuel Valve
  glowPower=outMin;//Stop Glow plug if On

  engineMode=modeWaiting;
  startStage=startStage0;
  preheatCompleted=false;
  preheatDelayTimeOld=0;
  
  fuelServo.write(MIN_MICROS);
  startServo.write(MIN_MICROS);
  
  digitalWrite(Fuel_Solenoid_Pin,LOW);
  gasOnTime=0;
  keroStartTime=0;
  if (keroStart)
  {
    ledcWrite(valvepwmchannel,0);
    ledcWrite(glowpwmchannel,0);
  }
   else
   {
    glowServo.write(MIN_MICROS);
     digitalWrite(GasPin,LOW);
   }
}

void ControlOutput()
{
 //Check Limits and stop runaway increment or decrement condition
 if (fuelFlowTarget>outMax) fuelFlowTarget=outMax;
   else if (fuelFlowTarget<outMin) fuelFlowTarget=outMin;
   
   if (pumpOnValue>outMin)
  {
  if ((fuelFlowNow<pumpOnValue)&&(fuelFlowTarget>outMin)) 
  {
    fuelFlowTarget=pumpOnValue;
    fuelFlowNow=pumpOnValue;
  }
  if (fuelFlowTarget==outMin) fuelFlowNow=outMin;
 //if (fuelFlowNow<pumpOnValue)  fuelFlowNow=(pumpOnValue-1);//required when system starts up as initially fuelFlowNow will be zero
   }
     
      
 if (fuelFlowTarget>outMin)
 {
  if (fuelFlowTarget>(fuelFlowNow+1)) fuelFlowTarget=fuelFlowNow+1;// never exceed more than One point
  if (fuelFlowTarget<(fuelFlowNow-1)) fuelFlowTarget=fuelFlowNow-1;
 }

 
 if (startMotorTarget>outMin)
{
  if (startMotorTarget>(startMotorNow+1)) startMotorTarget=startMotorNow+1;
  if (startMotorTarget<(startMotorNow-1)) startMotorTarget=startMotorNow-1;
}
/*
//scale according to motor voltage
 if (voltage>0) {
    startMotorTarget=int(round(startMotorTarget*(maxMotorVolt/voltage)));//Scale motor output according to battery used
   fuelFlowTarget=int(round(fuelFlowTarget*(maxPumpVolt/voltage)));//Scale fuel pump output according to battery used
  }
 */
  
  
            
    if (startMotorTarget>outMax) startMotorTarget=outMax;
    if (startMotorTarget<outMin) startMotorTarget=outMin;
    if (glowPower>outMax) glowPower=outMax;
    if (glowPower<outMin) glowPower=outMin;
    
     
   //Fuel Flow Solenoid control
   if(!keroStart) //Gas start mode
   {
   if ((!pumpPrime)&&(pumpOnValue>outMin)&&(fuelFlowNow>pumpOnValue)) fuelFlow=true;
  else if ((!pumpPrime)&&(pumpOnValue==outMin)&&(fuelFlowNow>outMin)) fuelFlow=true;
   else fuelFlow=false;
   }
   else //kero Start mode
   {
    if ((!pumpPrime)&&(fuelFlowNow>keroPump)) fuelFlow=true;  ///Fuel Flow valve control in kero mode
     else fuelFlow=false;
   }
 
   if ((!ignitionState)&&(pumpPrime)) //in case ECU is in pump prime mode and no ignition
   {
   
    fuelFlowNow=fuelFlowTarget;
    fuelServo.write(map(fuelFlowNow,outMin,outMax,MIN_MICROS,MAX_MICROS));
     digitalWrite(Fuel_Solenoid_Pin,LOW);//Keep Fuel Solenoid off to stop fuel from going to engine
   }
 //fuel flow will be controlled with different delays for RPM below idle and RPM above idle as the response time is different in both cases
 else if (((ignitionState)||(keroStart))&&((engineMode==modeStarting)||(engineMode==modeIdling)||(engineMode==modeCooldown)))//handling low RPM operation while engine running
  {
      if ((fuelFlowTarget>fuelFlowNow)&&((millis()-fuelFlowTimeOld)>lowAccelDelay)&&(tempGrad<maxTempGrad)&&((exTemp+(3*tempGrad))<maxTemp) )//fuel pump control with acceleration delay and temp gradient control
  {
    fuelFlowNow++;
  fuelServo.write(map(fuelFlowNow,outMin,outMax,MIN_MICROS,MAX_MICROS));//Servo command takes a value of min micros-max micros
  fuelFlowTimeOld=millis();
  }
  if ((fuelFlowTarget<fuelFlowNow)&&((millis()-fuelFlowTimeOld)>lowDecelDelay) )//fuel pump control with deceleration delay
  {
    fuelFlowNow--;
  fuelServo.write(map(fuelFlowNow,outMin,outMax,MIN_MICROS,MAX_MICROS));
  fuelFlowTimeOld=millis();
  }
  }
else if ((ignitionState)&&(engineMode==modeOperating))    //handle fuel flow above Idle RPM 
{
  if ((fuelFlowTarget>fuelFlowNow)&&((millis()-fuelFlowTimeOld)>accelerationDelay)&&(tempGrad<maxTempGrad)&&((exTemp+(3*tempGrad))<maxTemp))//fuel pump control with acceleration delay
  {
    fuelFlowNow++;
  fuelServo.write(map(fuelFlowNow,outMin,outMax,MIN_MICROS,MAX_MICROS));
  fuelFlowTimeOld=millis();
  }
  if ((fuelFlowTarget<fuelFlowNow)&&((millis()-fuelFlowTimeOld)>decelerationDelay) )//fuel pump control with deceleration delay
  {
    fuelFlowNow--;
    fuelServo.write(map(fuelFlowNow,outMin,outMax,MIN_MICROS,MAX_MICROS));//Servo command takes a value of 0-180
  fuelFlowTimeOld=millis();
  }
}
if(startMotorTarget>starterTransition)
{
  if ((millis()-startMotorTimeOld)>starterIncDelay) //start motor control loop after starter
  {
    if (startMotorTarget>startMotorNow) startMotorNow++;
      if (startMotorTarget<startMotorNow) startMotorNow--;
   startServo.write(map(startMotorNow,outMin,outMax,MIN_MICROS,MAX_MICROS));
  startMotorTimeOld=millis();
  }
  
  }
  else
  {
     if (startMotorTarget>startMotorNow) startMotorNow++;
      if (startMotorTarget<startMotorNow) startMotorNow--;
   startServo.write(map(startMotorNow,outMin,outMax,MIN_MICROS,MAX_MICROS));
  startMotorTimeOld=millis();
  }
if(!testGlowPlug)
{
if(!keroStart)  glowServo.write(map(glowPower,outMin,outMax,MIN_MICROS,MAX_MICROS)); 
else {
  if(glowPower==keroPower) ledcWrite(glowpwmchannel, keroPower); //power up kero valve if gasFlow variable is true
  else ledcWrite(glowpwmchannel, 0);//power off kero valve 
}
}

if(!testGasSolenoid)
{
if(!keroStart) //Gas flow solenoid valve for Gas Start
{ 
 if(gasFlow) digitalWrite(GasPin,HIGH);
 else digitalWrite(GasPin,LOW);
}
else //Kero Start case--Kero Start fuel Flow valve connected between pump and kero plug fuel supply
{
  if(gasFlow) 
  {
    ledcWrite(valvepwmchannel, keroValve); //pulse the kero valve
 if( fuelFlowTarget<keroPump) fuelFlowTarget=keroPump; //start pump for kero startup
  }
 else  ledcWrite(valvepwmchannel,0);

}
}
if(!testFuelSolenoid)
{
 if(fuelFlow) digitalWrite(Fuel_Solenoid_Pin,HIGH);
 else digitalWrite(Fuel_Solenoid_Pin,LOW);
}
    
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  DynamicJsonDocument readings(2048);
  if (sdavailable&&writefile) readings["datafilename"] = filename;
  else if (!sdavailable) readings["datafilename"] = "File Write Error";
  else if (!writefile) readings["datafilename"] = "Data recording stopped";
  readings["temperature"] = String(exTemp);
   readings["casepressure"] = String(casePress);
   readings["pressOffset"] = String(pressOffset);
  readings["tempGrad"] = String(tempGrad);
  readings["rpm"] =  String(RPMAvg/1000.0); //Send RPM for gauge by dividing with 1000
  readings["throttle"]=String(rcThrottleSignal);
  readings["modecontrol"]=String(rcModeSignal);
  readings["fuel"]=String(fuelFlowNow);//use String(fuelFlowNow/1.0);  to show in fractions
  readings["starter"]=String(startMotorNow);
  readings["glow"]=String(glowPower);
  readings["batvolt"]=String(voltage);
  readings["loadcellvalue"]=String(loadcellvalue);
  char tempbuffer[16];
  char tempbuffer2[16];
 ltoa(sdWriteTime,tempbuffer,10);
readings["sdlooptime"]= tempbuffer; 
ltoa(maxLoopTime,tempbuffer2,10);
readings["looptime"]= tempbuffer2;
  maxLoopTime=0;//reset maxLoopTime
  if (gasFlow) readings["gas"]="On";
  else readings["gas"]="Off";
  if (engineMode==modeWaiting)readings["ecumode"]="Auto Start";
else if ((engineMode==modeStarting)&&(startStage==startStage0)) readings["ecumode"]="Purge";
else if ((engineMode==modeStarting)&&(preheatDelayTimeOld>0)) readings["ecumode"]="Preheat";
else if ((engineMode==modeStarting)&&(startStage==startStage1))  readings["ecumode"]="Ramp Up";
else if (engineMode==modeIdling)  readings["ecumode"]="Idling";
else if (engineMode==modeOperating)  readings["ecumode"]="Operating";
else if (engineMode==modeCooldown)  readings["ecumode"]="Cooldown";

 if (pumpPrime) readings["thrmode"]="Pump";
 else readings["thrmode"]="Starter";
//1 = no Ignition,2=max temp exceeded,4=max RPM exceeded, 8=no RC signal,16=Flameout,32=RPM Sensor Failure , 64= No Fuel Failure,128=Unable to reach Idle RPM with full motor power//use by setting and reading bits
  if(bitRead(errorCode,0))  readings["error"]="No ignition";
else if(bitRead(errorCode,1))  readings["error"]="Temp Exceeded";
else if(bitRead(errorCode,2))  readings["error"]="RPM Exceeded";
else if(bitRead(errorCode,3))  readings["error"]="RC Signal Lost";
else if(bitRead(errorCode,4)) readings["error"]="Flameout";
else if(bitRead(errorCode,5)) readings["error"]="RPM Sensor Failure";
else if(bitRead(errorCode,6)) readings["error"]="Unable to accelerate on Fuel";
else if(bitRead(errorCode,7)) readings["error"]="Unable to reach Idle RPM";
else readings["error"]="No Error";
  
 
  String jsonString;
  serializeJsonPretty(readings,jsonString);
  return jsonString;
}

//Following functions are working but not used as they are consuming excessive time and slow down the main loop
    // Write the sensor readings on the SD card
void LogSDCard() {
 
 appendFile(SD_MMC,filename,Data.substring(0,1024).c_str());//The substring() method returns the part of the string from the start index up to and excluding the end index, or to the end of the string if no end index is supplied.
   Data.remove(0,1024);    //public string Remove (int startIndex, int count);
   //Data=""; //Empty data string
 
}
void PrintDataHeader()
{
headerData="";
headerData+="Time(s)";
headerData+=",";
headerData+="RPM";
headerData+=",";
headerData+="RPM Gradient(RPM/s)";
headerData+=",";
headerData+="Amb Temp(C)";
headerData+=",";
headerData+="Exhaust Temp(C)";
headerData+=",";
headerData+="Temp Gradient(C/s)";
headerData+=",";
headerData+="Case Pressure(kPa)";
headerData+=",";
headerData+="Thrust";
headerData+=",";
headerData+="Throttle";
headerData+=",";
headerData+="ModeSwitch";
headerData+=",";
headerData+="Engine Mode";
headerData+=",";
headerData+="Starter";
headerData+=",";
headerData+="FuelValve";
headerData+=",";
headerData+="Fuel";
headerData+=",";
headerData+="Glow";
headerData+=",";
headerData+="GAS";
headerData+=",";
headerData+="Voltage";
headerData+=",";
headerData+="Error";
headerData+=",";
headerData+="Loop Time";
headerData+=",";
headerData+="SD Write Time";
headerData+=",";
headerData+="Message";
headerData+='\n';
Serial.print(headerData);
  }
void SaveDataHeader()
{
PrintDataHeader();
if(sdavailable) appendFile(SD_MMC,filename,headerData.c_str());
Data=""; //Empty data string
  }
void updateSerialData()//
{
if (serialData.length()==0){
String newData;
newData="";
float x=millis()/1000.0;
newData+=x;
newData+=",";
newData+=RPMAvg;
newData+=",";
newData+=rpmGrad;
newData+=",";
newData+=ambTemp;
newData+=",";
newData+=exTemp;
newData+=",";
newData+=tempGrad;
newData+=",";
newData+=casePress;
newData+=",";
newData+=loadcellvalue;
newData+=",";
newData+=rcThrottleSignal;
newData+=",";
newData+=rcModeSignal;
newData+=",";
if (engineMode==modeWaiting )  newData+="Wait";   // 0  //No RPM //AutoStart
else if ((engineMode==modeStarting)&&(startRpmControl)) newData+="Start_RPM Control";
else if ((engineMode==modeStarting)&&(startPressureControl)) newData+="Start_Pressure Control";
else if ((engineMode==modeStarting)&&(startThrottleControl)) newData+="Start_Throttle Control";
else if ((engineMode==modeStarting)&&(preheatDelayTimeOld>0)) newData+="Preheat";
else if ((engineMode==modeIdling)&&(startRpmControl)) newData+="Idling_RPM Control";
else if ((engineMode==modeIdling)&&(startPressureControl)) newData+="Idling_Pressure Control";
else if ((engineMode==modeIdling)&&(startThrottleControl)) newData+="Idling_Throttle Control";
else if ((engineMode== modeOperating)&&(runRpmControl)) newData+="Operating_RPM Control";
else if ((engineMode== modeOperating)&&(runPressureControl)) newData+="Operating_Pressure Control";
else if ((engineMode== modeOperating)&&(runThrottleControl)) newData+="Operating_Throttle Control";
else if (engineMode==modeCooldown) newData+="Cooling";
//Data+=engineMode;
newData+=",";
newData+=startMotorNow;
newData+=",";
if (fuelFlow)newData+="ON";
else newData+="OFF";
newData+=",";
newData+=fuelFlowNow;
newData+=",";
newData+=glowPower;
newData+=",";
if (gasFlow)newData+="ON";
else newData+="OFF";
newData+=",";
newData+=voltage;
newData+=",";
newData+=errorCode;
newData+=",";
newData+=measuredLoopTime;
newData+=",";
newData+=sdWriteTime;
newData+=",";
newData+=sysMsg;
newData+='\n';
serialData=newData;
  }
}

  
void SaveData()//Called from Main Loop
{
float x=millis()/1000.0;
 if(( millis()-sdLoopTimeOld)>sdLoopTime)
  {
newData="";
newData+=x;
newData+=",";
newData+=RPMAvg;
newData+=",";
newData+=rpmGrad;
newData+=",";
newData+=ambTemp;
newData+=",";
newData+=exTemp;
newData+=",";
newData+=tempGrad;
newData+=",";
newData+=casePress;
newData+=",";
newData+=loadcellvalue;
newData+=",";
newData+=rcThrottleSignal;
newData+=",";
newData+=rcModeSignal;
newData+=",";
if (engineMode==modeWaiting )  newData+="Wait";   // 0  //No RPM //AutoStart
else if ((engineMode==modeStarting)&&(startRpmControl)) newData+="Start_RPM Control";
else if ((engineMode==modeStarting)&&(startPressureControl)) newData+="Start_Pressure Control";
else if ((engineMode==modeStarting)&&(startThrottleControl)) newData+="Start_Throttle Control";
else if ((engineMode==modeStarting)&&(preheatDelayTimeOld>0)) newData+="Preheat";
else if ((engineMode==modeIdling)&&(startRpmControl)) newData+="Idling_RPM Control";
else if ((engineMode==modeIdling)&&(startPressureControl)) newData+="Idling_Pressure Control";
else if ((engineMode==modeIdling)&&(startThrottleControl)) newData+="Idling_Throttle Control";
else if ((engineMode== modeOperating)&&(runRpmControl)) newData+="Operating_RPM Control";
else if ((engineMode== modeOperating)&&(runPressureControl)) newData+="Operating_Pressure Control";
else if ((engineMode== modeOperating)&&(runThrottleControl)) newData+="Operating_Throttle Control";
else if (engineMode==modeCooldown) newData+="Cooling";

//Data+=engineMode;
newData+=",";
newData+=startMotorNow;
newData+=",";
if (fuelFlow)newData+="ON";
else newData+="OFF";
newData+=",";
newData+=fuelFlowNow;
newData+=",";
newData+=glowPower;
newData+=",";
if (gasFlow)newData+="ON";
else newData+="OFF";
newData+=",";
newData+=voltage;
newData+=",";
newData+=errorCode;
newData+=",";
newData+=measuredLoopTime;
newData+=",";
newData+=sdWriteTime;
newData+=",";
newData+=sysMsg;
newData+='\n';
Data+=newData;
sysMsg="";//Clear sysMsg so it does not repeat without a trigger
if((sdavailable)) 
{ 
  if (Data.length()>1024)
{
 // Serial.print(Data.length());
//Serial.print(" ");
  sdWriteTime=millis();
  LogSDCard(); 
  sdWriteTime=millis()- sdWriteTime; //calculating time to write to SD card
  if(serverStarted){
  if (sdWriteTime>(sdMaxTime+100))
 { sdMaxCount++;
  if (sdMaxCount>1) 
  {
    sdavailable=false; //if SD write takes more than sdMAxTime then stop writing to SD Card. With AsyncWeb Server running, the server consumes 100ms looptime
//     digitalWrite(LEDPin_2,HIGH);
  }
    }
  else sdMaxCount=0;
  }
  
  else if(sdWriteTime>(sdMaxTime)) 
  {
     sdMaxCount++;
    if (sdMaxCount>1) 
    {
      sdavailable=false; //if SD write takes more than sdMAxTime then stop writing to SD Card
//       digitalWrite(LEDPin_2,HIGH);    
    }
    
  }
  else sdMaxCount=0;
  //Serial.println(sdWriteTime);
}
}

sdLoopTimeOld=millis();

  }
     
}
 void initPulseCounter (){                                    // initialise pulse counter
    pcnt_config_t pcntFreqConfig = { };                        // Instance of pulse counter
    pcntFreqConfig.pulse_gpio_num = RPMsensorPin;            // pin assignment for pulse counter 
    pcntFreqConfig.pos_mode = PCNT_COUNT_INC;                  // count Rising edges (=change from high to low logical level) as pulses
    pcntFreqConfig.counter_h_lim = PCNT_H_LIM_VAL;             // set upper limit of counting 
    pcntFreqConfig.unit = PCNT_FREQ_UNIT;                      // select ESP32 pulse counter unit 0
    pcntFreqConfig.channel = PCNT_CHANNEL_0;                   // select channel 0 of pulse counter unit 0
    pcnt_unit_config(&pcntFreqConfig);                         // configur rigisters of the pulse counter
  
    pcnt_counter_pause(PCNT_FREQ_UNIT);                        // pause puls counter unit
    pcnt_counter_clear(PCNT_FREQ_UNIT);                        // zero and reset of pulse counter unit
  
    pcnt_event_enable(PCNT_FREQ_UNIT, PCNT_EVT_H_LIM);         // enable event for interrupt on reaching upper limit of counting
    pcnt_isr_register(CounterOverflow, NULL, 0, &user_isr_handle);  // configure register overflow interrupt handler
    pcnt_intr_enable(PCNT_FREQ_UNIT);                          // enable overflow interrupt

    pcnt_set_filter_value(PCNT_FREQ_UNIT, PCNT_FILTER_VAL);    // set damping, inertia 
    pcnt_filter_enable(PCNT_FREQ_UNIT);                        // enable counter glitch filter (damping)
  
    pcnt_counter_resume(PCNT_FREQ_UNIT);                       // resume counting on pulse counter unit
  }
  
  void Read_Reset_PCNT() {                                     // function for reading pulse counter (for timer)
    pcnt_get_counter_value(PCNT_FREQ_UNIT, &PulseCounter);     // get pulse counter value - maximum value is 16 bits

    // resetting counter as if example, delet for application in PiedPiperS
    OverflowCounter = 0;                                       // set overflow counter to zero
    pcnt_counter_clear(PCNT_FREQ_UNIT);                        // zero and reset of pulse counter unit
    //conterOK = true;                                         // not in use, copy from example code ########################################
  }

  void Read_PCNT() {                                           // function for reading pulse counter (for timer)
    pcnt_get_counter_value(PCNT_FREQ_UNIT, &PulseCounter);     // get pulse counter value - maximum value is 16 bits
  }


  void SaveUsageData()
 {
  int tempStartCount;
  preferences.begin("settings",false);
  tempStartCount=preferences.getInt("engineStarts",engineStartCount);
  if(engineStartCount>tempStartCount) preferences.putInt("engineStarts",engineStartCount);
   preferences.putInt("runTime",engineRunTime);
   preferences.end();
 }
 
 void UpdateUsageData()
 {
if ((millis()-engineUsageTimeOld)>saveUsageTime)
{
   engineRunTime=engineRunTime+20;
   engineUsageTimeOld=millis();
   SaveUsageData();
 }

 }
 void initSD()
 {
  int x=0;
 bool result=false;

  do{ result=SD_MMC.begin();
  x++;}
    while ((!result)&&(x<5));
 //  if(!SD_MMC.begin()) {
 if(!result) {
    Serial.println("Card Mount Failed");
    sdavailable=false;
   // return;
  } else 
  {
    Serial.println("Card Mount Success");
    sdavailable=true;

  }
  
if(!dirpresent)listDir(SD_MMC, "/", 0);
 
  if(file) file.close();
 do {
  if(file) file.close();
  fileseq++;
  char seq[4];
  itoa(fileseq,seq,10);
  String fnametemp="/"+ssid+"/Data"+seq+".txt";
  fnametemp.toCharArray(filename,32);
  String settingsfnametemp="/"+ssid+"/Settings"+seq+".txt";
  settingsfnametemp.toCharArray(settingsfilename,32);
 // Serial.println(filename);
 file = SD_MMC.open(filename);
  } while(file);
   // saveSettingsToFile();//save parameters
    file = SD_MMC.open(filename,FILE_APPEND);
    SaveDataHeader();
  if (file){ 
   sdavailable=true;
   Serial.print("File opened ");
  Serial.println(filename);
//  digitalWrite(LEDPin_2,LOW);
 // file.close();
 } else 
 {
  Serial.print("Failed to open file");
  sdavailable=false;
//  digitalWrite(LEDPin_2,HIGH);
 }
 
   }


//Delete a file from the SD, it is called in void SD_dir()
void SD_file_delete(String deletefilename) 
{ 
  if (sdavailable) { 
  
    File dataFile = SD_MMC.open(deletefilename, FILE_READ); //Now read data from SD Card 
    if (dataFile)
    {
      dataFile.close();
      if (SD_MMC.remove(deletefilename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+deletefilename+"' has been erased</h3>"; 
        webpage += F("<br><br><br><a href='/page9'>[Back]</a><br><br>");
            }
      else
      { 
        webpage += F("<br><br><br><h3>File was not deleted - error</h3>");
        webpage += F("<br><br><br><a href='/'>[Back]</a><br><br>");
      }
    } else  Serial.println(F("File deleted successfully"));
  
  } else  Serial.println(F("File Not Present"));
} 

void SD_deleteAll(void) 
{
  sdavailable=false;
   //  File file;
  fileseq=0;
 do {
  if(file) 
  {
  file.close();
//  digitalWrite(LEDPin_2,HIGH);
   SD_MMC.remove(filename);
   Serial.println("deleted "+String(filename));
   }
  fileseq++;
  char seq[4];
  itoa(fileseq,seq,10);
  String fnametemp="/"+ssid+"/Data"+seq+".txt";
  fnametemp.toCharArray(filename,32);
 // Serial.println(filename);
 file = SD_MMC.open(filename);
  } while(file);
  //REmoving Settings files
   fileseq=0;
do {
  if(file) 
  {
  file.close();
//  digitalWrite(LEDPin_2,HIGH);
   SD_MMC.remove(filename);
     Serial.println("deleted "+String(filename));
   }
  fileseq++;
  char seq[4];
  itoa(fileseq,seq,10);
  String fnametemp="/"+ssid+"/Settings"+seq+".txt";
  fnametemp.toCharArray(filename,32);
 // Serial.println(filename);
 file = SD_MMC.open(filename);
  } while(file);
  
  fileseq=0;
  initSD();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
   
 
    File root=fs.open(dirname);
  
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (0==strcmp(file.name(),ssid.c_str())) {
              dirpresent=true;
              Serial.print(file.name());
            }
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
       //     Serial.print("  FILE: ");
       //     Serial.print(file.name());
        //    Serial.print("  SIZE: ");
        //    Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    String path="/"+ssid;
    if(!dirpresent) {
      createDir(SD_MMC,path.c_str());
      Serial.printf("new directory created %s", ssid);
    }
    else Serial.println("Directory exists "+ path);
}


void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
   // Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
     //   Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
      file.flush();
       // Serial.println("File written");
    } else {
       // Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
 //   Serial.printf("Appending to file: %s\n", path);

 //   File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        sdavailable=false;
        return;
    }
    if(file.print(message)){
            file.flush();
    
     // Serial.println("Message appended ");
   //     file.close();
            } else {
     //   Serial.println("Append failed");
        sdavailable=false;
    }
}

void saveSettingsToFile()
{

    file = SD_MMC.open(settingsfilename,FILE_APPEND);
    
  Serial.println(settingsfilename);
String settingsdata="";
settingsdata+="RPM Sensor Type";
settingsdata+=",";
if (cpr==1)settingsdata+="Magnetic";
else settingsdata+="Optical";
settingsdata +='\n';
settingsdata+="maxRPM";
settingsdata+=",";
settingsdata+=maxRPM;
settingsdata +='\n';
settingsdata+="maxTemp";
settingsdata+=",";
settingsdata+=maxTemp;
settingsdata +='\n';
settingsdata+="idleRPM";
settingsdata+=",";
settingsdata+=idleRPM;
settingsdata +='\n';
settingsdata+="rpmTolerance";
settingsdata+=",";
settingsdata+=rpmTolerance;
settingsdata +='\n';
settingsdata+="glowOnRPM";
settingsdata+=",";
settingsdata+=glowOnRPM;
settingsdata +='\n'; 
settingsdata+="glowOffRPM";
settingsdata+=",";
settingsdata+=glowOffRPM;
settingsdata +='\n';
settingsdata+="ignitionRPMHigh";
settingsdata+=",";
settingsdata+=ignitionRPMHigh;
settingsdata +='\n';
settingsdata+="ignitionRPMLow";
settingsdata+=",";
settingsdata+=ignitionRPMLow;
settingsdata +='\n';
settingsdata+="gasOnRPM";
settingsdata+=",";
settingsdata+=gasOnRPM;
settingsdata +='\n';   
settingsdata+="gasOffRPM";
settingsdata+=",";
settingsdata+=gasOffRPM;
settingsdata +='\n';
settingsdata+="starterOffRPM";
settingsdata+=",";
settingsdata+=starterOffRPM;
settingsdata +='\n'; 
settingsdata+="fuelOnRPM";
settingsdata+=",";
settingsdata+=fuelOnRPM;
settingsdata +='\n';  
settingsdata+="pumpOnValue";
settingsdata+=",";
settingsdata+=pumpOnValue;
settingsdata +='\n';   
settingsdata+="purgeThrottle";
settingsdata+=",";
settingsdata+=purgeThrottle;
settingsdata +='\n';  
settingsdata+="purgeTime";
settingsdata+=",";
settingsdata+=purgeTime;
settingsdata +='\n';
settingsdata+="starterIncDelay";
settingsdata+=",";
settingsdata+=starterIncDelay;
settingsdata +='\n';   
settingsdata+="noIgnitionThreshold";
settingsdata+=",";
settingsdata+=noIgnitionThreshold;
settingsdata +='\n';
settingsdata+="ignitionThreshold";
settingsdata+=",";
settingsdata+=ignitionThreshold;
settingsdata +='\n';
settingsdata+="fuelFlowDetectionTime";
settingsdata+=",";
settingsdata+=fuelFlowDetectionTime;
settingsdata +='\n';
settingsdata+="fuelFlowDetectionRPM";
settingsdata+=",";
settingsdata+=fuelFlowDetectionRPM;
settingsdata +='\n';
settingsdata+="startingTemp";
settingsdata+=",";
settingsdata+=startingTemp;
settingsdata +='\n';
settingsdata+="maxTempGrad";
settingsdata+=",";
settingsdata+=maxTempGrad;
settingsdata +='\n';
settingsdata+="MIN_MICROS";
settingsdata+=",";
settingsdata+=MIN_MICROS;
settingsdata +='\n';
settingsdata+="MAX_MICROS";
settingsdata+=",";
settingsdata+=MAX_MICROS;
settingsdata +='\n';
settingsdata+="accelerationDelay";
settingsdata+=",";
settingsdata+=accelerationDelay;
settingsdata +='\n';
settingsdata+="decelerationDelay";
settingsdata+=",";
settingsdata+=decelerationDelay;
settingsdata +='\n'; 
settingsdata+="lowAccelDelay";
settingsdata+=",";
settingsdata+=lowAccelDelay;
settingsdata +='\n';  
settingsdata+="lowDecelDelay";
settingsdata+=",";
settingsdata+=lowDecelDelay;
settingsdata +='\n'; 
settingsdata+="keroStart";
settingsdata+=",";
if(keroStart) settingsdata +="1";
else settingsdata+="0";
settingsdata +='\n'; 
settingsdata+="keroPower";
settingsdata+=",";
settingsdata+=keroPower;
settingsdata +='\n'; 
settingsdata+="keroTime";
settingsdata+=",";
settingsdata+=keroTime;
settingsdata +='\n'; 
settingsdata+="keroValve";
settingsdata+=",";
settingsdata+=keroValve;
settingsdata +='\n'; 
settingsdata+="keroFreq";
settingsdata+=",";
settingsdata+=keroFreq;
settingsdata +='\n'; 
settingsdata+="keroPump";
settingsdata+=",";
settingsdata+=keroPump;
settingsdata +='\n'; 
settingsdata+="idlePress"; // Pressure limit to switch to idle mode
settingsdata+=",";
settingsdata+=idlePress; // Pressure limit to switch to idle mode
settingsdata +='\n'; 
settingsdata+="maxPress"; // maximum Pressure limit
settingsdata+=",";
settingsdata+=maxPress; // maximum Pressure limit
settingsdata +='\n'; 
settingsdata+="pressTolerance"; // pressure control tolerance
settingsdata+=",";
settingsdata+=pressTolerance; // pressure control tolerance
settingsdata +='\n'; 
settingsdata+="idleFuel"; // Fuel Flow to switch to idle mode
settingsdata+=",";
settingsdata+=idleFuel; // Fuel Flow to switch to idle mode
settingsdata +='\n'; 
settingsdata+="maxFuel"; // maximum Fuel Flow limit
settingsdata+=",";
settingsdata+=maxFuel; // maximum Fuel Flow limit
settingsdata +='\n'; 
settingsdata+="fuelTolerance"; // Fuel Flow control tolerance limit
settingsdata+=",";
settingsdata+=fuelTolerance; // Fuel Flow control tolerance limit
settingsdata +='\n'; 
settingsdata+="pressOffset"; // zero Offset
settingsdata+=",";
settingsdata+=pressOffset; // zero Offset
settingsdata +='\n'; 
settingsdata+="startControl"; // 
settingsdata+=",";
if(startPressureControl)settingsdata +="SPressure" ;
else if (startRpmControl) settingsdata +="SRpm";
else settingsdata +="SThrottle"; 
settingsdata +='\n'; 
settingsdata+="runControl"; // 
settingsdata+=",";
if(runPressureControl)settingsdata +="RPressure" ;
else if (runRpmControl) settingsdata +="RRpm";
else settingsdata +="RThrottle"; 
settingsdata +='\n'; 
settingsdata+="serialData"; // 
settingsdata+=",";
if(startSerial) settingsdata +="startSerial";
else settingsdata +="stopSerial";
settingsdata +='\n'; 
settingsdata+="ignitionThrottle"; // 
settingsdata+=",";
settingsdata+=ignitionThrottle;
settingsdata +='\n'; 
settingsdata+="throttleHighTime";
settingsdata+=",";
settingsdata+=throttleLowTime;
settingsdata +='\n'; 
settingsdata+="throttleLowTime";
settingsdata+=",";
settingsdata+=throttleLowTime;
settingsdata +='\n'; 
settingsdata+="useCustomCal";
settingsdata+=",";
if(useCustomCal)settingsdata+="true";
else settingsdata+="false";
settingsdata +='\n';
settingsdata+="calPress0";
settingsdata+=",";
settingsdata+=calPress0;
settingsdata +='\n'; 
settingsdata+="calPress1";
settingsdata+=",";
settingsdata+=calPress1;
settingsdata +='\n'; 
settingsdata+="calVolt0";
settingsdata+=",";
settingsdata+=calVolt0;
settingsdata +='\n'; 
settingsdata+="calVolt1";
settingsdata+=",";
settingsdata+=calVolt1;
settingsdata +='\n'; 
settingsdata+="enableLoadCell";
settingsdata+=",";
if(loadCellEnable) settingsdata+="true";
else settingsdata+="false";
settingsdata +='\n'; 
settingsdata+="preheatDelay";
settingsdata+=",";
settingsdata+=preheatDelay;
settingsdata +='\n'; 
appendFile(SD_MMC,settingsfilename,settingsdata.c_str());
//file.flush();
file.close();
}

void loadSettingsFromFile()
{

 Serial.println("File Opening for load");
    Serial.println("/"+ssid+"/"+newsettingsfilename);
 File   newfile = SD_MMC.open("/"+ssid+"/"+newsettingsfilename,FILE_READ);
    while(newfile.available()) {
      String inBuffer = newfile.readStringUntil('\n');
     String parameter = inBuffer.substring(0,inBuffer.indexOf(','));
String val = inBuffer.substring(inBuffer.indexOf(',')+1);
Serial.print(parameter);
Serial.print("  ");
Serial.println(val);
        if (parameter=="RPM Sensor Type")
        {
          if (val=="Magnetic") cpr=1;
          else cpr=2;        
        }
        if (parameter=="maxRPM") maxRPM=val.toInt();
        if (parameter=="maxTemp") maxTemp=val.toInt();
        if (parameter=="idleRPM") idleRPM=val.toInt();
        if (parameter=="rpmTolerance") rpmTolerance=val.toInt();
        if (parameter=="glowOnRPM") glowOnRPM=val.toInt();
        if (parameter=="glowOffRPM") glowOffRPM=val.toInt();
        if (parameter=="ignitionRPMHigh") ignitionRPMHigh=val.toInt();
        if (parameter=="ignitionRPMLow") ignitionRPMLow=val.toInt();
        if (parameter=="gasOnRPM") gasOnRPM=val.toInt();
        if (parameter=="gasOffRPM") gasOffRPM=val.toInt();
        if (parameter=="starterOffRPM") starterOffRPM=val.toInt();
        if (parameter=="fuelOnRPM") fuelOnRPM=val.toInt();
        if (parameter=="pumpOnValue") pumpOnValue=val.toInt();
        if (parameter=="purgeThrottle") purgeThrottle=val.toInt();
        if (parameter=="purgeTime") purgeTime=val.toInt();
        if (parameter=="starterIncDelay") starterIncDelay=val.toInt();
        if (parameter=="noIgnitionThreshold") noIgnitionThreshold=val.toInt();
        if (parameter=="ignitionThreshold") ignitionThreshold=val.toInt();
        if (parameter=="fuelFlowDetectionTime") fuelFlowDetectionTime=val.toInt();
        if (parameter=="fuelFlowDetectionRPM") fuelFlowDetectionRPM=val.toInt();
        if (parameter=="startingTemp") startingTemp=val.toInt();
        if (parameter=="maxTempGrad") maxTempGrad=val.toInt();
        if (parameter=="MIN_MICROS") MIN_MICROS=val.toInt();
        if (parameter=="MAX_MICROS") MAX_MICROS=val.toInt();
        if (parameter=="accelerationDelay") accelerationDelay=val.toInt();
        if (parameter=="decelerationDelay") decelerationDelay=val.toInt();
        if (parameter=="lowAccelDelay") lowAccelDelay=val.toInt();
        if (parameter=="lowDecelDelay") lowDecelDelay=val.toInt();
        if(parameter=="keroStart")
            {
              if (val=="1") keroStart=true;
              else keroStart=false;
            }
        if(parameter=="keroPower")keroPower=val.toInt();//0-1000.   810 to provide 6v with 7.4 v Lipo battery
        if(parameter=="keroTime") keroTime=val.toInt(); //time in second default 5 seconds 0-60 seconds
        if(parameter=="keroValve") keroValve=val.toInt();//0-1000
        if(parameter=="keroFreq") keroFreq=val.toInt(); // kero valve frequency 0-60
        if(parameter=="keroPump") keroPump=val.toInt(); //0-1000
        if(parameter=="idlePress") idlePress=val.toFloat(); // Pressure limit to switch to idle mode
        if(parameter=="maxPress") maxPress=val.toFloat(); // Pressure limit to switch to idle mode
        if(parameter=="pressTolerance") pressTolerance=val.toInt(); // Pressure limit to switch to idle mode
        if(parameter=="idleFuel") idleFuel=val.toInt(); // Pressure limit to switch to idle mode
        if(parameter=="maxFuel") maxFuel=val.toInt(); 
        if(parameter=="fuelTolerance") fuelTolerance=val.toInt(); 
        if(parameter=="pressOffset") pressOffset=val.toFloat(); 
        if(parameter=="startControl") 
        {
          if (val=="SPressure") 
          {
            startPressureControl=true;
            startRpmControl=false;
            startThrottleControl=false;
          }
          else if (val=="SRpm")
          {
            startRpmControl=true;
            startPressureControl=false;
            startThrottleControl=false;
          }
          else 
          {
            startThrottleControl=true;
             startPressureControl=false;
            startRpmControl=false;
          }
        }
        if(parameter=="runControl") 
        {
          if (val=="RPressure") 
          {
            runPressureControl=true;
            runThrottleControl=false;
            runRpmControl=false;
          }
          else if (val=="RRpm") 
          {
            runRpmControl=true;
            runPressureControl=false;
            runThrottleControl=false;
          }
          else 
          {
            runThrottleControl=true;
            runRpmControl=false;
            runPressureControl=false;
          }
        }
if(parameter=="serialData") 
        {
          if (val=="startSerial") startSerial=true;
          else  startSerial=false;
          }
          if(parameter=="ignitionThrottle") ignitionThrottle=val.toInt();
          if(parameter=="throttleHighTime") throttleHighTime=val.toInt();
          if(parameter=="throttleLowTime") throttleLowTime=val.toInt();  
          if(parameter=="useCustomCal"){
          if (val=="true")useCustomCal=true;
          else useCustomCal=false;
          }

if(parameter=="calPress0") calPress0=val.toFloat();
if(parameter=="calPress1") calPress1=val.toFloat();
if(parameter=="calVolt0") calVolt0=val.toFloat();
if(parameter=="calVolt1") calVolt1=val.toFloat();
    if(parameter=="enableLoadCell"){
          if (val=="true")loadCellEnable=true;
          else loadCellEnable=false;
          }
if(parameter=="preheatDelay") preheatDelay=val.toInt();  

//reset the strings
        parameter="";
        val="";
      }
      newfile.close();
    }

void handleFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  int    start,uploadtime,uploadsize;
  if (!index) {
    String file = filename;
    if (!filename.startsWith("/")) file = "/"+ssid +"/" + filename;
    request->_tempFile = SD_MMC.open(file, "w");
    if (!request->_tempFile) Serial.println("Error creating file for upload...");
 start = millis();
  }
  if (request->_tempFile) {
    if (len) {
      request->_tempFile.write(data, len); // Chunked data
      Serial.println("Transferred : " + String(len) + " Bytes");
    }
    if (final) {
  uploadsize = request->_tempFile.size();
      request->_tempFile.close();
  uploadtime = millis() - start;
  strcpy(newsettingsfilename, filename.c_str());
     // request->redirect("/dir");
    }
  }
}
String page9processor(const String& var){
  //Serial.println(var);
  if(var == "PAGE9PLACEHOLDER"){
    String webpage="";
    String path="/"+ssid;
     File root = SD_MMC.open(path);
     if(!root) 
     {
      webpage+=" SD Card not available";
      return webpage;
     }
      if(!root.isDirectory()){
        Serial.println("Root failure");
   webpage+=" SD Card not available";
      return webpage;
  }
  webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name</th><th>Size</th></tr>");
  File dirfile = root.openNextFile();
   // int i = 0;
  while(dirfile){
 
    if(dirfile.isDirectory()){
 //     webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
    Serial.println("Directory file");
//     printDirectory(file.name(), levels-1);
    }
      else
    {
       Serial.println(dirfile.name());
      webpage += "<tr><td>"+String(dirfile.name())+"</td>";
  //  webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = dirfile.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,1)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,1)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,1)+" GB";
      webpage += "<td>"+fsize+"</td>";
      webpage += "<td>";
    webpage += F("<FORM action='/download' method='get'>"); 
        webpage += F("<button type='submit' name='download'"); 
          webpage += F("' value='"); webpage +=String(dirfile.name()); webpage +=F("'>Download</button>");
         webpage += F("</FORM>"); 
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/delete' method='get'>"); 
      webpage += F("<button type='submit' name='delete'"); 
           webpage += F("' value='"); webpage +=String("/"+ssid+"/"+dirfile.name()); webpage +=F("'>Delete</button>");
         webpage += F("</FORM>"); 
      webpage += "</td>";
    //  webpage += "</tr>";

    }
    dirfile = root.openNextFile();
   // i++;
  }
  dirfile.close();
    return webpage;
  }
  return String();
}

void WebServerFunction()
{
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); // Set WiFi RF power output to lowest level
  WiFi.softAP(ssid.c_str(), password.c_str());
  
   delay(200);
    WiFi.softAPConfig(local_IP, local_IP, subnet);
    delay(500);
   Serial.println(ssid.c_str());
      Serial.println(password.c_str());
     Serial.println(WiFi.softAPIP());
        serverStarted=true; //WebServer has been started
        digitalWrite(LEDPin_1,LOW);
     //  u8g2.clearBuffer();
       //    u8g2.drawStr(1,30,"Starting Wifi WebServer"); 
       //u8g2.sendBuffer();
       delay(3000);
//send webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
// request->send_P(200, "text/html", index_html);
  request->send_P(200, "text/html", index_html, page1processor);
});
//display data terminal with realtime values
 server.on("/page2", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage2);
});

//display data terminal with realtime values
 server.on("/page3", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage3);
});
//display data terminal with realtime values
 server.on("/page4", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage4);
});
//display data terminal with realtime values
 server.on("/page5", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage5);
});
//display data terminal with realtime values
 server.on("/page6", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage6);
});
//display engine lifetime data
 server.on("/page7", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send_P(200, "text/html", htmlPage7);
});
//display dir and download page
 server.on("/page8", HTTP_GET, [](AsyncWebServerRequest *request){
 
 request->send_P(200, "text/html",htmlPage8);
});
server.on("/page9", HTTP_GET, [](AsyncWebServerRequest *request){
 
 request->send_P(200, "text/html",htmlPage9,page9processor);
});
server.on("/toggleloadCellEnable",HTTP_GET, [](AsyncWebServerRequest *request){
loadCellEnable=!loadCellEnable;

 preferences.begin("settings",false);
 preferences.putBool("loadEnable",loadCellEnable);
 preferences.end();
 Serial.println("Load cell switched");
});
 
server.on("/useCusCal",HTTP_GET, [](AsyncWebServerRequest *request){
  if (engineMode==modeWaiting){
useCustomCal=true;
 pressOffset=0;
 preferences.begin("settings",false);
 preferences.putBool("customCal",useCustomCal);
 preferences.end();
 Serial.println("Using Custom Pressure Calibration");
  }
 request->send(200, "text/plane",htmlPage2);
});
server.on("/useDefCal",HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 useCustomCal=false;
 pressOffset=0;
 preferences.begin("settings",false);
 preferences.putBool("customCal",useCustomCal);
 preferences.end();
  Serial.println("Using Default Pressure Calibration");
   }
 request->send(200, "text/plane",htmlPage2);
});
server.on("/testFuel", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
  testFuelSolenoid=true;  
  testmillis=millis();  
   }
  request->send(200, "text/html", htmlPage6);   
    });
    server.on("/testGas", HTTP_GET, [](AsyncWebServerRequest *request){
       if (engineMode==modeWaiting){
     testGasSolenoid=true;
     testmillis=millis();
       }
     request->send(200, "text/html", htmlPage6);
    });
    server.on("/testGlow", HTTP_GET, [](AsyncWebServerRequest *request){
       if (engineMode==modeWaiting){
     testGlowPlug=true;
     testmillis=millis();
      Serial.println("Button Pressed");
       }
     request->send(200, "text/html", htmlPage6);
    });
      server.on("/stopTestGlow", HTTP_GET, [](AsyncWebServerRequest *request){
       if (engineMode==modeWaiting){
     testGlowPlug=false;
     testmillis=0;
     Serial.println("Button Released");
       }
     request->send(200, "text/html", htmlPage6);
    });
 server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request){
 
 Serial.println("Taring Now");
  tareconst= scale.read_average(5);
      Serial.println(tareconst);
      scale.tare();
      scale.set_offset(tareconst);
       taredone=true;
//Save values to EEPROM
  preferences.begin("settings",false);
  taredone=preferences.putBool("TareDone",taredone);
  tareconst=preferences.putFloat("TareConst",tareconst);
  preferences.end();
   request->send(200, "text/html", "Parameters updated as per input fields<br><a href=\"/\">Return to Home Page</a>");
       
    });
//start/Stop Serial data transmission
 server.on("/toggleSerialEnable", HTTP_GET, [](AsyncWebServerRequest *request){
startSerial=!startSerial;
if (startSerial) PrintDataHeader();
 preferences.begin("settings",false);
 preferences.putBool("startSerial",startSerial);
 preferences.end();
request->send(200, "text/html", htmlPage3);
});
//Zero the pressure sensor
 server.on("/zeroPressure", HTTP_GET, [](AsyncWebServerRequest *request){
 pressOffset=casePress;
 preferences.begin("settings",false);
 preferences.putFloat("pressOffset",pressOffset);
 preferences.end();
request->send(200, "text/html", htmlPage2);
});
//display data terminal with realtime values
 server.on("/ModeNormal", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
  AbortAll();
   pumpPrime=false;
   engineMode= modeWaiting;
   }
request->send(200, "text/html", htmlPage6);
});

//Set pump prime mode so throttle stick controls the pump
 server.on("/PumpPrime", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 pumpPrime=true;
   }
request->send(200, "text/html", htmlPage6);
});
//Reset Error code 
 server.on("/ResetError", HTTP_GET, [](AsyncWebServerRequest *request){
  errorCode=0;//reset error code
 request->send(200, "text/html", htmlPage6);

});
//Save Settings to SD card
 server.on("/SaveSettings", HTTP_GET, [](AsyncWebServerRequest *request){
 saveSettingsToFile(); 
 request->send(200, "text/html", htmlPage8);

});
//Stop recording will stop recording data 
 server.on("/StopRecording", HTTP_GET, [](AsyncWebServerRequest *request){
  writefile=false;
    //SD_MMC.close(file);
   // digitalWrite(LEDPin_2,HIGH);
 
 request->send(200, "text/html", htmlPage9);

});
//Start recording will restart recording data on the current file 
 server.on("/StartRecording", HTTP_GET, [](AsyncWebServerRequest *request){
   writefile=true;
 //  if(file)  digitalWrite(LEDPin_2,LOW);
     
  request->send(200, "text/html", htmlPage9);
});
//New File will open a new file and start recording data on the new file 
 server.on("/NewFile", HTTP_GET, [](AsyncWebServerRequest *request){
  if(file) file.flush();
  initSD();
  request->send(200, "text/html", htmlPage9);
});
//Delete all files and start from a new data file
 server.on("/DeleteAll", HTTP_GET, [](AsyncWebServerRequest *request){
 SD_deleteAll();
  request->send(200, "text/html", htmlPage8);
});

 server.on("/startrpmcontrol", HTTP_GET, [](AsyncWebServerRequest *request){
  if (engineMode==modeWaiting)
  {
  startRpmControl=true;
 startThrottleControl=false;
startPressureControl=false;
  preferences.begin("settings",false);
 preferences.putBool("spressControl",startPressureControl);
    preferences.putBool("srpmControl",startRpmControl);
  preferences.putBool("sthrControl",startThrottleControl);
  preferences.end();
  Serial.print("Control set to rpm control");
   request->send(200, "text/html", htmlPage3);
  }
});
server.on("/startpressurecontrol", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting)
  {
  startRpmControl=false;
 startThrottleControl=false;
startPressureControl=true;
  preferences.begin("settings",false);
 preferences.putBool("spressControl",startPressureControl);
    preferences.putBool("srpmControl",startRpmControl);
  preferences.putBool("sthrControl",startThrottleControl);
  preferences.end();
  Serial.print("Control set to pressure control");
   request->send(200, "text/html", htmlPage3);
  }
});
server.on("/startthrottlecontrol", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting)
  {
  startRpmControl=false;
 startThrottleControl=true;
startPressureControl=false;
  preferences.begin("settings",false);
 preferences.putBool("spressControl",startPressureControl);
    preferences.putBool("srpmControl",startRpmControl);
  preferences.putBool("sthrControl",startThrottleControl);
  preferences.end();
  Serial.print("Control set to throttle control");
   request->send(200, "text/html", htmlPage3);
  }
});
server.on("/runrpmcontrol", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting)
  {
  runRpmControl=true;
 runThrottleControl=false;
runPressureControl=false;
  preferences.begin("settings",false);
 preferences.putBool("rpressControl",runPressureControl);
    preferences.putBool("rrpmControl",runRpmControl);
  preferences.putBool("rthrControl",runThrottleControl);
  preferences.end();
  Serial.print("Control set to rpm control");
   request->send(200, "text/html", htmlPage3);
  }
});
server.on("/runpressurecontrol", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting)
  {
  runRpmControl=false;
 runThrottleControl=false;
runPressureControl=true;
  preferences.begin("settings",false);
preferences.putBool("rpressControl",runPressureControl);
    preferences.putBool("rrpmControl",runRpmControl);
  preferences.putBool("rthrControl",runThrottleControl);;
  preferences.end();
  Serial.print("Run Control set to pressure control");
   request->send(200, "text/html", htmlPage3);
  }
});
server.on("/runthrottlecontrol", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting)
  {
  runRpmControl=false;
 runThrottleControl=true;
runPressureControl=false;
  preferences.begin("settings",false);
 preferences.putBool("rpressControl",runPressureControl);
    preferences.putBool("rrpmControl",runRpmControl);
  preferences.putBool("rthrControl",runThrottleControl);
  preferences.end();
  Serial.print("Run Control set to throttle control");
   request->send(200, "text/html", htmlPage3);
  }
});
//Startup type set to Gas start
 server.on("/gasstart", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 keroStart=false;
ledcDetachPin(GlowPin);
ledcDetachPin(GasPin);
  glowServo.attach(GlowPin, MIN_MICROS, MAX_MICROS);
  preferences.begin("settings",false);
 preferences.putBool("kero",keroStart);
  preferences.end();
  Serial.print("Startup set to gas start");
   }
   request->send(200, "text/html", htmlPage3);
});
//Startup type set to Kero start
 server.on("/kerostart", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 keroStart=true;
 glowServo.detach();
 ledcAttachPin(GasPin,valvepwmchannel);// Solenoid valve will be used with keroFreq to meter fuel in keroplug fuel supply
 ledcWrite(valvepwmchannel, 0);
 ledcAttachPin(GlowPin,glowpwmchannel);// Solenoid valve will be used with keroFreq to meter fuel in keroplug fuel supply
 ledcWrite(glowpwmchannel, 0);
 
  preferences.begin("settings",false);
 preferences.putBool("kero",keroStart);
  preferences.end();
  Serial.print("Startup set to Kero start");
   }
   request->send(200, "text/html", htmlPage3);
});
//RPM Sensor type hall
 server.on("/hall", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 cpr=1;
  preferences.begin("settings",false);
 preferences.putInt("cpr",cpr);
  preferences.end();
  Serial.print("RPM Counts Per Revolution set to...");
  Serial.println(String(cpr));
   }
  request->send(200, "text/html", htmlPage3);
});
//RPM Sensor type optical
 server.on("/optical", HTTP_GET, [](AsyncWebServerRequest *request){
   if (engineMode==modeWaiting){
 cpr=2;
  preferences.begin("settings",false);
 preferences.putInt("cpr",cpr);
  preferences.end();
  Serial.print("RPM Counts Per Revolution set to...");
  Serial.println(String(cpr));
   }
  request->send(200, "text/html", htmlPage3);
});
 server.on("/download", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Downloading file...");
 
    if (request->hasParam("download")) {
     String downloadfilename = request->getParam("download")->value();
     Serial.println(downloadfilename);
     File file2 = SD_MMC.open("/"+ssid+"/"+downloadfilename, "r");//adding root path
     unsigned long downloadsize = file2.size();
     
      //String contentType = "text/plain";
      String contentType = "application/octet-stream";
     
      AsyncWebServerResponse *response = request->beginResponse(contentType, downloadsize, [file2](uint8_t *buffer, size_t maxLen, size_t total) mutable ->  size_t
                                                               { 
       return file2.read(buffer, maxLen); });
    //  response->setContentType("application/octet-stream");//for downloading file
    downloadfilename.toUpperCase();
     // String fname="\"" + downloadfilename + "\"";
     String fname=downloadfilename;
    //  response->addHeader( "Content-Disposition","attachment;downloadfilename="+fname);
  response->addHeader( "Content-Disposition","attachment;filename="+fname);
      //response->addHeader("Server", "ESP Async Web Server");
         request->send(response);
         
   }
 //   request->send(200, "text/html", htmlPage9);
  
  });
   server.on("/delete", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Deleting file...");
   // Select_File_For_Function("[DELETE]", "deletehandler"); // Build webpage ready for display
   if (request->hasParam("delete")) {
     String deletefilename= request->getParam("delete")->value();
     Serial.println(deletefilename);
     SD_file_delete(deletefilename);
    }
    request->send(200, "text/html",  htmlPage9);
  });


//display  value of ECU parameters at abort time
 server.on("/usage",HTTP_GET, [](AsyncWebServerRequest *request){
  String usagedata;
 DynamicJsonDocument usagedoc(256);
 usagedoc["startCount"] = String(startCount);
 usagedoc["engineStartCount"] = String(engineStartCount);
 usagedoc["runTime"] = String(engineRunTime);
 serializeJsonPretty(usagedoc,usagedata);
 request->send(200, "text/plane", usagedata);
});
server.on("/resetECU",HTTP_GET, [](AsyncWebServerRequest *request){
 startCount=0;
 preferences.begin("settings",false);
 preferences.putInt("startCount",startCount);
 preferences.end();
  request->send(200, "text/html",htmlPage8);
});
server.on("/resetengcnt",HTTP_GET, [](AsyncWebServerRequest *request){
  
 engineStartCount=0;
 preferences.begin("settings",false);
 preferences.putInt("engineStarts",engineStartCount);
  preferences.end();
 request->send(200, "text/html",htmlPage8);
});

server.on("/resetengtm",HTTP_GET, [](AsyncWebServerRequest *request){
  
 engineRunTime=0;
 preferences.begin("settings",false);
 preferences.putInt("runTime",engineRunTime);
 preferences.end();
 request->send(200, "text/plane",htmlPage8);
});


//display  value of ECU parameters at abort time
 server.on("/abortdata",HTTP_GET, [](AsyncWebServerRequest *request){
  String abortdata;
 DynamicJsonDocument abortdoc(1024);
 abortdoc["abexTemp"] = String(abexTemp);
 abortdoc["abRPMAvg"] = String(abRPMAvg);
 abortdoc["abfuelFlowNow"] = String(abfuelFlowNow);
 abortdoc["abstartMotorNow"] = String(abstartMotorNow);
 abortdoc["abgasFlow"] = String(abgasFlow);
 abortdoc["abfuelFlow"] = String(abfuelFlow);
 abortdoc["abignitionState"] = String(abignitionState);
 abortdoc["abrcModeSignal"] = String(abrcModeSignal);
 abortdoc["abrcThrottleSignal"] = String(abrcThrottleSignal);
 abortdoc["abglowPower"] = String(abglowPower);
 abortdoc["abengineMode"] = String(abengineMode);
 abortdoc["abstartStage"] = String(abstartStage);
 abortdoc["abvoltage"] = String(abvoltage);
 abortdoc["aberrorCode"] = String(aberrorCode);   
        
 serializeJsonPretty(abortdoc,abortdata);
 request->send(200, "text/plane", abortdata);
});
//display current value of engine parameters
 server.on("/param",HTTP_GET, [](AsyncWebServerRequest *request){
  //JSON for sending engine parameters through Webserver
DynamicJsonDocument doc(8192);

doc["maxTemp"] = String(maxTemp);
doc["maxTempGrad"] = String(maxTempGrad);
doc["startingTemp"]=String (startingTemp);
doc["preheatDelay"]=String (preheatDelay);
 doc["pumpOnValue"]=String(pumpOnValue);
 doc["voltIn"]=String(voltage);
 doc["purgeTime"]=String(purgeTime);
 doc["purgeThrottle"]=String(purgeThrottle);
 doc["starterIncDelay"]=String(starterIncDelay);
 doc["accelerationDelay"]=String(accelerationDelay);
 doc["decelerationDelay"]=String(decelerationDelay);
 doc["lowAccelDelay"]=String(lowAccelDelay);
 doc["lowDecelDelay"]=String(lowDecelDelay);
 doc["ignitionThreshold"]=String(ignitionThreshold);
 doc["noIgnitionThreshold"]=String(noIgnitionThreshold);
  doc["MIN_MICROS"]=String(MIN_MICROS);
 doc["MAX_MICROS"]=String(MAX_MICROS);
 doc["WiFiSSID"]=String(ssid);
 doc["WiFiPWD"]=String(password);
   String paramdata;
 serializeJsonPretty(doc,paramdata);
 request->send(200, "text/plane", paramdata);
});
//display current value of engine parameters
 server.on("/parampage2",HTTP_GET, [](AsyncWebServerRequest *request){
  //JSON for sending engine parameters through Webserver
DynamicJsonDocument docpage2(8192);
docpage2["maxRPM"] = String(maxRPM);
 docpage2["idleRPM"]=String(idleRPM);
 docpage2["rpmTolerance"]=String(rpmTolerance);
 docpage2["glowOnRPM"] = String(glowOnRPM);
 docpage2["glowOffRPM"] = String(glowOffRPM);
 docpage2["ignitionRPMHigh"]=String(ignitionRPMHigh);
 docpage2["ignitionRPMLow"]=String(ignitionRPMLow);
 docpage2["gasOnRPM"]=String(gasOnRPM);
 docpage2["gasOffRPM"]=String(gasOffRPM);
 docpage2["starterOffRPM"]=String(starterOffRPM);
 docpage2["fuelOnRPM"]=String(fuelOnRPM);
docpage2["idlePress"]=String(idlePress);
docpage2["maxPress"]=String(maxPress);
docpage2["pressTolerance"]=String(pressTolerance);
docpage2["casePress"]=String(casePress);
docpage2["ignThrottle"]=String(ignitionThrottle);
docpage2["thrDelay"]=String(throttleHighTime);
docpage2["thrDelay2"]=String(throttleLowTime);
docpage2["idleFuel"]=String(idleFuel);
docpage2["maxFuel"]=String(maxFuel);
docpage2["fuelTolerance"]=String(fuelTolerance);
docpage2["cpr"]=String(cpr);
if(useCustomCal) docpage2["pressCalType"]=String(1); //use custom Calibration
else  docpage2["pressCalType"]=String(2); //use default calibration
docpage2["calPress0"]=String(calPress0);
docpage2["calPress1"]=String(calPress1);
docpage2["calVolt0"]=String(calVolt0);
docpage2["calVolt1"]=String(calVolt1);

 String paramdatapage2;
 serializeJsonPretty(docpage2,paramdatapage2);
 request->send(200, "text/plane", paramdatapage2);
});
server.on("/parampage3",HTTP_GET, [](AsyncWebServerRequest *request){
  //JSON for sending engine parameters through Webserver
DynamicJsonDocument docpage3(8192);
  
if(startRpmControl) docpage3["startcontroltype"]=1; //RPM Control
else if(startPressureControl) docpage3["startcontroltype"]=2; //Pressure Control
else docpage3["startcontroltype"]=3;//Throttle control
if(runRpmControl) docpage3["runcontroltype"]=1; //RPM Control
else if(runPressureControl) docpage3["runcontroltype"]=2; //Pressure Control
else docpage3["runcontroltype"]=3;//Throttle control
 docpage3["cpr"]=String(cpr);
 if (!keroStart) docpage3["starttype"]=1; //gas start
 else docpage3["starttype"]=2; //kero start
 docpage3["keroPower"]=String(keroPower);
 docpage3["keroTime"]=String(keroTime);
 docpage3["keroValve"]=String(keroValve);
 docpage3["keroFreq"]=String(keroFreq);
 docpage3["keroPump"]=String(keroPump);
 docpage3["loadcellvalue"]=String(loadcellvalue);
  if(startSerial) docpage3["enableSerial"]=true;
 else docpage3["enableSerial"]=false;
 if(loadCellEnable) docpage3["enableLoadCell"]=true;
 else docpage3["enableLoadCell"]=false;
 String paramdatapage3;
 serializeJsonPretty(docpage3,paramdatapage3);
 request->send(200, "text/plane", paramdatapage3);
});


 // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
   String json = getSensorReadings();
  request->send(200, "application/json", json);
  json = String();
  });
  // Request for the latest sensor readings
  server.on("/keepalive", HTTP_GET, [](AsyncWebServerRequest *request){
    keepalivecounter=millis();//update keep alive counter to keep transmitting event messages
    });
      // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_ServerState)) {
      inputMessage = request->getParam(PARAM_ServerState)->value();
      inputParam = PARAM_ServerState;    
    }
   
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
   
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
     if(inputMessage=="0")
    {
      Serial.println("stopping server");
      serverStarted=false;
      startServer=false;//donot start WiFi server in main loop
    dnsServer.stop();
    server.end();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
    digitalWrite(LEDPin_1,HIGH);
    }
  });

 // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "true");
  });
  
  server.on("/upload", HTTP_POST, [] (AsyncWebServerRequest *request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,
     size_t len, bool final) {
    handleFileUpload(request, filename, index, data, len, final);//save this file to sd card
    Serial.println("File saved on card");
    loadSettingsFromFile();//load settings from new file
    Serial.println("Values Read from File");
    WriteSettings(); //Write settings to flash memory
    Serial.println("Values saved to Flash");
    request->send(200, "text/html", webpage);
    
  });
  
//server.onNotFound(notFound);  
server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
   
    // GET input values
   
    if (request->hasParam(PARAM_maxTemp)) {
      inputmaxTemp = request->getParam(PARAM_maxTemp)->value();
      if (inputmaxTemp.length()>0){
          maxTemp=inputmaxTemp.toInt();
          Serial.println("max Temp Updated to "+inputmaxTemp);
     preferences.begin("settings",false);
     preferences.putInt("maxTemp",maxTemp);
     preferences.end();
    }}

    if (request->hasParam(PARAM_maxRPM)) {
      inputmaxRPM = request->getParam(PARAM_maxRPM)->value();
      if (inputmaxRPM.length()>0){
          maxRPM=inputmaxRPM.toInt();
          Serial.println("max RPM Updated to "+inputmaxRPM);
     preferences.begin("settings",false);
     preferences.putInt("maxRPM",maxRPM);
     preferences.end();
    }}
     if (request->hasParam(PARAM_maxTempGrad)) {
      inputmaxTempGrad = request->getParam(PARAM_maxTempGrad)->value();
      if (inputmaxTempGrad.length()>0){
          maxTempGrad=inputmaxTempGrad.toInt();
          Serial.println("max Temp Grad Updated to "+inputmaxTempGrad);
     preferences.begin("settings",false);
     preferences.putInt("maxTempGrad",maxTempGrad);
     preferences.end();
    }}
    if (request->hasParam(PARAM_idleRPM)) {
      inputidleRPM = request->getParam(PARAM_idleRPM)->value();
      if (inputidleRPM.length()>0){
          idleRPM=inputidleRPM.toInt();
          Serial.println("idle RPM Updated to "+inputidleRPM);
     preferences.begin("settings",false);
     preferences.putInt("idleRPM",idleRPM);
     preferences.end();
    }}
     if (request->hasParam(PARAM_rpmTolerance)) {
      inputrpmTolerance = request->getParam(PARAM_rpmTolerance)->value();
      if (inputrpmTolerance.length()>0){
          rpmTolerance=inputrpmTolerance.toInt();
          Serial.println("RPM Tolerance Updated to "+inputrpmTolerance);
     preferences.begin("settings",false);
     preferences.putInt("rpmTolerance",rpmTolerance);
     preferences.end();
    }}
     if (request->hasParam(PARAM_glowOnRPM)) {
      inputglowOnRPM = request->getParam(PARAM_glowOnRPM)->value();
      if (inputglowOnRPM.length()>0){
          glowOnRPM=inputglowOnRPM.toInt();
          Serial.println("glow On RPM Updated to "+inputglowOnRPM);
     preferences.begin("settings",false);
     preferences.putInt("glowOnRPM",glowOnRPM);
     preferences.end();
    }}
     if (request->hasParam(PARAM_glowOffRPM)) {
      inputglowOffRPM = request->getParam(PARAM_glowOffRPM)->value();
      if (inputglowOffRPM.length()>0){
          glowOffRPM=inputglowOffRPM.toInt();
          Serial.println("glow Off RPM Updated to "+inputglowOffRPM);
     preferences.begin("settings",false);
     preferences.putInt("glowOffRPM",glowOffRPM);
     preferences.end();
    }}
    if (request->hasParam(PARAM_ignitionRPMHigh)) {
      inputignitionRPMHigh = request->getParam(PARAM_ignitionRPMHigh)->value();
      if (inputignitionRPMHigh.length()>0){
          ignitionRPMHigh=inputignitionRPMHigh.toInt();
          Serial.println("ignition RPM High Updated to "+inputignitionRPMHigh);
     preferences.begin("settings",false);
     preferences.putInt("ignitionRPMHigh",ignitionRPMHigh);
     preferences.end();
    }}
    if (request->hasParam(PARAM_ignitionRPMLow)) {
      inputignitionRPMLow = request->getParam(PARAM_ignitionRPMLow)->value();
      if (inputignitionRPMLow.length()>0){
          ignitionRPMLow=inputignitionRPMLow.toInt();
          Serial.println("ignition RPM Low Updated to "+inputignitionRPMLow);
     preferences.begin("settings",false);
     preferences.putInt("ignitionRPMLow",ignitionRPMLow);
     preferences.end();
    }}
     if (request->hasParam(PARAM_gasOnRPM)) {
      inputgasOnRPM = request->getParam(PARAM_gasOnRPM)->value();
      if (inputgasOnRPM.length()>0){
          gasOnRPM=inputgasOnRPM.toInt();
          Serial.println("Gas On RPM Updated to "+inputgasOnRPM);
     preferences.begin("settings",false);
     preferences.putInt("gasOnRPM",gasOnRPM);
     preferences.end();
    }}
    if (request->hasParam(PARAM_gasOffRPM)) {
      inputgasOffRPM = request->getParam(PARAM_gasOffRPM)->value();
      if (inputgasOffRPM.length()>0){
          gasOffRPM=inputgasOffRPM.toInt();
          Serial.println("Gas Off RPM Updated to "+inputgasOffRPM);
     preferences.begin("settings",false);
     preferences.putInt("gasOffRPM",gasOffRPM);
     preferences.end();
    }}
    if (request->hasParam(PARAM_starterOffRPM)) {
      inputstarterOffRPM = request->getParam(PARAM_starterOffRPM)->value();
      if (inputstarterOffRPM.length()>0){
          starterOffRPM=inputstarterOffRPM.toInt();
          Serial.println("Starter Off RPM Updated to "+inputstarterOffRPM);
     preferences.begin("settings",false);
     preferences.putInt("starterOffRPM",starterOffRPM);
     preferences.end();
    }}
if (request->hasParam(PARAM_fuelOnRPM)) {
      inputfuelOnRPM = request->getParam(PARAM_fuelOnRPM)->value();
      if (inputfuelOnRPM.length()>0){
          fuelOnRPM=inputfuelOnRPM.toInt();
          Serial.println("fuel On RPM Updated to "+inputfuelOnRPM);
     preferences.begin("settings",false);
     preferences.putInt("fuelOnRPM",fuelOnRPM);
     preferences.end();
    }}
    if (request->hasParam(PARAM_pumpOnValue)) {
      inputpumpOnValue = request->getParam(PARAM_pumpOnValue)->value();
      if (inputpumpOnValue.length()>0){
          pumpOnValue=inputpumpOnValue.toInt();
          Serial.println("Pump On Value Updated to "+inputpumpOnValue);
     preferences.begin("settings",false);
     preferences.putInt("pumpOnValue",pumpOnValue);
     preferences.end();
    }}
 if (request->hasParam(PARAM_purgeTime)) {
      inputpurgeTime = request->getParam(PARAM_purgeTime)->value();
      if (inputpurgeTime.length()>0){
          purgeTime=inputpurgeTime.toInt();
          Serial.println("purge time Updated to "+inputpurgeTime);
     preferences.begin("settings",false);
     preferences.putInt("purgeTime",purgeTime);
     preferences.end();
    }}
if (request->hasParam(PARAM_purgeThrottle)) {
      inputpurgeThrottle = request->getParam(PARAM_purgeThrottle)->value();
      if (inputpurgeThrottle.length()>0){
          purgeThrottle=inputpurgeThrottle.toInt();
          Serial.println("purge throttle Updated to "+inputpurgeThrottle);
     preferences.begin("settings",false);
     preferences.putInt("purgeThrottle",purgeThrottle);
     preferences.end();
    }}
 if (request->hasParam(PARAM_starterIncDelay)) {
      inputstarterIncDelay = request->getParam(PARAM_starterIncDelay)->value();
      if (inputstarterIncDelay.length()>0){
          starterIncDelay=inputstarterIncDelay.toInt();
          Serial.println("starter Delay Updated to "+inputstarterIncDelay);
     preferences.begin("settings",false);
     preferences.putInt("starterIDelay",starterIncDelay);
     preferences.end();
    }}
    

 if (request->hasParam(PARAM_accelerationDelay)) {
      inputaccelerationDelay = request->getParam(PARAM_accelerationDelay)->value();
      if (inputaccelerationDelay.length()>0){
          accelerationDelay=inputaccelerationDelay.toInt();
          Serial.println("acceleration Delay Updated to "+inputaccelerationDelay);
     preferences.begin("settings",false);
     preferences.putInt("accelDelay",accelerationDelay);
     preferences.end();
    }}
if (request->hasParam(PARAM_decelerationDelay)) {
      inputdecelerationDelay = request->getParam(PARAM_decelerationDelay)->value();
      if (inputdecelerationDelay.length()>0){
          decelerationDelay=inputdecelerationDelay.toInt();
          Serial.println("deceleration Delay Updated to "+inputdecelerationDelay);
     preferences.begin("settings",false);
     preferences.putInt("decelDelay",decelerationDelay);
     preferences.end();
    }}
 if (request->hasParam(PARAM_lowAccelDelay)) {
      inputlowAccelDelay = request->getParam(PARAM_lowAccelDelay)->value();
      if (inputlowAccelDelay.length()>0){
          lowAccelDelay=inputlowAccelDelay.toInt();
          Serial.println("low RPM Acceleration Delay Updated to "+inputlowAccelDelay);
     preferences.begin("settings",false);
     preferences.putInt("lowADelay",lowAccelDelay);
     preferences.end();
    }}
    if (request->hasParam(PARAM_lowDecelDelay)) {
      inputlowDecelDelay = request->getParam(PARAM_lowDecelDelay)->value();
      if (inputlowDecelDelay.length()>0){
          lowDecelDelay=inputlowDecelDelay.toInt();
          Serial.println("low RPM Deceleration Delay Updated to "+inputlowDecelDelay);
     preferences.begin("settings",false);
     preferences.putInt("lowDDelay",lowDecelDelay);
     preferences.end();
    }}
 if (request->hasParam(PARAM_ignitionThreshold)) {
      inputignitionThreshold = request->getParam(PARAM_ignitionThreshold)->value();
      if (inputignitionThreshold.length()>0){
          ignitionThreshold=inputignitionThreshold.toInt();
          Serial.println("ignition Threshold Updated to "+inputignitionThreshold);
     preferences.begin("settings",false);
     preferences.putInt("ignThreshold",ignitionThreshold);
     preferences.end();
    }}
     if (request->hasParam(PARAM_noIgnitionThreshold)) {
      inputnoIgnitionThreshold = request->getParam(PARAM_noIgnitionThreshold)->value();
      if (inputnoIgnitionThreshold.length()>0){
          noIgnitionThreshold=inputnoIgnitionThreshold.toInt();
          Serial.println("no ignition Threshold Updated to "+inputnoIgnitionThreshold);
     preferences.begin("settings",false);
     preferences.putInt("noIgnThreshold",noIgnitionThreshold);
     preferences.end();
    }}
      if (request->hasParam(PARAM_fuelFlowDetectionTime)) {
      inputfuelFlowDetectionTime = request->getParam(PARAM_fuelFlowDetectionTime)->value();
      if (inputfuelFlowDetectionTime.length()>0){
          fuelFlowDetectionTime=inputfuelFlowDetectionTime.toInt();
          Serial.println("Fuel Flow Detection Time Updated to "+fuelFlowDetectionTime);
     preferences.begin("settings",false);
     preferences.putInt("fuelDetTime",fuelFlowDetectionTime);
     preferences.end();
    }}
     if (request->hasParam(PARAM_fuelFlowDetectionRPM)) {
      inputfuelFlowDetectionRPM = request->getParam(PARAM_fuelFlowDetectionRPM)->value();
      if (inputfuelFlowDetectionRPM.length()>0){
          fuelFlowDetectionRPM=inputfuelFlowDetectionRPM.toInt();
          Serial.println("Fuel Flow Detection RPM Updated to "+fuelFlowDetectionRPM);
     preferences.begin("settings",false);
     preferences.putInt("fuelDetRPM",fuelFlowDetectionRPM);
     preferences.end();
    }}
      if (request->hasParam(PARAM_startingTemp)) {
      inputstartingTemp = request->getParam(PARAM_startingTemp)->value();
      if (inputstartingTemp.length()>0){
          startingTemp=inputstartingTemp.toInt();
          Serial.println("starting Temperature Updated to "+startingTemp);
     preferences.begin("settings",false);
     preferences.putInt("startingTemp",startingTemp);
     preferences.end();
    }}
     if (request->hasParam(PARAM_maxMotorVolt)) {
      inputmaxMotorVolt = request->getParam(PARAM_maxMotorVolt)->value();
      if (inputmaxMotorVolt.length()>0){
          maxMotorVolt=inputmaxMotorVolt.toFloat();
          Serial.println("Starter Motor Voltage Updated to "+String(maxMotorVolt));
     preferences.begin("settings",false);
     preferences.putFloat("maxMotorVolt",maxMotorVolt);
     preferences.end();
    }}
    if (request->hasParam(PARAM_maxPumpVolt)) {
      inputmaxPumpVolt = request->getParam(PARAM_maxPumpVolt)->value();
      if (inputmaxPumpVolt.length()>0){
          maxPumpVolt=inputmaxPumpVolt.toFloat();
          Serial.println("Fuel Pump Voltage Updated to "+String(maxPumpVolt));
     preferences.begin("settings",false);
     preferences.putFloat("maxPumpVolt",maxPumpVolt);
     preferences.end();
    }}
 if (request->hasParam(PARAM_MIN_MICROS)) {
      inputMIN_MICROS = request->getParam(PARAM_MIN_MICROS)->value();
      if (inputMIN_MICROS.length()>0){
          MIN_MICROS=inputMIN_MICROS.toInt();
          Serial.println("MIN_MICROS Updated to "+inputMIN_MICROS);
     preferences.begin("settings",false);
     preferences.putInt("MIN_MICROS",MIN_MICROS);
     preferences.end();
    }}

    if (request->hasParam(PARAM_MAX_MICROS)) {
      inputMAX_MICROS = request->getParam(PARAM_MAX_MICROS)->value();
      if (inputMAX_MICROS.length()>0){
          MAX_MICROS=inputMAX_MICROS.toInt();
          Serial.println("MAX_MICROS Updated to "+inputMAX_MICROS);
     preferences.begin("settings",false);
     preferences.putInt("MAX_MICROS",MAX_MICROS);
     preferences.end();
    }}
     if (request->hasParam(PARAM_voltIn)) {
      inputvoltIn = request->getParam(PARAM_voltIn)->value();
      if (inputvoltIn.length()>0){
        float volt=resistorRatio*3.3*(float(voltsensorValue)/4095);
       voltCorrection=inputvoltIn.toFloat()/volt;
          Serial.println("Input voltage calibration factor "+String(voltCorrection));
     preferences.begin("settings",false);
     preferences.putFloat("voltCorr",voltCorrection);
     preferences.end();
    }}

     if (request->hasParam(PARAM_WiFiPWD)) {
      inputWiFiPWD = request->getParam(PARAM_WiFiPWD)->value();
      if (inputWiFiPWD.length()>7){ //WiFi Password has to be more than 7 characters
          password=inputWiFiPWD;
          Serial.println("WiFi Password Updated to "+password);
     preferences.begin("settings",false);
     preferences.putString("PWD",password);
     preferences.end();
    }}

    if (request->hasParam(PARAM_WiFiSSID)) {
      inputWiFiSSID = request->getParam(PARAM_WiFiSSID)->value();
      if (inputWiFiSSID.length()>0){
          ssid=inputWiFiSSID;
          Serial.println("WiFi SSID Updated to "+ssid);
     preferences.begin("settings",false);
     preferences.putString("SSID",ssid);
     preferences.end();
    }}
     if (request->hasParam(PARAM_keroPower)) {
      inputkeroPower = request->getParam(PARAM_keroPower)->value();
      if (inputkeroPower.length()>0){
          keroPower=inputkeroPower.toInt();
          Serial.println("keroPower Updated to "+keroPower);
     preferences.begin("settings",false);
     preferences.putInt("keroPower",keroPower);
     preferences.end();
    }}
     if (request->hasParam(PARAM_keroTime)) {
      inputkeroTime = request->getParam(PARAM_keroTime)->value();
      if (inputkeroTime.length()>0){
          keroTime=inputkeroTime.toInt();
          Serial.println("kero Preheat time Updated to "+keroTime);
     preferences.begin("settings",false);
     preferences.putInt("keroTime",keroTime);
     preferences.end();
    }}
    if (request->hasParam(PARAM_keroValve)) {
      inputkeroValve = request->getParam(PARAM_keroValve)->value();
      if (inputkeroValve.length()>0){
          keroValve=inputkeroValve.toInt();
          Serial.println("kero valve Updated to "+keroValve);
     preferences.begin("settings",false);
     preferences.putInt("keroValve",keroValve);
     preferences.end();
    }}
    if (request->hasParam(PARAM_keroFreq)) {
      inputkeroFreq = request->getParam(PARAM_keroFreq)->value();
      if (inputkeroFreq.length()>0){
          keroFreq=inputkeroFreq.toInt();
          Serial.println("kero valve PWM Frequency Updated to "+keroFreq);
     preferences.begin("settings",false);
     preferences.putInt("keroFreq",keroFreq);
     preferences.end();
    }}
     if (request->hasParam(PARAM_keroPump)) {
      inputkeroPump = request->getParam(PARAM_keroPump)->value();
      if (inputkeroPump.length()>0){
          keroPump=inputkeroPump.toInt();
          Serial.println("kero start pump rate Updated to "+keroPump);
     preferences.begin("settings",false);
     preferences.putInt("keroPump",keroPump);
     preferences.end();
    }}
      if (request->hasParam(PARAM_idlePress)) {
      inputidlePress = request->getParam(PARAM_idlePress)->value();
      if (inputidlePress.length()>0){
          idlePress=inputidlePress.toFloat();
          Serial.println("idlePressure Updated to "+String(idlePress));
     preferences.begin("settings",false);
     preferences.putFloat("idlePress",idlePress);
     preferences.end();
    }}
      if (request->hasParam(PARAM_maxPress)) {
      inputmaxPress = request->getParam(PARAM_maxPress)->value();
      if (inputmaxPress.length()>0){
          maxPress=inputmaxPress.toFloat();
          Serial.println("max Pressure Updated to "+String(maxPress));
     preferences.begin("settings",false);
     preferences.putFloat("maxPress",maxPress);
     preferences.end();
    }}
    if (request->hasParam(PARAM_pressTolerance)) {
      inputpressTolerance = request->getParam(PARAM_pressTolerance)->value();
      if (inputpressTolerance.length()>0){
          pressTolerance=inputpressTolerance.toFloat();
          Serial.println("Pressure Tolerance Updated to "+String(maxPress));
     preferences.begin("settings",false);
     preferences.putFloat("pressTol",pressTolerance);
     preferences.end();
    }}
      if (request->hasParam(PARAM_ignitionThrottle)) {
      inputignitionThrottle = request->getParam(PARAM_ignitionThrottle)->value();
      if (inputignitionThrottle.length()>0){
          ignitionThrottle=inputignitionThrottle.toInt();
          Serial.println("ignition Throttle Updated to "+String(ignitionThrottle));
     preferences.begin("settings",false);
     preferences.putInt("ignThrottle",ignitionThrottle);
     preferences.end();
    }}
     if (request->hasParam(PARAM_throttleHighTime)) {
      inputthrottleHighTime = request->getParam(PARAM_throttleHighTime)->value();
      if (inputthrottleHighTime.length()>0){
          throttleHighTime=inputthrottleHighTime.toInt();
          Serial.println("High Throttle Delay Updated to "+String(throttleHighTime));
     preferences.begin("settings",false);
     preferences.putInt("thrDelay",throttleHighTime);
     preferences.end();
    }}
        if (request->hasParam(PARAM_throttleLowTime)) {
      inputthrottleLowTime = request->getParam(PARAM_throttleLowTime)->value();
      if (inputthrottleLowTime.length()>0){
          throttleLowTime=inputthrottleLowTime.toInt();
          Serial.println("Low Throttle Delay Updated to "+String(throttleLowTime));
     preferences.begin("settings",false);
     preferences.putInt("thrDelay",throttleLowTime);
     preferences.end();
    }}
     
     if (request->hasParam(PARAM_idleFuel)) {
      inputidleFuel = request->getParam(PARAM_idleFuel)->value();
      if (inputidleFuel.length()>0){
         idleFuel=inputidleFuel.toInt();
          Serial.println("Idle Fuel Flow Updated to "+String(idleFuel));
     preferences.begin("settings",false);
     preferences.putInt("idleFuel",idleFuel);
     preferences.end();
    }}
      if (request->hasParam(PARAM_maxFuel)) {
      inputmaxFuel = request->getParam(PARAM_maxFuel)->value();
      if (inputmaxFuel.length()>0){
         maxFuel=inputmaxFuel.toInt();
          Serial.println("max Fuel Flow Updated to "+String(maxFuel));
     preferences.begin("settings",false);
     preferences.putInt("maxFuel",maxFuel);
     preferences.end();
    }}
    if (request->hasParam(PARAM_fuelTolerance)) {
      inputfuelTolerance = request->getParam(PARAM_fuelTolerance)->value();
      if (inputfuelTolerance.length()>0){
         fuelTolerance=inputfuelTolerance.toInt();
          Serial.println("fuel tolerance Updated to "+String(maxFuel));
     preferences.begin("settings",false);
     preferences.putInt("fuelTol",fuelTolerance);
     preferences.end();
    }}
     if (request->hasParam(PARAM_calWeight)) {
      inputcalWeight = request->getParam(PARAM_calWeight)->value();
      if (inputcalWeight.length()>0){
         float calWeight=inputcalWeight.toFloat();
          
           scale.set_scale();
            long reading = scale.get_units(10);
           scaleconst=reading/calWeight;
       scale.set_scale(scaleconst);
       Serial.println(scaleconst);
       scalingdone=true;
    //Save values to EEPROM
    preferences.begin("settings",false);
   preferences.putBool("ScalingDone",scalingdone);
    preferences.putFloat("ScaleConst",scaleconst);
    preferences.end();
    }}
    if (request->hasParam(PARAM_calPress0)) {
    inputcalPress0 = request->getParam(PARAM_calPress0)->value();
    if (inputcalPress0.length()>0){
        calPress0=inputcalPress0.toFloat();
          //Save values to EEPROM
    preferences.begin("settings",false);
   preferences.putFloat("calPress0",calPress0);
    preferences.end();
     Serial.println("Cal Press 0 Updated to "+String(calPress0));
    }}
    if (request->hasParam(PARAM_calPress1)) {
    inputcalPress1 = request->getParam(PARAM_calPress1)->value();
    if (inputcalPress1.length()>0){
      calPress1=inputcalPress1.toFloat();
          //Save values to EEPROM
    preferences.begin("settings",false);
    preferences.putFloat("calPress1",calPress1);
    preferences.end();
    Serial.println("Cal Press 1 Updated to "+String(calPress1));
    }}
    if (request->hasParam(PARAM_calVolt0)) {
    inputcalVolt0 = request->getParam(PARAM_calVolt0)->value();
    if (inputcalVolt0.length()>0){
        calVolt0=inputcalVolt0.toFloat();
          //Save values to EEPROM
    preferences.begin("settings",false);
    preferences.putFloat("calVolt0",calVolt0);
    preferences.end();
    Serial.println("Cal Volt 0 Updated to "+String(calVolt0));
    }}
     if (request->hasParam(PARAM_calVolt1)) {
    inputcalVolt1 = request->getParam(PARAM_calVolt1)->value();
    if (inputcalVolt1.length()>1){
       calVolt1=inputcalVolt1.toFloat();
          //Save values to EEPROM
    preferences.begin("settings",false);
    preferences.putFloat("calVolt1",calVolt1);
    preferences.end();
    Serial.println("Cal Volt 1 Updated to "+String(calVolt1));
    }}
    if (request->hasParam(PARAM_preheatDelay)) {
    inputpreheatDelay = request->getParam(PARAM_preheatDelay)->value();
    if (inputpreheatDelay.length()>1){
       preheatDelay=inputpreheatDelay.toInt();
          //Save values to EEPROM
    preferences.begin("settings",false);
    preferences.putInt("phDelay",preheatDelay);
    preferences.end();
    Serial.println("preheatDelay Updated to "+String(preheatDelay));
    }}
    request->send(200, "text/html", "Parameters updated as per input fields<br><a href=\"/\">Return to Home Page</a>");
                                    
  });
dnsServer.start(53, "*", WiFi.softAPIP());
 events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  
  server.addHandler(&events);
   server.begin();  

   }
   // Replaces placeholder with button section in your web page
String page1processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<h4>WiFi WebServer - State <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}
String outputState(){
  if(serverStarted){
    return "checked";
  }
  else {
       return "";
  }
   return "";
 }  
