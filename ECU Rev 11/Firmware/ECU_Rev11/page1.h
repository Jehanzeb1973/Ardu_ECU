 const char index_html[] PROGMEM = R"=====(
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
        .switch {position: relative; display:inline-block;margin-left:15%; width:60px; height:34px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 17px}
    .slider:before {position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 34px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px)}
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

      <p>Max Temp(degC): <span id='idmaxTemp'></span></p>
    <form action="/get">
  <input type="number" name="maxTemp" min="100" max="1400"> 
    <input type="submit" value="Submit">
  </form><br>
  <p>Max Temp Gradient(degC/sec): <span id='idmaxTempGrad'></span></p>
    <form action="/get">
  <input type="number" name="maxTempGrad" min="0" max="600"> 
    <input type="submit" value="Submit">
  </form><br>
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
    <p>StartingTemp: <span id='idstartingTemp'></span></p>
   <form action="/get">
   <input type="number" name="startingTemp" min="0">
    <input type="submit" value="Submit">
  </form><br>    
   <p>Pump On Value: <span id='idpumpOnValue'></span></p>
   <form action="/get">
   <input type="number" name="pumpOnValue" min="0">
    <input type="submit" value="Submit">
  </form><br>
   <p>Input Voltage: <span id='idvoltIn'></span></p>
  <form action="/get">
   <input type="number" name="voltIn" min="0" step=0.01 >
    <input type="submit" value="Submit">
  </form><br>
   <hr/>
   
 %BUTTONPLACEHOLDER% 

   <script>
   
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
     
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
    requestData(); // get intial data straight away 
  
    // request data updates every 1000 milliseconds
    setInterval(requestData, 1000);

    function requestData() {

      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/param");

      xhr.onload = function() {
        if (xhr.status === 200) {

          if (xhr.responseText) { // if the returned data is not null, update the values

            var data = JSON.parse(xhr.responseText);

         
            document.getElementById("idmaxTemp").innerText = data.maxTemp;
            document.getElementById("idmaxTempGrad").innerText = data.maxTempGrad;
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
            document.getElementById("idstartingTemp").innerText = data.startingTemp;
            document.getElementById("idpumpOnValue").innerText = data.pumpOnValue;
            document.getElementById("idvoltIn").innerText = data.voltIn;
                     } 
                     else { // a problem occurred
           
            document.getElementById("idmaxTemp").innerText = "?";
            document.getElementById("idmaxTempGrad").innerText = "?";
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
            document.getElementById("idstartingTemp").innerText = "?";
            document.getElementById("idpumpOnValue").innerText = "?";
            document.getElementById("idvoltIn").innerText = "?";
                    }
        } else {
          console.log('Request failed.  Returned status of ' + xhr.status);

            document.getElementById("idmaxTemp").innerText = "?";
            document.getElementById("idmaxTempGrad").innerText = "?";
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
            document.getElementById("idstartingTemp").innerText = "?";
            document.getElementById("idpumpOnValue").innerText = "?";
            document.getElementById("idvoltIn").innerText = "?";
            }
      };
      
      xhr.send();
    }
    
  </script>
</body>

</html>
)=====";
