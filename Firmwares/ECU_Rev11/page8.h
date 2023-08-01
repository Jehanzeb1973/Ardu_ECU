const char htmlPage8[] PROGMEM = R"=====(
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
    <title>Ardu ECU - Page7</title>

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
 <p>  <h4>Data Files</h4> </p>
 %PAGE8PLACEHOLDER%  
  
 </div>



</body>
</html>
)=====";
