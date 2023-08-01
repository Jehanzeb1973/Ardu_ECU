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
    <title>Ardu ECU - Page5</title>

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
   <div class="card">
  <h4>Ardu_ECU Variables at Abort time</h4>
  <p>RPM:<span id="abRPMAvg">0</span></p>
  <p>Temperature:<span id="abexTemp">0</span></p>
  <p>Fuel Flow:<span id="abfuelFlowNow">0</span></p>
  <p>Starter:<span id="abstartMotorNow">0</span></p>
  <p>Gas Flow:<span id="abgasFlow">0</span></p>
  <p>Fuel Flow:<span id="abfuelFlow">0</span></p>
  <p>Ignition State:<span id="abignitionState">0</span></p>
  <p>RC Mode:<span id="abrcModeSignal">0</span></p>
  <p>RC Throttle:<span id="abrcThrottleSignal">0</span></p>
  <p>Glow Signal:<span id="abglowPower">0</span></p>
  <p>ECU Mode:<span id="abengineMode">0</span></p>
  <p>StartStage:<span id="abstartStage">0</span></p>
  <p>Battery Voltage:<span id="abvoltage">0</span></p>
  <p>Error Code:<span id="aberrorCode">0</span></p>
  
 </div>
<script>

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
    var xhr = new XMLHttpRequest();
      xhr.open("GET", "/abortdata");

      xhr.onload = function() {
        if (xhr.status === 200) {

          if (xhr.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(xhr.responseText);

            document.getElementById("abRPMAvg").innerText = data.abRPMAvg;
            document.getElementById("abexTemp").innerText = data.abexTemp;
            document.getElementById("abfuelFlowNow").innerText = data.abfuelFlowNow;
            document.getElementById("abstartMotorNow").innerText = data.abstartMotorNow;
            document.getElementById("abgasFlow").innerText = data.abgasFlow;
             document.getElementById("abfuelFlow").innerText = data.abfuelFlow;
            document.getElementById("abignitionState").innerText = data.abignitionState;
            document.getElementById("abrcModeSignal").innerText = data.abrcModeSignal;
            document.getElementById("abrcThrottleSignal").innerText = data.abrcThrottleSignal;
            document.getElementById("abglowPower").innerText = data.abglowPower;
            document.getElementById("abengineMode").innerText = data.abengineMode;
            document.getElementById("abstartStage").innerText = data.abstartStage;
            document.getElementById("abvoltage").innerText = data.abvoltage;
            document.getElementById("aberrorCode").innerText = data.aberrorCode;
           

          } else { // a problem occurred

            document.getElementById("abRPMAvg").innerText = "?";
            document.getElementById("abexTemp").innerText = "?";
            document.getElementById("abfuelFlowNow").innerText = "?";
            document.getElementById("abstartMotorNow").innerText = "?";
            document.getElementById("abgasFlow").innerText = "?";
             document.getElementById("abfuelFlow").innerText = "?";
            document.getElementById("abignitionState").innerText = "?";
            document.getElementById("abrcModeSignal").innerText = "?";
            document.getElementById("abrcThrottleSignal").innerText ="?";
            document.getElementById("abglowPower").innerText = "?";
            document.getElementById("abengineMode").innerText = "?";
            document.getElementById("abstartStage").innerText = "?";
            document.getElementById("abvoltage").innerText = "?";
            document.getElementById("aberrorCode").innerText = "?";
          }
        } else {
          console.log('Request failed.  Returned status of ' + xhr.status);

          document.getElementById("abRPMAvg").innerText = "?";
            document.getElementById("abexTemp").innerText = "?";
            document.getElementById("abfuelFlowNow").innerText = "?";
            document.getElementById("abstartMotorNow").innerText = "?";
            document.getElementById("abgasFlow").innerText = "?";
             document.getElementById("abfuelFlow").innerText = "?";
            document.getElementById("abignitionState").innerText = "?";
            document.getElementById("abrcModeSignal").innerText = "?";
            document.getElementById("abrcThrottleSignal").innerText ="?";
            document.getElementById("abglowPower").innerText = "?";
            document.getElementById("abengineMode").innerText = "?";
            document.getElementById("abstartStage").innerText = "?";
            document.getElementById("abvoltage").innerText = "?";
            document.getElementById("aberrorCode").innerText = "?";

        }
      };
      
      xhr.send();
}
</script>
</body>
</html>
)=====";
