const char htmlPage6[] PROGMEM = R"=====(
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
    <title>Ardu ECU - Page6</title>

<style>
/*# sourceMappingURL=bootstrap.min.css.map */
font-family: Arial, Helvetica, sans-serif; 
  text-align: center;

h1 {
  font-size: 1.8rem; 
  color: white;
   margin: 5%;
}

p { 
  align-items: center;
 // font-size: 1.4rem;
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

.card-grid { 
  max-width: 1200px; 
  margin: 0 auto; 
  display: grid; 
  grid-gap: 2rem; 
  grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
}
.card { 
  background-color: white; 
  align-items: center;
  text-align: center;
  box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
}
.card-title { 
  align-items: center;
  text-align: center;
  font-size: 1.2rem;
  font-weight: bold;
  color: #034078
}
    #item {
      margin: 30px;
      width: 150px;
      height: 150px;
      background-color: #EEE;
      border: 5px dashed #111;
      border-radius: 50%;
    }

    #item:hover {
      background-color: #CCC;
      cursor: pointer;
    }

</style>
</head>

<body>
    <div class="container">
        <h1>Ardu_ECU</h1>
        <p>Arduino ECU Data Terminal</p>
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
    </style>
        <p>
           <a  href="/" class="button">Page1</a>
            <a href="/page2" class="button" >Page2</a>
            <a  href="/page3" class="button">Page3</a>
            <a href="/page4" class="button">Page4</a>
            <a  href="/page5" class="button">Page5</a>
            <a  href="/page6" class="button buttongreen">Page6</a>
            <a  href="/page7" class="button">Page7</a>
            <a  href="/page8" class="button">Page8</a>
            <a  href="/page9" class="button">Page9</a>
        </p>

    
    </div>
    <hr/>
   <div class="content">
    <p>Data_File:<span id="datafilename"></span></p>
      <div class="card-grid">
      <div class="card">
      <p class="card-title">Throttle</p><span id="throttle">%</span>
           </div>
           <div class="card">
      <p class="card-title">Starter</p><span id="starter">%</span>
           </div>
            <div class="card">
      <p class="card-title">Gas</p><span id="gas">%</span>
           </div>
            <div class="card">
      <p class="card-title">Glow</p><span id="glow">%</span>
           </div>
            <div class="card">
      <p class="card-title">Fuel</p><span id="fuel">%</span>
           </div>
        <div class="card">
         <p class="card-title">Temperature</p><span id="temp">%</span>
         </div>
           <div class="card">
         <p class="card-title">Pressure</p><span id="press">%</span>
         </div>
          <div class="card">
         <p class="card-title">Thrust</p><span id="thrust">%</span>
         </div>
         <div class="card">
         <p class="card-title">Temp Gradient</p><span id="tempGrad">%</span>
         </div>
        <div class="card">
          <p class="card-title">RPM</p><span id="rpm">%</span>
        </div>
         <div class="card">
      <p class="card-title">ECU Mode</p><span id="mode"></span>
           </div>
            <div class="card">
      <p class="card-title">Throttle Mode</p><span id="thrmode"></span>
           </div>
            <div class="card">
      <p class="card-title">Error</p><span id="error"></span>
           </div>
             <div class="card">
      <p class="card-title">Batt.Volt</p><span id="batvolt"></span>
           </div>
             <div class="card">
      <p class="card-title">MaxLoopTime(millisec)</p><span id="looptime"></span>
           </div>
            <div class="card">
      <p class="card-title">SDLoopTime(millisec)</p><span id="sdlooptime"></span>
           </div>
      </div>
      <div>
      <p>  <h4>Waiting Mode Throttle Stick Function</h4> </p>
      <a href="/ModeNormal" class="button" >Starter Motor Control</a>
         <!--   <a href="/ModeFuel" class="button">Manual Fuel Control</a> -->
            <a  href="/PumpPrime" class="button">Fuel Pump Control</a>          
             <a  href="/ResetError" class="button">Clear Error</a>
               <hr/>
             <a  href="/testFuel" class="button">Test Fuel Valve</a>          
             <a  href="/testGas" class="button">Test Gas Valve</a>
           <a  href="/testGlow" class="button">Test Glow Plug</a> 
                   
      
    <script>
  
  
     function startGlowTest() {
           var xhr = new XMLHttpRequest();          
      xhr.open("GET", "/TestGlow");
      xhr.send();
    }
    function stopGlowTest() {
      document.getElementById("btnGlow").className = "release";
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/stopTestGlow");
      xhr.send();
    }
 
  window.addEventListener('load', getReadings);
 
sendkeepalive(); // get intial data straight away 
  
    // request data updates every 4000 milliseconds
    setInterval(sendkeepalive, 4000);

    function sendkeepalive() {
          var xhr2 = new XMLHttpRequest();
      xhr2.open("GET", "/keepalive");
      xhr2.send();
    }



// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var datafilename=myObj.datafilename;
      var temp = myObj.temperature;
       var tempGrad = myObj.tempGrad;
      var rpm = myObj.rpm;
      var throttle = myObj.throttle;
       var starter = myObj.starter;
        var glow = myObj.glow;
       var fuel = myObj.fuel;
        var gas = myObj.gas;
         var ecumode = myObj.ecumode;
         var thrmode = myObj.thrmode;
           var pressure = myObj.casepressure;
           var thrust = myObj.loadcellvalue;
          var error = myObj.error;
          var batvolt = myObj.batvolt;
          var looptime = myObj.looptime;
               var sdlooptime = myObj.sdlooptime;
      document.getElementById("datafilename").innerHTML = datafilename;
     document.getElementById("throttle").innerHTML = throttle;
      document.getElementById("temp").innerHTML = temp;
      document.getElementById("press").innerHTML = pressure;
         document.getElementById("thrust").innerHTML = thrust;
        document.getElementById("tempGrad").innerHTML = tempGrad;
      document.getElementById("starter").innerHTML= starter;
      document.getElementById("glow").innerHTML = glow;
      document.getElementById("fuel").innerHTML = fuel;
      document.getElementById("rpm").innerHTML = 1000*rpm;
      document.getElementById("gas").innerHTML=gas;
      document.getElementById("mode").innerHTML=ecumode;
      document.getElementById("thrmode").innerHTML=thrmode;
      document.getElementById("error").innerHTML=error;
      document.getElementById("batvolt").innerHTML=batvolt;
      document.getElementById("looptime").innerHTML=looptime;
       document.getElementById("sdlooptime").innerHTML=sdlooptime;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
     document.getElementById("datafilename").innerHTML = myObj.datafilename;
     document.getElementById("throttle").innerHTML = myObj.throttle;
      document.getElementById("temp").innerHTML = myObj.temperature;
        document.getElementById("press").innerHTML = myObj.casepressure;
        document.getElementById("thrust").innerHTML = myObj.loadcellvalue;
      document.getElementById("tempGrad").innerHTML = myObj.tempGrad;
      document.getElementById("starter").innerHTML= myObj.starter;
      document.getElementById("glow").innerHTML = myObj.glow;
      document.getElementById("fuel").innerHTML = myObj.fuel;
      document.getElementById("rpm").innerHTML = 1000*myObj.rpm;
      document.getElementById("gas").innerHTML=myObj.gas;
      document.getElementById("mode").innerHTML=myObj.ecumode;
      document.getElementById("thrmode").innerHTML=myObj.thrmode;
      document.getElementById("error").innerHTML=myObj.error;
      document.getElementById("batvolt").innerHTML=myObj.batvolt;
      document.getElementById("looptime").innerHTML=myObj.looptime;
       document.getElementById("sdlooptime").innerHTML=myObj.sdlooptime;
  }, false);
}
  </script>
           
  </body>

</html>
)=====";
