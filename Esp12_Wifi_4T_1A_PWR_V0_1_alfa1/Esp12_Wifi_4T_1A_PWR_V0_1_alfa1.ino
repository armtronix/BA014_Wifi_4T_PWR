/*
Code for Wifi three triac 2 amps  board
This code is for ESP8266
Firmware Version: 0.1
Hardware Version: 0.1
use Board as Node mcu 1.0

Code Edited By :Naren N Nayak
Date: 06/01/2018
Last Edited By: 
Date: 

 *  While a WiFi config is not set or can't connect:
 *    http://server_ip will give a config page with 
 *  While a WiFi config is set:
 *    http://server_ip/gpio -> Will display the GIPIO state and a switch form for it
 *    http://server_ip/gpio?state=0 -> Will change the GPIO directly and display the above aswell
 *    http://server_ip/cleareeprom -> Will reset the WiFi setting and rest to configure mode as AP
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected. (most likly it will be 192.168.4.1)
 * To force AP config mode, press button 20 Secs!
 *  For several snippets used, the credit goes to:
 *  - https://github.com/esp8266
 *  - https://github.com/chriscook8/esp-arduino-apboot
 *  - https://github.com/knolleary/pubsubclient
 *  - https://github.com/vicatcu/pubsubclient <- Currently this needs to be used instead of the origin
 *  - https://gist.github.com/igrr/7f7e7973366fc01d6393
 *  - http://www.esp8266.com/viewforum.php?f=25
 *  - http://www.esp8266.com/viewtopic.php?f=29&t=2745
 *  - And the whole Arduino and ESP8266 comunity
 */
//#define W60_POWER_VALUE
//#define W200_POWER_VALUE
//#define DEFAULT_POWER_VALUE
#define DEBUG
//#define WEBOTA
//debug added for information, change this according your needs

#ifdef DEBUG
  #define Debug(x)    Serial.print(x)
  #define Debugln(x)  Serial.println(x)
  #define Debugf(...) Serial.printf(__VA_ARGS__)
  #define Debugflush  Serial.flush
#else
  #define Debug(x)    {}
  #define Debugln(x)  {}
  #define Debugf(...) {}
  #define Debugflush  {}
#endif


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
//#include <EEPROM.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "HLW8012.h"

extern "C" {
  #include "user_interface.h" //Needed for the reset command
}

//***** Settings declare ********************************************************************************************************* 
String hostName ="Armtronix"; //The MQTT ID -> MAC adress will be added to make it kind of unique
int iotMode=0; //IOT mode: 0 = Web control, 1 = MQTT (No const since it can change during runtime)
//select GPIO's
#define INPIN 0  // input pin (push button)
#define RESTARTDELAY 3 //minimal time in sec for button press to reset
#define HUMANPRESSDELAY 50 // the delay in ms untill the press should be handled as a normal push by human. Button debounce. !!! Needs to be less than RESTARTDELAY & RESETDELAY!!!
#define RESETDELAY 20 //Minimal time in sec for button press to reset all settings and boot to config mode
#define RESET_PIN 16


/*HLW8012.h     GPIOS                      */
#define SEL_PIN                         5
#define CF1_PIN                         13
#define CF_PIN                          14


// Check values every 2 seconds
#define UPDATE_TIME                     2000
#define STATUS_UPDATE_TIME              200
// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where a
// the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define CURRENT_MODE                    HIGH

// These are the nominal values for the resistors in the circuit
#define CURRENT_RESISTOR                0.001
#define VOLTAGE_RESISTOR_UPSTREAM       ( 5 * 470000 ) // Real: 2280k
#define VOLTAGE_RESISTOR_DOWNSTREAM     ( 1000 ) // Real 1.009k

HLW8012 hlw8012;

//##### Object instances #####
int https_port_no= 80;  //added on 14/02/2019
MDNSResponder mdns;
ESP8266WebServer server(https_port_no);
WebSocketsServer webSocket = WebSocketsServer(81);  
WiFiClient wifiClient;
PubSubClient mqttClient;
Ticker btn_timer;
Ticker otaTickLoop;

//##### Flags ##### They are needed because the loop needs to continue and cant wait for long tasks!
int rstNeed=0;   // Restart needed to apply new settings
int toPub=0; // determine if state should be published.
int configToClear=0; // determine if config should be cleared.
int otaFlag=0;
boolean inApMode=0;
//##### Global vars ##### 
int webtypeGlob;
int otaCount=300; //imeout in sec for OTA mode
int current; //Current state of the button


unsigned long count = 0; //Button press time counter
String st; //WiFi Stations HTML list
String state; //State of light
char buf[40]; //For MQTT data recieve
char* host; //The DNS hostname
//To be read from Config file
String esid="";
String epass = "";
String pubTopic;
String subTopic;
String mqttServer = "";
String mqtt_user ="";      //added on 28/07/2018
String mqtt_passwd ="";    //added on 28/07/2018
String mqtt_will_msg =" disconnected";  //added on 28/07/2018
String mqtt_port= "";   //added on 14/02/2019
String Sptemp="";  //added on 14/02/2019
String Svtemp="";  //added on 14/02/2019
int Iptemp=60;   //load to be connected for calib //added on 14/02/2019
int Ivtemp=235;   //input voltage at the time of calib //added on 14/02/2019

const char* otaServerIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
String javaScript,XML;

/*Alexa event names */
String firstName;
char string[32];
char byteRead;
String serialReceived="";
String serialReceived_buf="";

int dimmer_state;
int triac_1_state;
int triac_2_state;
int triac_3_state;
int triac_1_state_toggle =0;
int triac_2_state_toggle =0;
int triac_3_state_toggle =0;

int old_dimmer_state;
int old_triac_1_state;
int old_triac_2_state;
int old_triac_3_state;

int mqtt_dimmer_state;
int mqtt_triac_1_state;
int mqtt_triac_2_state;
int mqtt_triac_3_state;

volatile boolean mqtt_dimpub =false;
volatile boolean mqtt_t1pub =false;
volatile boolean mqtt_t2pub =false;
volatile boolean mqtt_t3pub =false;
volatile boolean mqtt_powpub =false;

//-------------- void's -------------------------------------------------------------------------------------
void setup() 
{
  
  Serial.begin(115200);
  delay(100);
  WiFi.printDiag(Serial);
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, HIGH);
  //btn_timer.attach(0.05, btn_handle);
  
  Debugln("DEBUG: Entering loadConfig()");
  if (!SPIFFS.begin()) 
  {
    Serial.println("Failed to mount file system");
  }
  uint8_t mac[6];
  WiFi.macAddress(mac);
  hostName += "-";
  hostName += macToStr(mac);
  String hostTemp=hostName;
  hostTemp.replace(":","-");
  host = (char*) hostTemp.c_str();
  loadConfig();
  //loadConfigOld();
  Debugln("DEBUG: loadConfig() passed");
  // Connect to WiFi network
  Debugln("DEBUG: Entering initWiFi()");
  initWiFi();
  Debugln("DEBUG: initWiFi() passed");
  Debug("iotMode:");
  Debugln(iotMode);
  Debug("webtypeGlob:");
  Debugln(webtypeGlob);
  Debug("otaFlag:");
  Debugln(otaFlag);
  Debugln("DEBUG: Starting the main loop");

  //Initialize HLW8012
    // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
    // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
    // * currentWhen is the value in sel_pin to select current sampling
    // * set use_interrupts to false, we will have to call handle() in the main loop to do the sampling
    // * set pulse_timeout to 500ms for a fast response but losing precision (that's ~24W precision :( )
    hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, false, 2000000);//500000);

    // These values are used to calculate current, voltage and power factors as per datasheet formula
    // These are the nominal values for the Sonoff POW resistors:
    // * The CURRENT_RESISTOR is the 1milliOhm copper-manganese resistor in series with the main line
    // * The VOLTAGE_RESISTOR_UPSTREAM are the 5 470kOhm resistors in the voltage divider that feeds the V2P pin in the HLW8012
    // * The VOLTAGE_RESISTOR_DOWNSTREAM is the 1kOhm resistor in the voltage divider that feeds the V2P pin in the HLW8012

   /*[HLW] Default current multiplier : 14484.49
     [HLW] Default voltage multiplier : 408636.51
     [HLW] Default power multiplier   : 10343611.74 */

   /*    200W bulb
     [HLW] New current multiplier : 16405.11
     [HLW] New voltage multiplier : 132090.21
     [HLW] New power multiplier   : 59106352.77  */

   /*    60W bulb
     [HLW] New current multiplier : 14709.45
     [HLW] New voltage multiplier : 132090.21
     [HLW] New power multiplier   : 56419700.37  */


    hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);

    // Show default (as per datasheet) multipliers
    Serial.print("[HLW] Default current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
    Serial.print("[HLW] Default voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
    Serial.print("[HLW] Default power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());
    //Serial.println();

     // Iptemp= Sptemp.toInt();
     // Ivtemp= Svtemp.toInt();
    // calibrate(Iptemp,Ivtemp);
    // Serial.println("configured power");

  
}

void unblockingDelay(unsigned long mseconds) {
    unsigned long timeout = millis();
    while ((millis() - timeout) < mseconds) delay(1);
}
void calibrate(float load_power, float input_voltage) {
    float current_consumed = load_power/input_voltage;
    // Let's first read power, current and voltage
    // with an interval in between to allow the signal to stabilise:

    hlw8012.getActivePower();

    hlw8012.setMode(MODE_CURRENT);
    unblockingDelay(2000);
    hlw8012.getCurrent();

    hlw8012.setMode(MODE_VOLTAGE);
    unblockingDelay(2000);
    hlw8012.getVoltage();

    // Calibrate using a 60W bulb (pure resistive) on a 230V line
    hlw8012.expectedActivePower(load_power);
    hlw8012.expectedVoltage(input_voltage);
    hlw8012.expectedCurrent(current_consumed);

    // Show corrected factors
    Serial.print("[HLW] New current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
    Serial.print("[HLW] New voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
    Serial.print("[HLW] New power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());
    Serial.println();

}

void btn_handle()
{
  if(!digitalRead(INPIN)){
    ++count; // one count is 50ms
  } else {
    if (count > 1 && count < HUMANPRESSDELAY/5) { //push between 50 ms and 1 sec      
      Serial.print("button pressed "); 
      Serial.print(count*0.05); 
      Serial.println(" Sec."); 
      if(iotMode==1 && mqttClient.connected()){
        toPub=1;        
        Debugln("DEBUG: toPub set to 1");
      }
    } else if (count > (RESTARTDELAY/0.05) && count <= (RESETDELAY/0.05)){ //pressed 3 secs (60*0.05s)
      Serial.print("button pressed "); 
      Serial.print(count*0.05); 
      Serial.println(" Sec. Restarting!"); 
      setOtaFlag(!otaFlag);      
      ESP.reset();
    } else if (count > (RESETDELAY/0.05)){ //pressed 20 secs
      Serial.print("button pressed "); 
      Serial.print(count*0.05); 
      Serial.println(" Sec."); 
      Serial.println(" Clear settings and resetting!");       
      configToClear=1;
      }
    count=0; //reset since we are at high
  }
}



//-------------------------------- Main loop ---------------------------
void loop() {
  webSocket.loop();
  
    static unsigned long last = millis();

    // This UPDATE_TIME should be at least twice the minimum time for the current or voltage
    // signals to stabilize. Experimentally that's about 1 second.
    if ((millis() - last) > UPDATE_TIME) 
    {

        //last = millis();
        //Serial.print("[HLW] Active Power (W)    : "); Serial.println(hlw8012.getActivePower());
        //Serial.print("[HLW] Voltage (V)         : "); Serial.println(hlw8012.getVoltage());
        //Serial.print("[HLW] Current (A)         : "); Serial.println(hlw8012.getCurrent());
        //Serial.print("[HLW] Apparent Power (VA) : "); Serial.println(hlw8012.getApparentPower());
       // Serial.print("[HLW] Power Factor (%)    : "); Serial.println((int) (100 * hlw8012.getPowerFactor()));
        //Serial.println();

        // When not using interrupts we have to manually switch to current or voltage monitor
        // This means that every time we get into the conditional we only update one of them
        // while the other will return the cached value.
        hlw8012.toggleMode();
        
     }
     if ((millis() - last) > STATUS_UPDATE_TIME) 
     {
       Serial.println("status:"); 
      if(Serial.available())
      {
               size_t len = Serial.available();
               uint8_t sbuf[len];
               Serial.readBytes(sbuf, len); 
               serialReceived = (char*)sbuf;
               if(serialReceived.substring(0,2) == "D:")
               {
//               mqttClient.publish((char*)pubTopic.c_str(),serialReceived.substring(2,4).c_str());
               serialReceived_buf = serialReceived;
               serialReceived="";
               dimmer_state = serialReceived_buf.substring(2,4).toInt();
               triac_1_state = serialReceived_buf.substring(4,5).toInt(); //added on 15/02/19
               triac_2_state = serialReceived_buf.substring(5,6).toInt(); //added on 15/02/19
               triac_3_state = serialReceived_buf.substring(6,7).toInt(); //added on 15/02/19
                   if(dimmer_state != old_dimmer_state)
                   {
                    old_dimmer_state = dimmer_state;
                    mqtt_dimmer_state = dimmer_state;
                    mqtt_dimpub = true;
                    //Serial.println(dimmer_state);
                   }
                   if(triac_1_state != old_triac_1_state)
                   {
                    old_triac_1_state = triac_1_state;
                    mqtt_triac_1_state = triac_1_state;
                    mqtt_t1pub = true;
                   }
                   if(triac_2_state != old_triac_2_state)
                   {
                    old_triac_2_state = triac_2_state;
                    mqtt_triac_2_state = triac_2_state;
                    mqtt_t2pub = true;
                   }
                   if(triac_3_state != old_triac_3_state)
                   {
                    old_triac_3_state = triac_3_state;
                    mqtt_triac_3_state = triac_3_state;
                    mqtt_t3pub = true;
                   }
               serialReceived_buf="";
               }  
      } 
       last = millis();
     }


if (mqttClient.connected()) 
{
if(mqtt_powpub == true)
{
   String temp_pow ="PIS"+(String)hlw8012.getActivePower();
   Serial.println(temp_pow);
   mqttClient.publish((char*)pubTopic.c_str(),(char*)temp_pow.c_str());
   mqtt_handler();
   mqtt_powpub = false;
   
}
if(mqtt_t1pub == true)
  {
    String temp_t1 ="R1IS"+Status_triac_one();
   mqttClient.publish((char*)pubTopic.c_str(),(char*)temp_t1.c_str());
   mqtt_handler();
   mqtt_t1pub = false;
  }

if(mqtt_t2pub == true)
  {
    String temp_t2 ="R2IS"+Status_triac_two();
   mqttClient.publish((char*)pubTopic.c_str(),(char*)temp_t2.c_str());
   mqtt_handler();
   mqtt_t2pub = false;
  }

if(mqtt_t3pub == true)
  {
    String temp_t3 ="R3IS"+Status_triac_three();
   mqttClient.publish((char*)pubTopic.c_str(),(char*)temp_t3.c_str());
   mqtt_handler();
   mqtt_t3pub = false;
  }

  
if(mqtt_dimmer_state == 0 && mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS0");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >0 && mqtt_dimmer_state <= 5 )&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS5");
   mqtt_handler();
   mqtt_dimpub = false; 
  }
else if((mqtt_dimmer_state >5 && mqtt_dimmer_state <= 10) && mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS10");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >10 && mqtt_dimmer_state <= 15) && mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS15");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >15 && mqtt_dimmer_state <= 20) && mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS20");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >20 && mqtt_dimmer_state <= 25)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS25");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >25 && mqtt_dimmer_state <= 30)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS30");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >30 && mqtt_dimmer_state <= 35)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS35");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >35 && mqtt_dimmer_state <= 40)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS40");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >40 && mqtt_dimmer_state <= 45)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS45");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >45 && mqtt_dimmer_state <= 50)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS50");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >50 && mqtt_dimmer_state <= 55)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS55");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >55 && mqtt_dimmer_state <= 60)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS60");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >60 && mqtt_dimmer_state <= 65)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS65");
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >65 && mqtt_dimmer_state <= 70)&& mqtt_dimpub == true)
 {
  mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS70");
  mqtt_handler();
  mqtt_dimpub = false;
 }
else if((mqtt_dimmer_state >70 && mqtt_dimmer_state <= 75)&& mqtt_dimpub == true)
 {
  mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS75");
  mqtt_handler();
  mqtt_dimpub = false;
 }
else if((mqtt_dimmer_state >75 && mqtt_dimmer_state <= 80)&& mqtt_dimpub == true)
 {
  mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS80");
  mqtt_handler();
  mqtt_dimpub = false;
 }
else if((mqtt_dimmer_state >80 && mqtt_dimmer_state <= 85)&& mqtt_dimpub == true)
 {
  mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS85");
  mqtt_handler();
  mqtt_dimpub = false;
 }
else if((mqtt_dimmer_state >85 && mqtt_dimmer_state <= 90)&& mqtt_dimpub == true)
   {                      
    mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS90");    
    mqtt_handler();                    
    mqtt_dimpub = false;                        
   }
else if((mqtt_dimmer_state >90 && mqtt_dimmer_state <= 95)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS95"); 
   mqtt_handler();
   mqtt_dimpub = false;
  }
else if((mqtt_dimmer_state >95 && mqtt_dimmer_state <= 100)&& mqtt_dimpub == true)
  {
   mqttClient.publish((char*)pubTopic.c_str(),"DimmerIS99");
   mqtt_handler();
   mqtt_dimpub = false;        
  } 
}



  //Debugln("DEBUG: loop() begin");
  if(configToClear==1){
    //Debugln("DEBUG: loop() clear config flag set!");
    clearConfig()? Serial.println("Config cleared!") : Serial.println("Config could not be cleared");
    delay(1000);
    ESP.reset();
  }
  //Debugln("DEBUG: config reset check passed");  
  if (WiFi.status() == WL_CONNECTED && otaFlag){
    if(otaCount<=1) {
      Serial.println("OTA mode time out. Reset!"); 
      setOtaFlag(0);
      ESP.reset();
      delay(100);
    }
    server.handleClient();
    delay(1);
  } else if (WiFi.status() == WL_CONNECTED || webtypeGlob == 1){
    //Debugln("DEBUG: loop() wifi connected & webServer ");
    if (iotMode==0 || webtypeGlob == 1){
      //Debugln("DEBUG: loop() Web mode requesthandling ");
      server.handleClient();
      delay(1);
      if(esid != "" && WiFi.status() != WL_CONNECTED) //wifi reconnect part
      {
        Scan_Wifi_Networks();
      }
    } else if (iotMode==1 && webtypeGlob != 1 && otaFlag !=1){
          //Debugln("DEBUG: loop() MQTT mode requesthandling ");
          if (!connectMQTT()){
              delay(200);          
          }                    
          if (mqttClient.connected()){            
              //Debugln("mqtt handler");
              mqtt_handler();
          } else {
              Debugln("mqtt Not connected!");
          }
    }
  } else{
    Debugln("DEBUG: loop - WiFi not connected");  
    delay(1000);
    initWiFi(); //Try to connect again
  }
    //Debugln("DEBUG: loop() end");
}
