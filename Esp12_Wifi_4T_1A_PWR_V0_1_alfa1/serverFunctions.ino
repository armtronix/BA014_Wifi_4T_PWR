// -- BEGIN mainpage.html
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ARMtronix Technologies LLP</title>
<style>
body { background: #ffffff repeat; font-family: Arial, Helvetica, Sans-Serif; Color:  #ff0000; }
#container { width: 80%; max-width: 450px; margin: auto; }
.bulb { display: block; clear: none; width: 32px; height: 32px; padding-bottom: 0.2em; background-attachment: fixed; background-position: center; background-repeat: no-repeat; }
.bulbOn { background: transparent url('http://sasakaranovic.com/iotdimmer/bulbON.png') top left no-repeat; float: right; }
.bulbOff{ background: transparent url('http://sasakaranovic.com/iotdimmer/bulbOFF.png') top left no-repeat; float: left; }
.image {font-size: 0; vertical-align: middle; text-align: center; width: 150px;  height: 100px; margin: auto}
img {display: inline-block; vertical-align: middle; max-height: 100%; max-width: 100%;}
.trick {display: inline-block; vertical-align: middle; height: 150px; margin-left: 0; margin-right: 0; }

h1 {  display: block; font-size: 1.2em; margin-top: 0.10em; margin-bottom: 0.10em; margin-left: 0; margin-right: 0; font-weight: bold; text-align: center; }
.slidecontainer {width: 100%; }
.slider {width: 100%; margin: 0 0 2em 0; }
  .buttonOff { background-color: #ff0000;margin-top: 0.1em; margin-bottom: 0.1em }
  .buttonOn{ background-color: #4CAF50;margin-top: 0.1em; margin-bottom: 0.1em }
  a { 
  background-color: #212121;
    border: none;
    color: white;
    padding: 15px 20px;
  margin-right: 1em;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
  border-radius: 4px;
  }
</style>
<script>
var websock;
var xmlHttp=createXmlHttpObject();

function start() 
{
 websock = new WebSocket('ws://' + window.location.hostname + ':81/');
 websock.onopen = function(evt) { console.log('websock open'); };
 websock.onclose = function(evt) { console.log('websock close'); };
 websock.onerror = function(evt) { console.log(evt); };
 websock.onmessage = function(evt) 
 {
  console.log(evt);
  var ch1 = document.getElementById('ch1');
  var ch2 = document.getElementById('ch2');
  var ch3 = document.getElementById('ch3');
  var ch4 = document.getElementById('ch4');
  var ch_values = JSON.parse(evt.data);
  ch1.value = ch_values.ch1;
  ch2.value = ch_values.ch2;
  ch3.value = ch_values.ch3;
  ch4.value = ch_values.ch4;
 };
 process();
}

function setCh1(val) 
{
 var dat = 'ch1:'+ val
 websock.send(dat);
}

function setCh2(val) 
{
 var dat = 'ch2:'+ val
 websock.send(dat);
}

function setCh3(val) 
{
 var dat = 'ch3:'+ val
 websock.send(dat);
}

function setCh4(val) 
{
 var dat = 'ch4:'+ val
 websock.send(dat);
}

function updateSlider(e) 
{
 var dat = e.id +':'+ e.value
 websock.send(dat);
}

function createXmlHttpObject()
{
 if(window.XMLHttpRequest)
   {
    xmlHttp=new XMLHttpRequest();
   }
 else
   {
    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');
   }
 return xmlHttp;
}

function process()
{
 if(xmlHttp.readyState==0 || xmlHttp.readyState==4)
   {
    xmlHttp.open('PUT','xml',true);
    xmlHttp.onreadystatechange=handleServerResponse;
    xmlHttp.send(null);
   }
 setTimeout('process()',500);
}
 
function handleServerResponse()
{
 if(xmlHttp.readyState==4 && xmlHttp.status==200)
   {
    xmlResponse=xmlHttp.responseXML;
    xmldoc1 = xmlResponse.getElementsByTagName('activepower');
    xmldoc2 = xmlResponse.getElementsByTagName('t1');
    xmldoc3 = xmlResponse.getElementsByTagName('t2');
    xmldoc4 = xmlResponse.getElementsByTagName('t3');
    xmldoc5 = xmlResponse.getElementsByTagName('dim');
    message1 = xmldoc1[0].firstChild.nodeValue;
    message2 = xmldoc2[0].firstChild.nodeValue;
    message3 = xmldoc3[0].firstChild.nodeValue;
    message4 = xmldoc4[0].firstChild.nodeValue;
    message5 = xmldoc5[0].firstChild.nodeValue;
    document.getElementById('runtime1').innerHTML=message1;
    document.getElementById('runtime2').innerHTML=message2;
    document.getElementById('runtime3').innerHTML=message3;
    document.getElementById('runtime4').innerHTML=message4;
    document.getElementById('runtime5').innerHTML=message5;
   }
}
</script>
</head>
<body onload="javascript:start();">
<div id="container">
<div class="image">
<div class="trick"></div>
  <img src='http://armtronix.in/img/logo.png'/>
</div>
<h1>Wifi 4T with PWR Board</h1>
<div class="slidecontainer">
<div class="bulb bulbOn"></div>
<div class="bulb bulbOff"></div>
<input id="ch1" type="range" min="0" max="99" value="0" class="slider" onchange="updateSlider(this)">
</div>
<div class="slidecontainer"style="display:none; visibility: hidden">
<div class="bulb bulbOn"></div>
<div class="bulb bulbOff"></div>
<input id="ch2" type="range" min="0" max="100" value="0" class="slider" onchange="updateSlider(this)">
</div>
<div class="slidecontainer" style="display:none; visibility: hidden">
<div class="bulb bulbOn"></div>
<div class="bulb bulbOff"></div>
<input id="ch3" type="range" min="0" max="100" value="0" class="slider" onchange="updateSlider(this)">
</div>
<div class="slidecontainer" style="display:none; visibility: hidden">
<div class="bulb bulbOn"></div>
<div class="bulb bulbOff"></div>
<href="#d" input id="ch4" type="range" min="0" max="100" value="0" class="slider" onchange="updateSlider(this)">
</div>
<div class="slidecontainer" style="text-align: center">
<a href="#d0" class="buttonOff" onClick="setCh1(0)"> Off </a> <a href="#d50" onClick="setCh1(50)"> 50% </a> <a href="#d100" class="buttonOn"  onClick="setCh1(99)"> 100% </a>
</div>
<div class="slidecontainer" style="text-align: center">
<a href="#t1"  class="buttonOn"  id='runtime2' onClick="setCh2(100)"></a> <a href="#t2" class="buttonOn"  id='runtime3' onClick="setCh3(100)"></a> <a href="#t3" class="buttonOn" id='runtime4' onClick="setCh4(100)"></a>
</div>
<div class="slidecontainer" style="text-align: center">
<a class="buttonOff"<label>Active Power:</label></a>
<a class="buttonOff" id='runtime1'></a>
</div>
<div class="slidecontainer" style="text-align: center">
<a class="buttonOff"<label>Dimmer:</label></a>
<a class="buttonOff" id='runtime5'></a>
</div>
</div>
</body>
</html>
)rawliteral";

/*
<div class="slidecontainer" style="text-align: center">
<a class="buttonOff"<label>Load One:</label></a>
<a class="buttonOff" id='runtime2'></a>
</div>
<div class="slidecontainer" style="text-align: center">
<a class="buttonOff"<label>Load Two:</label></a>
<a class="buttonOff" id='runtime3'></a>
</div>
<div class="slidecontainer" style="text-align: center">
<a class="buttonOff"<label>Load Three:</label></a>
<a class="buttonOff" id='runtime4'></a>
</div>
*/
// ------------------------------------ END Index.html -----------------------------------------------------------------------------------------------------------------------------------
//url('https://litumiot.com/wp-content/uploads/2016/10/page-center-background-image-iot-2.jpg')

// ------------------------------------  MAIN.html -----------------------------------------------------------------------------------------------------------------------------------

static const char PROGMEM MAIN_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ARMtronix Technologies LLP</title>
<style>
body { background: #ffffff  repeat; font-family: Arial, Helvetica, Sans-Serif; Color:  #f44336; }
#container { width: 80%; max-width: 450px; margin: auto; }
.image {font-size: 0; vertical-align: middle; text-align: center; width: 200px;  height: 100px;}
img {display: inline-block; vertical-align: middle; max-height: 100%; max-width: 100%;}
.trick {display: inline-block; vertical-align: middle; height: 150px; margin-left: 0; margin-right: 0;}
h1 {  display: block; font-size: 2em; margin-top: 0.67em; margin-bottom: 0.67em; margin-left: 0; margin-right: 0; font-weight: bold; text-align: center; color: #f44336; }
  a { 
  background-color: #ff0000;
    border: none;
    color: white;
    padding: 15px 32px;
  margin-top: 0.67em; 
  margin-bottom: 0.67em;
  margin-right: 0em;
  margin-left: 0em; 
    text-align: center;
    text-decoration: none;
    display: block;
    font-size: 18px;
    font-weight: bold;
  border-radius: 4px;
  }
</style>
</head>
<body>
<div id="container">
<div class="image">
<div class="trick"></div>
  <img src='http://armtronix.in/img/logo.png'/>
</div>
<h1>Wifi 4T with PWR Board</h1>
<a href=\gpio>Control Outputs</a><br />
<a href=\cleareeprom>Clear settings an boot into Config mode</a><br />
</div>
</body>
</html>
)rawliteral";
// ------------------------------------ END MAIN.html -----------------------------------------------------------------------------------------------------------------------------------

static const char PROGMEM CONFIG_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ARMtronix Technologies LLP</title>
</head>
<style>
body { background: #ffffff  repeat; font-family: Arial, Helvetica, Sans-Serif; font-weight: bold; Color:  #f44336; }
h1 {  display: block; font-size: 1.80em; margin-top: 0.67em; margin-bottom: 0.67em; margin-left: 0; margin-right: 0; font-weight: bold; text-align: center; color: #f44336; }
input[type=text] {
  width: 100%;
  padding: 8px 20px;
  margin: 8px 0;
  box-sizing: border-box;
}
input[type=password] {
  width: 100%;
  padding: 8px 20px;
  margin: 8px 0;
  box-sizing: border-box;
}
input[type=submit]{
  background-color: #ff0000;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-weight: bold; 
  box-sizing: border-box;
}
</style>
<body>
<h1>Wifi 4T with PWR Board</h1>
<form method='get' action='a'>
<label>SSID: </label><input type="text" name='ssid' length=32><label> Pass: </label><input name='pass' type='password' length=64></br>
<label>IOT Mode: </label><input type='radio' name='iot' value='0'>HTTP <input type='radio' name='iot' value='1' checked> MQTT</br>
<label>MQTT Broker IP/DNS: </label><input type="text" name='host' length=15></br>
<label>MQTT Publish too topic: </label><input type="text" name='pubtop' length=64></br>
<label>MQTT Subscrbe too topic: </label><input type="text" name='subtop' length=64></br>
<label>Power Consumed: </label><input type="text" name='power' length=64></br>
<label>Input AC Voltage: </label><input type="text" name='inputvoltage' length=64></br>
<input type='submit'>
</form>
</body>
</html>
)rawliteral";


void initWiFi(){
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  
  // test esid 
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi ");
  Serial.println(esid);
  Debugln(epass);
  WiFi.begin((char*)esid.c_str(), (char*)epass.c_str());
  if ( testWifi() == 20 ) { 
      launchWeb(0);
      return;
  }
  Serial.println("Opening AP");
  setupAP();   
}

int testWifi(void) {
  int c = 0;
  Debugln("Wifi test...");  
  while ( c < 30 ) {
    if (WiFi.status() == WL_CONNECTED) { return(20); } 
    delay(500);
    Serial.print(".");    
    c++;
  }
  Serial.println("WiFi Connect timed out!");
  return(10);
} 


void setupAP(void) {
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0){
    Serial.println("no networks found");
    st ="<b>No networks found:</b>";
  } else {
    Serial.print(n);
    Serial.println(" Networks found");
    st = "<ul>";
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" (OPEN)":"*");
      
      // Print to web SSID and RSSI for each network found
      st += "<li>";
      st +=i + 1;
      st += ": ";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" (OPEN)":"*";
      st += "</li>";
      delay(10);
     }
    st += "</ul>";
  }
  Serial.println(""); 
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP);

  
  WiFi.softAP(host);
  WiFi.begin(host); // not sure if need but works
  Serial.print("Access point started with name ");
  Serial.println(host);
  inApMode=1;
  launchWeb(1);
}

void launchWeb(int webtype) {
    Serial.println("");
    Serial.println("WiFi connected");    
    //Start the web server or MQTT
    if(otaFlag==1 && !inApMode){
      Serial.println("Starting OTA mode.");    
      Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
      Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());
      MDNS.begin(host);
      server.on("/", HTTP_GET, [](){
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/html", otaServerIndex);
      });
      server.on("/update", HTTP_POST, [](){
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
        setOtaFlag(0); 
        ESP.restart();
      },[](){
        HTTPUpload& upload = server.upload();
        if(upload.status == UPLOAD_FILE_START){
          //Serial.setDebugOutput(true);
          WiFiUDP::stopAll();
          Serial.printf("Update: %s\n", upload.filename.c_str());
          otaCount=300;
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if(!Update.begin(maxSketchSpace)){//start with max available size
            Update.printError(Serial);
          }
        } else if(upload.status == UPLOAD_FILE_WRITE){
          if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
            Update.printError(Serial);
          }
        } else if(upload.status == UPLOAD_FILE_END){
          if(Update.end(true)){ //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          } else {
            Update.printError(Serial);
          }
          Serial.setDebugOutput(false);
        }
        yield();
      });
      server.begin();
      Serial.printf("Ready! Open http://%s.local in your browser\n", host);
      MDNS.addService("http", "tcp", https_port_no);
      MDNS.addService("ws", "tcp", 81);  //added by naren on 02/01/2019
      otaTickLoop.attach(1, otaCountown);
    } else { 
      //setOtaFlag(1); 
      if (webtype==1 || iotMode==0){ //in config mode or WebControle
          if (webtype==1) {           
            webtypeGlob == 1;
            Serial.println(WiFi.softAPIP());
            server.on("/", webHandleConfig);
            server.on("/a", webHandleConfigSave);
            server.on("/gpio", webHandleGpio);//naren          
          } else {
             webSocket.begin();  //added by naren on 02/01/2019
             webSocket.onEvent(webSocketEvent);  //added by naren on 02/01/2019
             Serial.println("setting up MDNS responder now!");
            //setup DNS since we are a client in WiFi net
            if (!MDNS.begin(host)) {
              Serial.println("Error setting up MDNS responder!");
            } else {
              Serial.println("mDNS responder started");
              MDNS.addService("http", "tcp", https_port_no);
            }          
            Serial.println(WiFi.localIP());
            server.on("/", webHandleRoot);  
            server.on("/cleareeprom", webHandleClearRom);
            server.on("/gpio", webHandleGpio);
            server.on("/xml",handleXML);
          }
          //server.onNotFound(webHandleRoot);
          server.begin();
          Serial.println("Web server started");   
          webtypeGlob=webtype; //Store global to use in loop()
        } else if(webtype!=1 && iotMode==1){ // in MQTT and not in config mode     
          //mqttClient.setBrokerDomain((char*) mqttServer.c_str());//naren
          //mqttClient.setPort(1883);//naren
          Serial.println(mqtt_port.toInt()); //added by naren on 14/02/2019
          mqttClient.setServer((char*) mqttServer.c_str(),mqtt_port.toInt()); //added by naren on 14/02/2019
          mqttClient.setCallback(mqtt_arrived);
          mqttClient.setClient(wifiClient);
          if (WiFi.status() == WL_CONNECTED){
            if (!connectMQTT()){
                delay(2000);
                if (!connectMQTT()){                            
                  Serial.println("Could not connect MQTT.");
                  Serial.println("Starting web server instead.");
                  iotMode=0;
                  launchWeb(0);
                  webtypeGlob=webtype;
                }
              }                    
            }
      }
    }
}

void webHandleConfig(){
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  String s;
 
  s = "Configuration of " + hostName + " at ";
  s += ipStr;
  s += "<p><a href=\"/gpio\">Control GPIO</a><br />";
  s += st;
  s += "<form method='get' action='a'>";
  s += "<label>SSID: </label><input name='ssid' length=32><label> Pass: </label><input name='pass' type='password' length=64></br>";
  s += "The following is not ready yet!</br>";
  s += "<label>IOT mode: </label><input type='radio' name='iot' value='0'> HTTP<input type='radio' name='iot' value='1' checked> MQTT</br>";
  s += "<label>MQTT Broker IP/DNS: </label><input name='host' length=15></br>";
  s += "<label>MQTT User Name: </label><input name='mqtt_user' length=15></br>";
  s += "<label>MQTT Password: </label><input name='mqtt_passwd' type='password' length=15></br>";
  s += "<label>MQTT Port: </label><input name='mqtt_port' value='1883' length=15></br>";
  s += "<label>MQTT Publish topic: </label><input name='pubtop' length=64></br>";
  s += "<label>MQTT Subscribe topic: </label><input name='subtop' length=64></br>";
  s += "<label>Power Consumed: </label><input type='text' name='power' length=64></br>";
  s += "<label>Input AC Voltage: </label><input type='text' name='inputvoltage' length=64></br>";
  s += "<input type='submit'></form></p>";
  s += "\r\n\r\n";
  Serial.println("Sending 200");  
  server.send(200, "text/html", s); 
   //server.send_P(200, "text/html", CONFIG_HTML);
}

void webHandleConfigSave(){
  // /a?ssid=blahhhh&pass=poooo
  String s;
  s = "<p>Plz wait, settings saved to eeprom and resetting to boot into new settings</p>\r\n\r\n";
  server.send(200, "text/html", s); 
  Serial.println("clearing EEPROM.");
  clearConfig();
  String qsid; 
  qsid = server.arg("ssid");
  qsid.replace("%2F","/");
  Serial.println("Got SSID: " + qsid);
  esid = (char*) qsid.c_str();
  
  String qpass;
  qpass = server.arg("pass");
  qpass.replace("%2F","/");
  Serial.println("Got pass: " + qpass);
  epass = (char*) qpass.c_str();

  String qiot;
  qiot= server.arg("iot");
  Serial.println("Got iot mode: " + qiot);
  qiot=="0"? iotMode = 0 : iotMode = 1 ;
  
  String qsubTop;
  qsubTop = server.arg("subtop");
  qsubTop.replace("%2F","/");
  Serial.println("Got subtop: " + qsubTop);
  subTopic = (char*) qsubTop.c_str();
  
  String qpubTop;
  qpubTop = server.arg("pubtop");
  qpubTop.replace("%2F","/");
  Serial.println("Got pubtop: " + qpubTop);
  pubTopic = (char*) qpubTop.c_str();
  
  mqttServer = (char*) server.arg("host").c_str();
  Serial.print("Got mqtt Server: ");
  Serial.println(mqttServer);

   String qmqtt_user;//qmqtt_user
  qmqtt_user = server.arg("mqtt_user");
  qmqtt_user.replace("%2F", "/");
  Serial.println("Got mqtt_user: " + qmqtt_user);
  mqtt_user = (char*) qmqtt_user.c_str();

  String qmqtt_passwd;//qmqtt_passwd
  qmqtt_passwd = server.arg("mqtt_passwd");
  qmqtt_passwd.replace("%2F", "/");
  Serial.println("Got mqtt_passwd: " + qmqtt_passwd);
  mqtt_passwd = (char*) qmqtt_passwd.c_str();

  String qmqtt_port;//qmqtt_port
  qmqtt_port = server.arg("mqtt_port");
  qmqtt_port.replace("%2F", "/");
  Serial.println("Got mqtt_port: " + qmqtt_port);
  mqtt_port = (char*) qmqtt_port.c_str();

  String qpower;//qmqtt_port//added on 14/02/2019
  qpower = server.arg("power");//added on 14/02/2019
  qpower.replace("%2F", "/"); //added on 14/02/2019
  Serial.println("Got power: " + qpower); //added on 14/02/2019
  Sptemp=qpower.c_str();  //added on 14/02/2019

  String qinputvoltage;//qmqtt_port //added on 14/02/2019
  qinputvoltage = server.arg("inputvoltage"); //added on 14/02/2019
  qinputvoltage.replace("%2F", "/"); //added on 14/02/2019
  Serial.println("Got input voltage: " + qinputvoltage);  //added on 14/02/2019
  Svtemp=qinputvoltage.c_str();  //added on 14/02/2019


  Serial.print("Settings written ");
  saveConfig()? Serial.println("sucessfully.") : Serial.println("not succesfully!");;
  Serial.println("Restarting!"); 
  delay(1000);
  ESP.reset();
}

void webHandleRoot(){
  String s;
  s = "<p>Hello from ESP8266";
  s += "</p>";
  s += "<a href=\"/gpio\">Controle GPIO</a><br />";
  s += "<a href=\"/cleareeprom\">Clear settings an boot into Config mode</a><br />";
  s += "\r\n\r\n";
  Serial.println("Sending 200");  
  //server.send(200, "text/html", s); 
  server.send_P(200, "text/html",MAIN_HTML);
}

void webHandleClearRom(){
  String s;
  s = "<p>Clearing the config and reset to configure new wifi<p>";
  s += "</html>\r\n\r\n";
  Serial.println("Sending 200"); 
  server.send(200, "text/html", s); 
  Serial.println("clearing config");
  clearConfig();
  delay(10);
  Serial.println("Done, restarting!");
  ESP.reset();
}


void webHandleGpio(){
  String s;
  int state_sw;
  int state_dimmer;
  
   // Set GPIO according to the request
    if (server.arg("state_sw")=="1" || server.arg("state_sw")=="0" ) {
         state_sw = server.arg("state_sw").toInt();       
      if (state_sw == LOW)
      {
      Serial.println("Dimmer:0");
      }
      else
      {
      Serial.println("Dimmer:99");
      }
          
    }
   if (server.arg("state_triac_one")=="1" || server.arg("state_triac_one")=="0" ) 
     {
      toggle_t1();
     }
  if (server.arg("state_triac_two")=="1" || server.arg("state_triac_two")=="0" ) 
     {
      toggle_t2();    
     }
  if (server.arg("state_triac_three")=="1" || server.arg("state_triac_three")=="0" ) 
     {
      toggle_t3(); 
     }
   if (server.arg("state_dimmer") !="") 
     {
      state_dimmer = server.arg("state_dimmer").toInt();
      //digitalWrite(BUILTIN_LED, state_triac_one);
     // Serial.print("Light switched via web request to  "); 
      Serial.print("Dimmer:");     
      Serial.println(state_dimmer); 
      delay(5);   
     }
    s  = "<p>Four Triac Board</p>"; 
    s += "Dimmable TRIAC is now ";
    s += (state_dimmer)?"ON":"OFF";
    s += "<p>Change to <form action='gpio'><input type='radio' name='state_sw' value='1' ";
    s += (state_dimmer)?"checked":"";
    s += ">TRIAC_ON<input type='radio' name='state_sw' value='0' ";
    s += (state_dimmer)?"":"checked";
    s += ">TRIAC_OFF <input type='submit' value='Submit'></form></p>";  

    s += "Non Dimmable Triac One is now ";
    s += (triac_1_state_toggle)?"ON":"OFF";
    s += "<p>Change to <form action='gpio'><input type='radio' name='state_triac_one' value='1' ";
    s += (triac_1_state_toggle)?"checked":"";
    s += ">TRIAC_1_ON <input type='radio' name='state_triac_one' value='0' ";
    s += (triac_1_state_toggle)?"":"checked";
    s += ">TRIAC_1_OFF <input type='submit' value='Submit'></form></p>"; 

    s += "Non Dimmable Triac Two is now ";
    s += (triac_2_state_toggle)?"ON":"OFF";
    s += "<p>Change to <form action='gpio'><input type='radio' name='state_triac_two' value='1' ";
    s += (triac_2_state_toggle)?"checked":"";
    s += ">TRIAC_2_ON <input type='radio' name='state_triac_two' value='0' ";
    s += (triac_2_state_toggle)?"":"checked";
    s += ">TRIAC_2_OFF <input type='submit' value='Submit'></form></p>"; 

    s += "Non Dimmable Triac Three is now ";
    s += (triac_3_state_toggle)?"ON":"OFF";
    s += "<p>Change to <form action='gpio'><input type='radio' name='state_triac_three' value='1' ";
    s += (triac_3_state_toggle)?"checked":"";
    s += ">TRIAC_3_ON <input type='radio' name='state_triac_three' value='0' ";
    s += (triac_3_state_toggle)?"":"checked";
    s += ">TRIAC_3_OFF <input type='submit' value='Submit'></form></p>"; 
    
    s += "<p>Change to <form action='gpio' name='state' method='GET' oninput='showValue(state_dimmer.value)'><input type='range' name='state_dimmer' id='state_dimmer' min='0' max='90' step='5' value='0'  >" ; 
    s += "</form></p>";
    s += "<script type='text/javascript'>";
    s += "function showValue(dimming)";
    s += "{  document.querySelector('#state_dimmer').value=dimming ;";
    s += "   document.forms['state'].submit();";
    s += "}";
    s += "</script>";

   //server.send(200, "text/html", s);
   server.send_P(200, "text/html", INDEX_HTML);  
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
 
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            //Serial.println(num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
           // Serial.printf("[%u] get Text: %s\n", num, payload);
           //  Serial.write((const char *)payload);
            if(payload[2]=='0')
            {
             // Serial.println("%s",payload);
             // tarBrightness =payload;
            }
            if(payload[2]=='1')
            {   String dum1=(const char *)payload;
                //Serial.write((const char *)payload);
                Serial.print("Dimmer:");
                Serial.println(dum1.substring(4));
            }
           if(payload[2]=='2')
            {
              String dum2=(const char *)payload;
              if(dum2.substring(4).toInt()==100)
              {
               toggle_t1();
              }
            }
            if(payload[2]=='3')
            {
              String dum3=(const char *)payload;
              if(dum3.substring(4).toInt()==100)
              {
               toggle_t2();
              }
            }
           if(payload[2]=='4')
            {
              String dum4=(const char *)payload;
              if(dum4.substring(4).toInt()==100)
              {    
               toggle_t3();
              }
            }
            break;
    }
 
}
void toggle_t1()
{
 triac_1_state_toggle=triac_1_state_toggle^1;
 Serial.print("R_1 switched via web request to ");
 Serial.println(((triac_1_state_toggle)));  
}

void toggle_t2()
{
  triac_2_state_toggle=triac_2_state_toggle^1;
  Serial.print("R_2 switched via web request to ");
  Serial.println(((triac_2_state_toggle))); // triac_1_state ^1

}
void toggle_t3()
{
  triac_3_state_toggle=triac_3_state_toggle^1;   
  Serial.print("R_3 switched via web request to ");
  Serial.println(((triac_3_state_toggle))); // triac_1_state ^1
}
//
//void buildJavascript(){
//  javaScript="<SCRIPT>\n";
//  javaScript+="var xmlHttp=createXmlHttpObject();\n";
//
//  javaScript+="function createXmlHttpObject(){\n";
//  javaScript+=" if(window.XMLHttpRequest){\n";
//  javaScript+="    xmlHttp=new XMLHttpRequest();\n";
//  javaScript+=" }else{\n";
//  javaScript+="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
//  javaScript+=" }\n";
//  javaScript+=" return xmlHttp;\n";
//  javaScript+="}\n";
//
//  javaScript+="function process(){\n";
//  javaScript+=" if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
//  javaScript+="   xmlHttp.open('PUT','xml',true);\n";
//  javaScript+="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
//  javaScript+="   xmlHttp.send(null);\n";
//  javaScript+=" }\n";
//  javaScript+=" setTimeout('process()',1000);\n";
//  javaScript+="}\n";
// 
//  javaScript+="function handleServerResponse(){\n";
//  javaScript+=" if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
//  javaScript+="   xmlResponse=xmlHttp.responseXML;\n";
//  javaScript+="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
//  javaScript+="   message = xmldoc[0].firstChild.nodeValue;\n";
//  javaScript+="   document.getElementById('runtime').innerHTML=message;\n";
//  javaScript+=" }\n";
//  javaScript+="}\n";
//  javaScript+="</SCRIPT>\n";
//}


  

void buildXML(){
  XML="<?xml version='1.0'?>";
  XML="<Title>";
  XML+="<activepower>";
  XML+=hlw8012.getActivePower();
  XML+="</activepower>";
  XML+="<t1>";
  XML+=Status_triac_one();
  XML+="</t1>";
  XML+="<t2>";
  XML+=Status_triac_two();
  XML+="</t2>";
  XML+="<t3>";
  XML+=Status_triac_three();
  XML+="</t3>";
  XML+="<dim>";
  XML+=Status_triac_dim();
  XML+="</dim>";
  XML+="</Title>";
}

String millis2time(){
  String Time="";
  unsigned long ss;
  byte mm,hh;
  ss=millis()/1000;
  hh=ss/3600;
  mm=(ss-hh*3600)/60;
  ss=(ss-hh*3600)-mm*60;
  if(hh<10)Time+="0";
  Time+=(String)hh+":";
  if(mm<10)Time+="0";
  Time+=(String)mm+":";
  if(ss<10)Time+="0";
  Time+=(String)ss;
  return Time;
}

String Status_triac_one()
{
  String tstatus_1;
if(triac_1_state ==1)
{
  tstatus_1="ON";
}
else if(triac_1_state ==0)
{
  tstatus_1="OFF";
}
return tstatus_1;
}

String Status_triac_two()
{
  String tstatus_2;
if(triac_2_state ==1)
{
  tstatus_2="ON";
}
else if(triac_2_state ==0)
{
  tstatus_2="OFF";
}
return tstatus_2;
}

String Status_triac_three()
{
  String tstatus_3;
if(triac_3_state ==1)
{
  tstatus_3="ON";
}
else if(triac_3_state ==0)
{
  tstatus_3="OFF";
}
return tstatus_3;
}

String Status_triac_dim()
{
 return (String)dimmer_state;
}

void handleXML(){
  buildXML();
  server.send(200,"text/xml",XML);
}
