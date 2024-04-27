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
    <title>Ardu_ECU - Page2</title>

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
      .switch {position: relative; display: inline-block;margin-left: 15%; width: 60px; height: 34px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 17px}
    .slider:before {position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 34px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px)}
  
    </style>
        <p>
           <a  href="/" class="button">Page1</a>
            <a href="/page2" class="button buttongreen" >Page2</a>
            <a  href="/page3" class="button">Page3</a>
            <a href="/page4" class="button">Page4</a>
            <a  href="/page5" class="button">Page5</a>
            <a  href="/page6" class="button">Page6</a>
            <a  href="/page7" class="button">Page7</a>
            <a  href="/page8" class="button">Page8</a>
            <a  href="/page9" class="button">Page9</a>
            
        </p>
    </div>
<hr/>
   <fieldset>
    <legend><b>Settings for RPM Control Mode</b></legend>
    <div>
  <p>Max RPM: <span id='idmaxRPM'></span></p>
  <form action="/get">
    <input type="number" name="maxRPM" min="0"  > 
    <input type="submit" value="Submit">
  </form><br>
   <p>Idle RPM: <span id='ididleRPM'></span></p>
   <form action="/get">
    <input type="number" name="idleRPM" min="0"> 
    <input type="submit" value="Submit">
  </form><br>  
  <p>RPM Tolerance: <span id='idrpmTolerance'></span></p>
   <p>(for RPM>idleRPM)</p>
   <form action="/get">
    <input type="number" name="rpmTolerance" min="0"> 
    <input type="submit" value="Submit">
  </form><br>  
   <p>Glow On RPM: <span id='idglowOnRPM'></span></p>
    <form action="/get">
    <input type="number" name="glowOnRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Glow Off RPM: <span id='idglowOffRPM'></span></p>
  <form action="/get">
    <input type="number" name="glowOffRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Ignition RPM High: <span id='idignitionRPMHigh'></span></p>
  <form action="/get">
  <input type="number" name="ignitionRPMHigh" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Ignition RPM Low: <span id='idignitionRPMLow'></span></p>
    <form action="/get">
 <input type="number" name="ignitionRPMLow" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Gas On RPM: <span id='idgasOnRPM'></span></p>
   <form action="/get">
 <input type="number" name="gasOnRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Gas Off RPM: <span id='idgasOffRPM'></span></p>
   <form action="/get">
   <input type="number" name="gasOffRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Starter Off RPM: <span id='idstarterOffRPM'></span></p>
   <form action="/get">
   <input type="number" name="starterOffRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Fuel On RPM: <span id='idfuelOnRPM'></span></p>
   <form action="/get">
   <input type="number" name="fuelOnRPM" min="0">
    <input type="submit" value="Submit">
  </form><br>
  </fieldset>
  <fieldset>
    <legend><b>Settings for Pressure Control Mode</b></legend>
    <div>
    <p>Idle Pressure(kPa): <span id='ididlePress'></span></p>
  <form action="/get">
   <input type="number" name="idlePress"min="0" step=0.1>
    <input type="submit" value="Submit">
  </form><br>
  <p>max Pressure(kPa): <span id='idmaxPress'></span></p>
  <form action="/get">
   <input type="number" name="maxPress"min="0" step=0.1 >
    <input type="submit" value="Submit">
  </form><br>
  <p>Pressure Tolerance(kPa): <span id='idpressTolerance'></span></p>
  <form action="/get">
   <input type="number" name="pressTolerance"min="0" step=0.1 >
    <input type="submit" value="Submit">
  </form><br>
   <div>
   <form  name="CalibrationType">
  <input type="radio" id="useCustomCal" name="pressCalType" value=1 onChange="setCustomCal()"/>
  <label for="useCustomCal">Use Custom Calibration</label>
  <div>
  <input type="radio" id="useDefultCal" name="pressCalType" value=2 onChange="setDefaultCal()"/>
  <label for="useDefultCal">Use Default(0-200kPa) Calibration</label>
  <div>
  </form>
    <p>  <h4>Calibrate Pressure Sensor</h4> </p>
    <p>Pressure: <span id='idcasePress'></span></p>
     <a href="/zeroPressure" class="button">Set Pressure Zero</a>
  <br>
     <p>Calibrate Volt Low: <span id='idcalVolt0'></span></p>
     <form action="/get">
     <input type="number" name="calVolt0"step=0.01>
     <input type="submit" value="Submit">
     </form><br>
     <p>Calibrate Pressure Low: <span id='idcalPress0'></span></p>
     <form action="/get">
     <input type="number" name="calPress0"step=0.01>
     <input type="submit" value="Submit">
     </form><br>        
     <p>Calibrate Volt High: <span id='idcalVolt1'></span></p>
     <form action="/get">
     <input type="number" name="calVolt1"step=0.01>
     <input type="submit" value="Submit">
     </form><br>
     <p>Calibrate Pressure High: <span id='idcalPress1'></span></p>
     <form action="/get">
     <input type="number" name="calPress1"step=0.01>
     <input type="submit" value="Submit">
     </form><br>
       </fieldset>
  <br>
   <p>Ignition Throttle High (0-1000): <span id='idignThrottle'></span></p>
    <form action="/get">
   <input type="number" name="ignitionThrottle" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Throttle High Time(milliseconds): <span id='idthrDelay'></span></p>
     <form action="/get">
   <input type="number" name="throttleHighTime" min="0">
    <input type="submit" value="Submit">
  </form><br>
  <p>Throttle Low Time(milliseconds): <span id='idthrDelay2'></span></p>
     <form action="/get">
   <input type="number" name="throttleLowTime" min="0">
    <input type="submit" value="Submit">
  </form><br>
     <fieldset>
    <legend><b>Settings for Fuel Control Mode</b></legend>
    <div>
    <p>Idle Fuel Flow(0-1000): <span id='ididleFuel'></span></p>
  <form action="/get">
   <input type="number" name="idleFuel" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Maximum Fuel Flow(0-1000): <span id='idmaxFuel'></span></p>
     <form action="/get">
   <input type="number" name="maxFuel" min="0">
    <input type="submit" value="Submit">
  </form><br>
  <p>Fuel Flow Tolerance(0-100): <span id='idfuelTolerance'></span></p>
     <form action="/get">
   <input type="number" name="fuelTolerance" min="0">
    <input type="submit" value="Submit">
  </form><br>
    </fieldset>  

 <br>
  <br>
   <script>
 function setCustomCal() {
 var xhttp = new XMLHttpRequest();
 xhttp.open("GET", "/useCusCal", true);
 xhttp.send();
}
 function setDefaultCal() {
 var xhttp = new XMLHttpRequest();
 xhttp.open("GET", "/useDefCal", true);
 xhttp.send();
}

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
              document.CalibrationType.pressCalType.value=data.pressCalType;
            document.getElementById("idmaxRPM").innerText = data.maxRPM;
            document.getElementById("ididleRPM").innerText = data.idleRPM;
            document.getElementById("idrpmTolerance").innerText = data.rpmTolerance;
            document.getElementById("idglowOnRPM").innerText = data.glowOnRPM;
            document.getElementById("idglowOffRPM").innerText = data.glowOffRPM;
            document.getElementById("idignitionRPMHigh").innerText = data.ignitionRPMHigh;
            document.getElementById("idignitionRPMLow").innerText = data.ignitionRPMLow;
            document.getElementById("idgasOnRPM").innerText = data.gasOnRPM;
            document.getElementById("idgasOffRPM").innerText = data.gasOffRPM;
            document.getElementById("idstarterOffRPM").innerText = data.starterOffRPM;
            document.getElementById("idfuelOnRPM").innerText = data.fuelOnRPM;
             document.getElementById("ididlePress").innerText = data.idlePress;
             document.getElementById("idmaxPress").innerText = data.maxPress;
             document.getElementById("idpressTolerance").innerText = data.pressTolerance;
               document.getElementById("idcasePress").innerText = data.casePress;
                document.getElementById("idcalPress0").innerText = data.calPress0;
                document.getElementById("idcalPress1").innerText = data.calPress1;
                document.getElementById("idcalVolt0").innerText = data.calVolt0;
                 document.getElementById("idcalVolt1").innerText = data.calVolt1;
             document.getElementById("idignThrottle").innerText = data.ignThrottle;
             document.getElementById("idthrDelay").innerText = data.thrDelay;
              document.getElementById("idthrDelay2").innerText = data.thrDelay2;
             document.getElementById("ididleFuel").innerText = data.idleFuel;
             document.getElementById("idmaxFuel").innerText = data.maxFuel;
              document.getElementById("idfuelTolerance").innerText = data.fuelTolerance;
             
          } else { // a problem occurred
            document.getElementById("idmaxRPM").innerText = "?";
            document.getElementById("ididleRPM").innerText = "?";
            document.getElementById("idrpmTolerance").innerText = "?";
            document.getElementById("idglowOnRPM").innerText = "?";
            document.getElementById("idglowOffRPM").innerText = "?";
            document.getElementById("idignitionRPMHigh").innerText = "?";
            document.getElementById("idignitionRPMLow").innerText = "?";
            document.getElementById("idgasOnRPM").innerText = "?";
            document.getElementById("idgasOffRPM").innerText = "?";
            document.getElementById("idstarterOffRPM").innerText = "?";
            document.getElementById("idfuelOnRPM").innerText = "?";
            document.getElementById("ididlePress").innerText = "?";
            document.getElementById("idmaxPress").innerText = "?";
            document.getElementById("idpressTolerance").innerText ="?";
            document.getElementById("idcasePress").innerText ="?";
            document.CalibrationType.pressCalType.value="?";
              document.getElementById("idcalPress0").innerText = "?";
                document.getElementById("idcalPress1").innerText = "?";
                document.getElementById("idcalVolt0").innerText = "?";
                 document.getElementById("idcalVolt1").innerText ="?";
            document.getElementById("idignitionThrottle").innerText = "?";
            document.getElementById("idthrottleHighTime").innerText = "?";
            document.getElementById("idthrottleLowTime").innerText = "?";
            document.getElementById("ididleFuel").innerText = "?";
            document.getElementById("idmaxFuel").innerText = "?";
            document.getElementById("idfuelTolerance").innerText = "?";
        
          }
        } else {
          console.log('Request failed.  Returned status of ' + xhr.status);
               document.getElementById("idmaxRPM").innerText = "?";
            document.getElementById("ididleRPM").innerText = "?";
            document.getElementById("idrpmTolerance").innerText = "?";
            document.getElementById("idglowOnRPM").innerText = "?";
            document.getElementById("idglowOffRPM").innerText = "?";
            document.getElementById("idignitionRPMHigh").innerText = "?";
            document.getElementById("idignitionRPMLow").innerText = "?";
            document.getElementById("idgasOnRPM").innerText = "?";
            document.getElementById("idgasOffRPM").innerText = "?";
            document.getElementById("idstarterOffRPM").innerText = "?";
            document.getElementById("idfuelOnRPM").innerText = "?"; 
              document.getElementById("ididlePress").innerText = "?";
             document.getElementById("idmaxPress").innerText = "?";
              document.getElementById("idpressTolerance").innerText ="?";
            document.getElementById("idcasePress").innerText ="?";
             document.CalibrationType.pressCalType.value="?";
            document.getElementById("idcalPress0").innerText = "?";
                document.getElementById("idcalPress1").innerText = "?";
                document.getElementById("idcalVolt0").innerText = "?";
                 document.getElementById("idcalVolt1").innerText ="?";
            document.getElementById("idignitionThrottle").innerText = "?";
            document.getElementById("idthrottleHighTime").innerText = "?";
              document.getElementById("idthrottleLowTime").innerText = "?";
            document.getElementById("ididleFuel").innerText = "?";
            document.getElementById("idmaxFuel").innerText = "?";
            document.getElementById("idfuelTolerance").innerText = "?";
           
             
           
           
         
          
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
