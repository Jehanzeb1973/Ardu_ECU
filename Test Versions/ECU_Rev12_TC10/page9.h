const char htmlPage9[] PROGMEM = R"=====(
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
    <title>Ardu ECU - Page9</title>

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
      .buttongreen {background-color: #4CAF50;} /* Green */
      .buttongray {background-color: #e7e7e7; color: black;} /* Gray */
      .progress-wrapper {
    width:100%;
}

.progress-bar {
  width: 100%;
  height: 20px;
  background-color: #ddd;
}
#progress {

  width: 0%;
  height: 100%;
  background-color: #4CAF50;
 
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
            <a  href="/page9" class="button buttongreen">Page9</a>
        </p>

    
    </div>
  <hr/>
 
<p>  <h4>Load Settings From File</h4> </p>

<!-- form action="/handleupload" enctype="multipart/form-data" target="_blank" method="post" -->
<form id="upload-form">
  <input type="file" id="file-input" accept=".doc,.docx,.txt,application/msword,application/vnd.openxmlformats-officedocument.wordprocessingml.document"/>
  <input type="submit" value="Upload" />
 <progress id="progress" value="0" max="100"></progress>
</form>
  <p>  <h4>Save Settings</h4> </p>
     <a href="/SaveSettings" class="button buttongray" >Save Settings</a>
                               
     <p>  <h4>File Recording</h4> </p>
      <a href="/StopRecording" class="button buttongray" >Stop Recording</a>
            <a  href="/StartRecording" class="button buttongray">Start Recording</a>
            <a href="/NewFile" class="button buttongray">Open New File</a>
              <a href="/DeleteAll" class="button buttongray">Delete All Files</a>
       
 <br>
  <hr/>

 <p>  <h4>Data Files</h4> </p>
   <br>
  <hr/>
 %PAGE9PLACEHOLDER%  
  
<script>
const form = document.getElementById('upload-form');
const fileInput = document.getElementById('file-input');
const progressBar = document.getElementById('progress');

form.addEventListener('submit', e => {
  e.preventDefault();

  const file = fileInput.files[0];
  const formData = new FormData();
  formData.append('file', file);

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/upload', true);
  xhr.upload.onprogress = e => {
    if (e.lengthComputable) {
      const percentComplete = (e.loaded / e.total) * 100;
      progressBar.style.width = `${percentComplete}%`;
   
     
    }
  };
  xhr.send(formData);
});
</script>
</body>
</html>
)=====";
