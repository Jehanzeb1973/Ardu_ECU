 const char htmlPage3[] PROGMEM = R"=====(
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
    <title>Ardu_ECU - Page3</title>

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
      .buttongreen {background-color: #4CAF50;} /* Green */
        .switch {position: relative; display:inline-block;margin-left:15%; width:60px; height:34px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 17px}
    .slider:before {position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 34px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px)}
    </style>
        <p>
           <a  href="/" class="button" >Page1</a>
            <a href="/page2" class="button" >Page2</a>
            <a  href="/page3" class="button buttongreen">Page3</a>
            <a href="/page4" class="button">Page4</a>
            <a  href="/page5" class="button">Page5</a>
            <a  href="/page6" class="button">Page6</a>
            <a  href="/page7" class="button">Page7</a>
             <a  href="/page8" class="button">Page8</a>
             <a href="/page9" class="button" >Page9</a>
            
        </p>
    </div>
<hr/>
  <form  name="StartControlMethod">
   <fieldset>
    <legend>Start Control Method:</legend>
    <div>
  <input type="radio" id="startRPM" name="startcontroltype" value=1 onChange="StartChange_rpm()"/>
  <label for="RPM">RPM Control</label>
  </div>
  <input type="radio" id="startPressure" name="startcontroltype" value=2 onChange="StartChange_pressure()"/>
  <label for="Pressure">Pressure Control</label>
  </div>
  <input type="radio" id="startThrottle" name="startcontroltype" value=3 onChange="StartChange_throttle()"/>
  <label for="Throttle">Fuel Control</label>
  </div>
    </form><br>
    </fieldset>
    <form  name="RunControlMethod">
   <fieldset>
    <legend>Run Control Method:</legend>
    <div>
  <input type="radio" id="runRPM" name="runcontroltype" value=1 onChange="RunChange_rpm()"/>
  <label for="RPM">RPM Control</label>
  </div>
  <input type="radio" id="runPressure" name="runcontroltype" value=2 onChange="RunChange_pressure()"/>
  <label for="Pressure">Pressure Control</label>
  </div>
  <input type="radio" id="runThrottle" name="runcontroltype" value=3 onChange="RunChange_throttle()"/>
  <label for="Throttle">Fuel Control</label>
  </div>
    </form><br>
    </fieldset>
    

   <form  name="StartingMethod">
   <fieldset>
    <legend>Starting Fuel Selection:</legend>
    <div>
  <input type="radio" id="Gas" name="starttype" value=1 onChange="Change_gas()"/>
  <label for="Gas">Gas Start</label>
  </div>
  <input type="radio" id="Kero" name="starttype" value=2 onChange="Change_kero()"/>
  <label for="Kero">Kero Start</label>
  </div>
    </form>
     <p>Kero Plug Startup Power (0-1000): <span id='idkeroPower'></span></p>
     <form action="/get">
   <input type="number" name="keroPower" min="0" max="1000">
    <input type="submit" value="Submit">
  </form><br>
  <p>Kero Plug Heating Time(Seconds) (0-60): <span id='idkeroTime'></span></p>
   <form action="/get">
   <input type="number" name="keroTime" min="0" max="60">
    <input type="submit" value="Submit">
  </form><br>
   <p>Kero Plug Fuel Flow (0-1000): <span id='idkeroValve'></span></p>
   <form action="/get">
   <input type="number" name="keroValve" min="0" max="1000">
    <input type="submit" value="Submit">
  </form><br>
  <p>Kero Plug Fuel Valve PWM Frequency (0-60): <span id='idkeroFreq'></span></p>
   <form action="/get">
   <input type="number" name="keroFreq" min="0" max="60">
    <input type="submit" value="Submit">
  </form><br>
  <p>Kero Start Fuel Pump Setting (0-1000): <span id='idkeroPump'></span></p>
   <form action="/get">
   <input type="number" name="keroPump" min="0" max="1000">
    <input type="submit" value="Submit">
  </form><br>
  </fieldset>
   <form  name="RPMSensor">
  <fieldset>
    <legend>RPM Sensor type:</legend>
    <div>
  <input type="radio" id="hall" name="cprtype" value=1 onChange="Change_hall()"/>
  <label for="hall">Magnetic Sensor - 1 Pulse per rev</label>
  </div>
  <input type="radio" id="optical" name="cprtype" value=2 onChange="Change_optical()"/>
  <label for="optical">Optical Sensor - 2 Pulses per rev</label>
  </div>
    </form>
  </fieldset>
    <br>
     <form  name="Serial Data ">
  <fieldset>
    <legend>Serial Data Transmission:</legend>
    <div>
    <input type="checkbox" name="serialData" id="checkSerial" onclick="toggleSerialCheckbox()" value="0">Enable Serial Data</><br>
      
</fieldset>
 </div>
    </form>
      <form  name="Load Cell ">
 <fieldset>
    <legend>Load Cell Calibration</legend>
    <div>
   <input type="checkbox" name="loadCell" id="checkLoadCell" onclick="toggleCheckbox()" value="0">Enable Load Cell</><br>
     <label>1-Tare with no load to set zero scale</label><br>
     <label>2-Calibrate with a known load </label><br>
     <a href="/tare" class="button">Tare(No Load)</a><br>
     <p>Calibrate Load: <span id='idload'></span></p>
      <form action="/get">
  <input type="number" name="calWeight"> 
    <input type="submit" value="Submit">
  </form><br>
</fieldset>
</div>
    </form>

   <script>
 function StartChange_rpm() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/startrpmcontrol", true);
  xhttp.send();
}
 function StartChange_pressure() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/startpressurecontrol", true);
  xhttp.send();
}
 function StartChange_throttle() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/startthrottlecontrol", true);
  xhttp.send();
}
 function RunChange_rpm() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/runrpmcontrol", true);
  xhttp.send();
}
 function RunChange_pressure() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/runpressurecontrol", true);
  xhttp.send();
}
 function RunChange_throttle() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/runthrottlecontrol", true);
  xhttp.send();
}
 function Change_gas() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/gasstart", true);
  xhttp.send();
}
function Change_kero() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/kerostart", true);
  xhttp.send();
}
  function Change_hall() {
 var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/hall", true);
  xhttp.send();
}
function Change_optical() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/optical", true);
  xhttp.send();
}  
function toggleSerialCheckbox() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/toggleSerialEnable");
  xhr.send();
}
function toggleCheckbox() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/toggleloadCellEnable");
  xhr.send();
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
      xhr.open("GET", "/parampage3");

      xhr.onload = function() {
        if (xhr.status === 200) {

          if (xhr.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(xhr.responseText);
            document.StartControlMethod.startcontroltype.value=data.startcontroltype;
             document.RunControlMethod.runcontroltype.value=data.runcontroltype;
            document.RPMSensor.cprtype.value=data.cpr;
            document.StartingMethod.starttype.value=data.starttype;
            document.getElementById("idkeroPower").innerText = data.keroPower;
            document.getElementById("idkeroTime").innerText = data.keroTime;
            document.getElementById("idkeroValve").innerText = data.keroValve;
            document.getElementById("idkeroFreq").innerText = data.keroFreq;
            document.getElementById("idkeroPump").innerText = data.keroPump;
            document.getElementById("idload").innerText = data.loadcellvalue;
            document.getElementById("checkSerial").checked=data.enableSerial;
            document.getElementById("checkLoadCell").checked=data.enableLoadCell;
            
            
          } else { // a problem occurred
            
          
           document.ControlMethod.controltype.value="?";
           document.RPMSensor.cprtype.value="?";
           document.StartingMethod.starttype.value="?";
            document.getElementById("idkeroPower").innerText = "?";
            document.getElementById("idkeroTime").innerText = "?";
            document.getElementById("idkeroValve").innerText = "?";
            document.getElementById("idkeroFreq").innerText = "?";
            document.getElementById("idkeroPump").innerText = "?";
            document.getElementById("idload").innerText = "?";
             document.getElementById("checkSerial").checked="?";
            document.getElementById("checkLoadCell").checked="?";
           
    //         document.getElementById("output").checked = "?";
      //       document.getElementById("outputState").innerHTML = "?";

          }
        } else {
          console.log('Request failed.  Returned status of ' + xhr.status);
            
            document.ControlMethod.controltype.value="?";
           document.RPMSensor.cprtype.value="?";
           document.StartingMethod.starttype.value="?";
            document.getElementById("idkeroPower").innerText = "?";
            document.getElementById("idkeroTime").innerText = "?";
            document.getElementById("idkeroValve").innerText = "?";
            document.getElementById("idkeroFreq").innerText = "?";
            document.getElementById("idkeroPump").innerText = "?";
             document.getElementById("idload").innerText = "?";
              document.getElementById("checkSerial").checked="?";
             document.getElementById("checkLoadCell").checked="?";
         
          
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
