 const char htmlPage2[] PROGMEM = R"=====(
<html>

<head>
    <meta http-equiv="cache-control" content="max-age=0" />
    <meta http-equiv="cache-control" content="no-cache" />
    <meta http-equiv="expires" content="0" />
    <meta http-equiv="expires" content="Tue, 01 Jan 1980 1:00:00 GMT" />
    <meta http-equiv="pragma" content="no-cache" />
    <meta http-equiv="content-type" content="text/html;charset=UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
    <title>Ardu_ECU - Page1</title>

    <style>

font-family: Arial, Helvetica, sans-serif; 
  display: inline-block; 
  text-align: center;

h1 {
 font-size: 1.8rem; 
  color: white;
  margin: 5%;
}
p { 
 
    padding-left: 5%;
}
form {
  margin-left: 5%;
}
span{
 margin-left: 5%;
}
.topnav { 
  overflow: hidden; 
  background-color: #0A1128;
}
 .switch {position: relative; display: inline-block;margin-left: 15%; width: 60px; height: 34px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 17px}
    .slider:before {position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 34px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px)}
  
</style>
   
</head>

<body>
    <div class="container">
        <h1>Ardu_ECU</h1>
        <p>Arduino ECU Engine Parameters Page</p>
            <style>
      .button {
        background-color: #1c87c9;
        border: none;
        color: white;
        padding: 5px 9px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 16px;
        margin: 4px 2px;
        cursor: pointer;
        border-radius: 8px;
      }
    </style>
        <p>
           <a  href="/" class="button">Page1</a>
            <a href="/page2" class="button" >Page2</a>
            <a  href="/page3" class="button">Page3</a>
            <a href="/page4" class="button">Page4</a>
            <a  href="/page5" class="button">Page5</a>
            <a  href="/page6" class="button">Page6</a>
            <a  href="/page7" class="button">Page7</a>
              <a  href="/page8" class="button">Page8</a>
            
        </p>
    </div>
<hr/>
 
    <p>Purge Time(millisec): <span id='idpurgeTime'></span></p>
  <form action="/get">
   <input type="number" name="purgeTime"min="0" >
    <input type="submit" value="Submit">
  </form><br>
   <p>Purge Throttle (0-1000): <span id='idpurgeThrottle'></span></p>
   <form action="/get">
   <input type="number" name="purgeThrottle" min="0" max="500">
    <input type="submit" value="Submit">
  </form><br>
   <p>Starter Delay(millisec): <span id='idstarterIncDelay'></span></p>
  <form action="/get">
   <input type="number" name="starterIncDelay" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Starter Limit(0-1000): <span id='idstarterLimit'></span></p>
   <p>Use with unreliable RPM Measurement only</p>
  <form action="/get">
   <input type="number" name="starterLimit" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Acceleration Delay (millisec): <span id='idaccelerationDelay'></span></p>
   <form action="/get">
   <input type="number" name="accelerationDelay" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Deceleration Delay (millisec): <span id='iddecelerationDelay'></span></p>
  <form action="/get">
 <input type="number" name="decelerationDelay" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Low RPM Acceleration Delay (millisec): <span id='idlowAccelDelay'></span></p>
   <form action="/get">
   <input type="number" name="lowAccelDelay" min="0">
    <input type="submit" value="Submit">
  </form><br>
  <p>Low RPM Deceleration Delay (millisec): <span id='idlowDecelDelay'></span></p>
   <form action="/get">
   <input type="number" name="lowDecelDelay" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Ignition Threshold Temp.(degC): <span id='idignitionThreshold'></span></p>
   <form action="/get">
   <input type="number" name="ignitionThreshold" min="0">
    <input type="submit" value="Submit">
  </form><br>
  <p>No Ignition Timeout(millisec): <span id='idnoIgnitionThreshold'></span></p>
   <form action="/get">
   <input type="number" name="noIgnitionThreshold" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <form  name="RPMSensor">
  <fieldset>
    <legend>RPM Sensor type:</legend>
    <div>
  <input type="radio" id="hall" name="cprtype" value=1 onChange="Change1()"/>
  <label for="hall">Magnetic Sensor - 1 Pulse per rev</label>
  </div>
  <input type="radio" id="optical" name="cprtype" value=2 onChange="Change2()"/>
  <label for="optical">Optical Sensor - 2 Pulses per rev</label>
  </div>
    </form>
  </fieldset>

<p> <h4>Trial Mode Settings (Use With Caution)</H4></p>
   <p>TrialModeFuelOnRPM: <span id='idtrialModeFuelOnRPM'></span></p>
   <form action="/get">
   <input type="number" name="trialModeFuelOnRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
  <p>TrialModeFuelFlow:(0-1000) <span id='idtrialModeFuelFlow'></span></p>
   <form action="/get">
   <input type="number" name="trialModeFuelFlow" min="0" max="500">
    <input type="submit" value="Submit">
  </form><br>  
 <hr/>   
      <p>Minimum Servo Signal(microsec): <span id='idMIN_MICROS'></span></p>
  <form action="/get">
   <input type="number" name="MIN_MICROS" min="500">
    <input type="submit" value="Submit">
  </form><br>
  <p>Maximum Servo Signal(microsec): <span id='idMAX_MICROS'></span></p>
  <form action="/get">
   <input type="number" name="MAX_MICROS" max="2500">
    <input type="submit" value="Submit">
  </form><br>
  <hr/>
    <p>  <h4>WiFi Settings</h4> </p>
  <p>Wifi SSID: <span id='idWiFiSSID'></span></p>
  <form action="/get">
   <input type="string" name="WiFiSSID" >
    <input type="submit" value="Submit">
  </form><br>
  <p>Wifi Password: <span id='idWiFiPWD'></span></p>
  <form action="/get">
   <input type="string" name="WiFiPWD" >
    <input type="submit" value="Submit">
  </form><br>


 
  <hr/>

   <script>
   
function Change1() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/hall", true);
  xhttp.send();
}
function Change2() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/optical", true);
  xhttp.send();
}
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
    requestData(); // get intial data straight away 
  
    // request data updates every 1000 milliseconds
    setInterval(requestData, 1000);

    function requestData() {

      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/parampage2");

      xhr.onload = function() {
        if (xhr.status === 200) {

          if (xhr.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(xhr.responseText);
          
            
            document.getElementById("idpurgeTime").innerText = data.purgeTime;
            document.getElementById("idpurgeThrottle").innerText = data.purgeThrottle;
            document.getElementById("idstarterIncDelay").innerText = data.starterIncDelay;
            document.getElementById("idstarterLimit").innerText = data.starterLimit;
            document.getElementById("idaccelerationDelay").innerText = data.accelerationDelay;
            document.getElementById("iddecelerationDelay").innerText = data.decelerationDelay;
            document.getElementById("idlowAccelDelay").innerText = data.lowAccelDelay;
            document.getElementById("idlowDecelDelay").innerText = data.lowDecelDelay;
            document.getElementById("idignitionThreshold").innerText = data.ignitionThreshold;
            document.getElementById("idnoIgnitionThreshold").innerText = data.noIgnitionThreshold;
            document.RPMSensor.cprtype.value=data.cpr;
            document.getElementById("idtrialModeFuelOnRPM").innerText = data.trialModeFuelOnRPM;
            document.getElementById("idtrialModeFuelFlow").innerText = data.trialModeFuelFlow;
             document.getElementById("idMIN_MICROS").innerText = data.MIN_MICROS;
            document.getElementById("idMAX_MICROS").innerText = data.MAX_MICROS;
            document.getElementById("idWiFiSSID").innerText = data.WiFiSSID;
            document.getElementById("idWiFiPWD").innerText = data.WiFiPWD;
            
            
          } else { // a problem occurred
            
          
            document.getElementById("idpurgeTime").innerText = "?";
            document.getElementById("idpurgeThrottle").innerText = "?";
            document.getElementById("idstarterIncDelay").innerText = "?";
            document.getElementById("idstarterLimit").innerText = "?";
            document.getElementById("idaccelerationDelay").innerText = "?";
            document.getElementById("iddecelerationDelay").innerText = "?";
            document.getElementById("idlowAccelDelay").innerText = "?";
            document.getElementById("idlowDecelDelay").innerText = "?";
            document.getElementById("idignitionThreshold").innerText = "?";
            document.getElementById("idnoIgnitionThreshold").innerText = "?";
            document.RPMSensor.cprtype.value="?";
            document.getElementById("idtrialModeFuelOnRPM").innerText = "?";
            document.getElementById("idtrialModeFuelFlow").innerText = "?";
              document.getElementById("idMIN_MICROS").innerText = "?";
            document.getElementById("idMAX_MICROS").innerText = "?";
            document.getElementById("idWiFiSSID").innerText = "?";
             document.getElementById("idWiFiPWD").innerText = "?";
           
    //         document.getElementById("output").checked = "?";
      //       document.getElementById("outputState").innerHTML = "?";

          }
        } else {
          console.log('Request failed.  Returned status of ' + xhr.status);
               
             
            document.getElementById("idpurgeTime").innerText = "?";
            document.getElementById("idpurgeThrottle").innerText = "?";
            document.getElementById("idstarterIncDelay").innerText = "?";
             document.getElementById("idstarterLimit").innerText = "?";
            document.getElementById("idaccelerationDelay").innerText = "?";
            document.getElementById("iddecelerationDelay").innerText = "?";
             document.getElementById("idlowAccelDelay").innerText = "?";
            document.getElementById("idlowDecelDelay").innerText = "?";
            document.getElementById("idignitionThreshold").innerText = "?";
            document.getElementById("idnoIgnitionThreshold").innerText = "?";
            document.RPMSensor.cprtype.value="?";
           document.getElementById("idtrialModeFuelOnRPM").innerText = "?";
            document.getElementById("idtrialModeFuelFlow").innerText = "?";
              document.getElementById("idMIN_MICROS").innerText = "?";
            document.getElementById("idMAX_MICROS").innerText = "?";
             document.getElementById("idWiFiSSID").innerText = "?";
            document.getElementById("idWiFiPWD").innerText = "?";
         
          
       //     document.getElementById("output").checked = "?";
         //    document.getElementById("outputState").innerHTML = "?";

        }
      };
      
      xhr.send();
    }
    
  </script>
</body>

</html>
)=====";
