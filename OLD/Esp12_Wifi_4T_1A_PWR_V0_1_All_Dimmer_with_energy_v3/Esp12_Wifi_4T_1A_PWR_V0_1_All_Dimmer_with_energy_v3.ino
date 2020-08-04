/*
  Code for Wifi Four triac 1 amps  board
  This code is for ESP8266
  Firmware Version: 0.3
  Hardware Version: 0.1
  use Board as Node mcu 1.0

  Code Edited By :Naren N Nayak
  Date: 06/01/2018
  Last Edited By: Karthik S B
  Date: 1/04/19

    While a WiFi config is not set or can't connect:
      http://server_ip will give a config page with
    While a WiFi config is set:
      http://server_ip/gpio -> Will display the GIPIO state and a switch form for it
      http://server_ip/gpio?state=0 -> Will change the GPIO directly and display the above aswell
      http://server_ip/cleareeprom -> Will reset the WiFi setting and rest to configure mode as AP
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected. (most likly it will be 192.168.4.1)
   To force AP config mode, press button 20 Secs!
    For several snippets used, the credit goes to:
    - https://github.com/esp8266
    - https://github.com/chriscook8/esp-arduino-apboot
    - https://github.com/knolleary/pubsubclient
    - https://github.com/vicatcu/pubsubclient <- Currently this needs to be used instead of the origin
    - https://gist.github.com/igrr/7f7e7973366fc01d6393
    - http://www.esp8266.com/viewforum.php?f=25
    - http://www.esp8266.com/viewtopic.php?f=29&t=2745
    - And the whole Arduino and ESP8266 comunity


    // J1 header Programming pins


  VCC 5V . . GND
  TXDE   . . RXDA
  RXDE   . . TXDA
  DTRE   . . DTRA
  RTSE   . . GND


*/
//#define W60_POWER_VALUE
//#define W200_POWER_VALUE
//#define DEFAULT_POWER_VALUE
#define DEBUG
#define ALEXAEN
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

#ifdef ALEXAEN
#include <Espalexa.h>
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

#ifdef ALEXAEN
void firstLightChanged(uint8_t brightness);
void secondLightChanged(uint8_t brightness);
void thirdLightChanged(uint8_t brightness);
void fourthLightChanged(uint8_t brightness);
#endif

//***** Settings declare *********************************************************************************************************
String hostName = "Armtronix"; //The MQTT ID -> MAC adress will be added to make it kind of unique
int iotMode = 0; //IOT mode: 0 = Web control, 1 = MQTT (No const since it can change during runtime)
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
#define STATUS_UPDATE_TIME              5000
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
int https_port_no = 80; //added on 14/02/2019
MDNSResponder mdns;
ESP8266WebServer server(https_port_no);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClient wifiClient;
PubSubClient mqttClient;
Ticker btn_timer;
Ticker otaTickLoop;
#ifdef ALEXAEN
Espalexa espalexa;
#endif
//##### Flags ##### They are needed because the loop needs to continue and cant wait for long tasks!
int rstNeed = 0; // Restart needed to apply new settings
int toPub = 0; // determine if state should be published.
int configToClear = 0; // determine if config should be cleared.
int otaFlag = 0;
boolean inApMode = 0;
//##### Global vars #####
int webtypeGlob;
int otaCount = 300; //imeout in sec for OTA mode
int current; //Current state of the button
#ifdef ALEXAEN
boolean wifiConnected = false;
#endif

unsigned long count = 0; //Button press time counter
String st; //WiFi Stations HTML list
String state; //State of light
char buf[40]; //For MQTT data recieve
char* host; //The DNS hostname
//To be read from Config file
String esid = "";
String epass = "";
String pubTopic;
String subTopic;
String mqttServer = "";
String mqtt_user = "";     //added on 28/07/2018
String mqtt_passwd = "";   //added on 28/07/2018
String mqtt_will_msg = " disconnected"; //added on 28/07/2018
String mqtt_port = "";  //added on 14/02/2019

float P,V;
String Values_HLW8021;
String Values_HLW8021_ActivePower;
String Values_HLW8021_Voltage;
String Values_HLW8021_Current;
String Values_HLW8021_ApparentPower;
String Values_HLW8021_Powerfactor;
String Values_HLW8021_Energy;
double setmulPower=0;
double setmulCurrent=0;
double setmulVoltage=0;
int calflag =0;
String pubTopic_HLW8021;
String Sptemp = ""; //added on 14/02/2019
String Svtemp = ""; //added on 14/02/2019
//int Iptemp = 60; //load to be connected for calib //added on 14/02/2019
//int Ivtemp = 235; //input voltage at the time of calib //added on 14/02/2019
int Updated_power;

const char* otaServerIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
String javaScript, XML;

/*Alexa event names */
String firstName;
String secondName;
String thirdName;
String fourthName;
char string[32];
char byteRead;
String serialReceived = "";
String serialReceived_buf = "";

int dimmer_state1;
int dimmer_state2;
int dimmer_state3;
int dimmer_state4;
int triac_1_state_toggle = 0;
int triac_2_state_toggle = 0;
int triac_3_state_toggle = 0;

int old_dimmer_state1;
int old_dimmer_state2;
int old_dimmer_state3;
int old_dimmer_state4;
int mqtt_dimmer_state1;
int mqtt_dimmer_state2;
int mqtt_dimmer_state3;
int mqtt_dimmer_state4;

volatile boolean mqtt_dimpub1 = false;
volatile boolean mqtt_dimpub2 = false;
volatile boolean mqtt_dimpub3 = false;
volatile boolean mqtt_dimpub4 = false;
volatile boolean mqtt_powpub = false;

//-------------- void's -------------------------------------------------------------------------------------


// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void ICACHE_RAM_ATTR hlw8012_cf1_interrupt() {
  hlw8012.cf1_interrupt();
}
void ICACHE_RAM_ATTR hlw8012_cf_interrupt() {
  hlw8012.cf_interrupt();
}

// Library expects an interrupt on both edges
void setInterrupts() {
  attachInterrupt(CF1_PIN, hlw8012_cf1_interrupt, CHANGE);
  attachInterrupt(CF_PIN, hlw8012_cf_interrupt, CHANGE);
}

void calibrate(float power_value , float voltage_value) {
 /*
  delay(2000);
  Serial.println("Dimmer1:99");
  delay(100);
  Serial.println("Dimmer2:99");
  delay(100);
  Serial.println("Dimmer3:99");
  delay(100);
  Serial.println("Dimmer4:99");
  */
  Serial.print("Power = ");
  Serial.println(power_value);
  Serial.print("Voltage = ");
  Serial.println(voltage_value);
  

  // Let some time to register values
  unsigned long timeout = millis();
  while ((millis() - timeout) < 10000) {
    delay(1);
  }

  // Calibrate using a 60W bulb (pure resistive) on a 230V line
    setmulPower=hlw8012.expectedActivePower(power_value);
    setmulVoltage=hlw8012.expectedVoltage(voltage_value);
    setmulCurrent=hlw8012.expectedCurrent(power_value / voltage_value);
    
    hlw8012.setCurrentMultiplier(setmulCurrent);
    hlw8012.setVoltageMultiplier(setmulVoltage);
    hlw8012.setPowerMultiplier(setmulPower);

  // Show corrected factors
  Serial.print("[HLW] New current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
  Serial.print("[HLW] New voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
  Serial.print("[HLW] New power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());


  delay(100);
  Serial.println("Dimmer1:0");
  delay(100);
  Serial.println("Dimmer2:0");
  delay(100);
  Serial.println("Dimmer3:0");
  delay(100);
  Serial.println("Dimmer4:0");

}


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
  String hostTemp = hostName;
  hostTemp.replace(":", "-");
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


  // Initialize HLW8012
  // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
  // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
  // * currentWhen is the value in sel_pin to select current sampling
  // * set use_interrupts to true to use interrupts to monitor pulse widths
  // * leave pulse_timeout to the default value, recommended when using interrupts
  hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);

  // These values are used to calculate current, voltage and power factors as per datasheet formula
  // These are the nominal values for the Sonoff POW resistors:
  // * The CURRENT_RESISTOR is the 1milliOhm copper-manganese resistor in series with the main line
  // * The VOLTAGE_RESISTOR_UPSTREAM are the 5 470kOhm resistors in the voltage divider that feeds the V2P pin in the HLW8012
  // * The VOLTAGE_RESISTOR_DOWNSTREAM is the 1kOhm resistor in the voltage divider that feeds the V2P pin in the HLW8012
  hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
  hlw8012.setCurrentMultiplier(setmulCurrent);
  hlw8012.setVoltageMultiplier(setmulVoltage);
  hlw8012.setPowerMultiplier(setmulPower);

  // Show default (as per datasheet) multipliers
  Serial.print("[HLW] Default current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
  Serial.print("[HLW] Default voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
  Serial.print("[HLW] Default power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());
  Serial.println();

  setInterrupts();

#ifdef ALEXAEN
  //Alexa Part
  if (wifiConnected) {
    server.on("/", HTTP_GET, []() {
      server.send(200, "text/plain", "This is an example index page your server may send.");
    });
    server.on("/test", HTTP_GET, []() {
      server.send(200, "text/plain", "This is a second subpage you may have.");
    });
    server.onNotFound([]() {
      if (!espalexa.handleAlexaApiCall(server.uri(), server.arg(0))) //if you don't know the URI, ask espalexa whether it is an Alexa control request
      {
        //whatever you want to do with 404s
        server.send(404, "text/plain", "Not found");
      }
    });
  }
  // Define your devices here.
  espalexa.addDevice((char*)firstName.c_str(), firstLightChanged); //simplest definition, default state off
  espalexa.addDevice((char*)secondName.c_str(), secondLightChanged); //simplest definition, default state off
  espalexa.addDevice((char*)thirdName.c_str(), thirdLightChanged); //simplest definition, default state off
  espalexa.addDevice((char*)fourthName.c_str(), fourthLightChanged); //simplest definition, default state off

  espalexa.begin(&server); //give espalexa a pointer to your server object so it can use your server instead of creating its own
#endif



 // calibrate(P,V);



}

//void unblockingDelay(unsigned long mseconds) {
//    unsigned long timeout = millis();
//    while ((millis() - timeout) < mseconds) delay(1);
//}

void btn_handle()
{
  if (!digitalRead(INPIN)) {
    ++count; // one count is 50ms
  } else {
    if (count > 1 && count < HUMANPRESSDELAY / 5) { //push between 50 ms and 1 sec
      Serial.print("button pressed ");
      Serial.print(count * 0.05);
      Serial.println(" Sec.");
      if (iotMode == 1 && mqttClient.connected()) {
        toPub = 1;
        Debugln("DEBUG: toPub set to 1");
      }
    } else if (count > (RESTARTDELAY / 0.05) && count <= (RESETDELAY / 0.05)) { //pressed 3 secs (60*0.05s)
      Serial.print("button pressed ");
      Serial.print(count * 0.05);
      Serial.println(" Sec. Restarting!");
      setOtaFlag(!otaFlag);
      ESP.reset();
    } else if (count > (RESETDELAY / 0.05)) { //pressed 20 secs
      Serial.print("button pressed ");
      Serial.print(count * 0.05);
      Serial.println(" Sec.");
      Serial.println(" Clear settings and resetting!");
      configToClear = 1;
    }
    count = 0; //reset since we are at high
  }
}



//-------------------------------- Main loop ---------------------------
void loop() {
  webSocket.loop();

  static unsigned long last = millis();

  //    // This UPDATE_TIME should be at least twice the minimum time for the current or voltage
  //    // signals to stabilize. Experimentally that's about 1 second.
  //    if ((millis() - last) > UPDATE_TIME)
  //    {
  //
  //      //  last = millis();
  //      //  Updated_power = hlw8012.getActivePower();
  ////        Serial.print("[HLW] Active Power (W)    : "); Serial.println(Updated_power);
  ////        Serial.print("[HLW] Voltage (V)         : "); Serial.println(hlw8012.getVoltage());
  ////        Serial.print("[HLW] Current (A)         : "); Serial.println(hlw8012.getCurrent());
  ////        Serial.print("[HLW] Apparent Power (VA) : "); Serial.println(hlw8012.getApparentPower());
  ////        Serial.print("[HLW] Power Factor (%)    : "); Serial.println((int) (100 * hlw8012.getPowerFactor()));
  ////        Serial.println();
  //
  //     }
 if ((millis() - last) > STATUS_UPDATE_TIME)
  {
    Serial.println("status:");
    Updated_power = hlw8012.getActivePower();
    if (Serial.available())
    {
      size_t len = Serial.available();
      uint8_t sbuf[len];
      Serial.readBytes(sbuf, len);
      serialReceived = (char*)sbuf;
      if (serialReceived.substring(0, 2) == "D:")
      {
        //               mqttClient.publish((char*)pubTopic.c_str(),serialReceived.substring(2,4).c_str());
        serialReceived_buf = serialReceived;
        serialReceived = "";
        dimmer_state1 = serialReceived_buf.substring(2, 4).toInt();
        dimmer_state2 = serialReceived_buf.substring(5, 7).toInt(); //added on 15/02/19
        dimmer_state3 = serialReceived_buf.substring(8, 10).toInt(); //added on 15/02/19
        dimmer_state4 = serialReceived_buf.substring(11, 13).toInt(); //added on 15/02/19
        if (dimmer_state1 != old_dimmer_state1)
        {
          old_dimmer_state1 = dimmer_state1;
          mqtt_dimmer_state1 = dimmer_state1;
          mqtt_dimpub1 = true;
          //Serial.println(dimmer_state1);
        }
        if (dimmer_state2 != old_dimmer_state2)
        {
          old_dimmer_state2 = dimmer_state2;
          mqtt_dimmer_state2 = dimmer_state2;
          mqtt_dimpub2 = true;
          //Serial.println(dimmer_state2);
        }
        if (dimmer_state3 != old_dimmer_state3)
        {
          old_dimmer_state3 = dimmer_state3;
          mqtt_dimmer_state3 = dimmer_state3;
          mqtt_dimpub3 = true;
          //Serial.println(dimmer_state3);
        }
        if (dimmer_state4 != old_dimmer_state4)
        {
          old_dimmer_state4 = dimmer_state4;
          mqtt_dimmer_state4 = dimmer_state4;
          mqtt_dimpub4 = true;
          //Serial.println(dimmer_state4);
        }

        serialReceived_buf = "";
      }
    }
    last = millis();
  }
 if ((millis() - last) > UPDATE_TIME) 
    {

        last = millis();
        
        Values_HLW8021_ActivePower = String(hlw8012.getActivePower());
        Values_HLW8021_Voltage = String(hlw8012.getVoltage());
        Values_HLW8021_Current = String(hlw8012.getCurrent());
       // Values_HLW8021_ApparentPower = String(hlw8012.getApparentPower());
      //  Values_HLW8021_Powerfactor = String((int) (100 * hlw8012.getPowerFactor()));
      //  Values_HLW8021_Energy = String(hlw8012.getEnergy());
        
        Serial.print("[HLW] Active Power (W)    : "); Serial.println(Values_HLW8021_ActivePower);
        Serial.print("Power = ");
        Serial.println(P);
        Serial.print("Voltage = ");
        Serial.println(V);
        Serial.println();
        
        Values_HLW8021= "P:"+Values_HLW8021_ActivePower+"W"+", V:"+Values_HLW8021_Voltage+"V";
        pubTopic_HLW8021=pubTopic+"/HLW8021";
         if (mqttClient.connected()) 
         {
         mqttClient.publish((char*)pubTopic_HLW8021.c_str(), (char*)Values_HLW8021.c_str());
         mqttClient.loop();
         }
    }
    if(calflag ==1)
   { 
    calibrate(P,V);
    calflag =0; 
    saveConfig()? Serial.println("sucessfully.") : Serial.println("not succesfully!");;
    ESP.wdtFeed();
    delay(100);
    WiFi.forceSleepBegin(); wdt_reset(); 
    ESP.restart(); 
    while(1)wdt_reset();
   }

  if (mqttClient.connected())
  {
    if (mqtt_powpub == true)
    {
      String temp_pow = "PIS" + (String)Values_HLW8021_ActivePower;
      Serial.println(temp_pow);
      mqttClient.publish((char*)pubTopic.c_str(), (char*)temp_pow.c_str());
      mqtt_handler();
      mqtt_powpub = false;

    }


    if (mqtt_dimmer_state1 == 0 && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS0");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 0 && mqtt_dimmer_state1 <= 5 ) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS5");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 5 && mqtt_dimmer_state1 <= 10) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS10");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 10 && mqtt_dimmer_state1 <= 15) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS15");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 15 && mqtt_dimmer_state1 <= 20) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS20");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 20 && mqtt_dimmer_state1 <= 25) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS25");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 25 && mqtt_dimmer_state1 <= 30) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS30");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 30 && mqtt_dimmer_state1 <= 35) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS35");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 35 && mqtt_dimmer_state1 <= 40) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS40");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 40 && mqtt_dimmer_state1 <= 45) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS45");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 45 && mqtt_dimmer_state1 <= 50) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS50");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 50 && mqtt_dimmer_state1 <= 55) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS55");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 55 && mqtt_dimmer_state1 <= 60) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS60");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 60 && mqtt_dimmer_state1 <= 65) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS65");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 65 && mqtt_dimmer_state1 <= 70) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS70");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 70 && mqtt_dimmer_state1 <= 75) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS75");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 75 && mqtt_dimmer_state1 <= 80) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS80");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 80 && mqtt_dimmer_state1 <= 85) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS85");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 85 && mqtt_dimmer_state1 <= 90) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS90");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 90 && mqtt_dimmer_state1 <= 95) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS95");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }
    else if ((mqtt_dimmer_state1 > 95 && mqtt_dimmer_state1 <= 100) && mqtt_dimpub1 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer1IS99");
      mqtt_handler();
      mqtt_dimpub1 = false;
    }

    if (mqtt_dimmer_state2 == 0 && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS0");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 0 && mqtt_dimmer_state2 <= 5 ) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS5");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 5 && mqtt_dimmer_state2 <= 10) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS10");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 10 && mqtt_dimmer_state2 <= 15) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS15");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 15 && mqtt_dimmer_state2 <= 20) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS20");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 20 && mqtt_dimmer_state2 <= 25) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS25");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 25 && mqtt_dimmer_state2 <= 30) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS30");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 30 && mqtt_dimmer_state2 <= 35) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS35");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 35 && mqtt_dimmer_state2 <= 40) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS40");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 40 && mqtt_dimmer_state2 <= 45) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS45");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 45 && mqtt_dimmer_state2 <= 50) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS50");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 50 && mqtt_dimmer_state2 <= 55) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS55");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 55 && mqtt_dimmer_state2 <= 60) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS60");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 60 && mqtt_dimmer_state2 <= 65) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS65");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 65 && mqtt_dimmer_state2 <= 70) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS70");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 70 && mqtt_dimmer_state2 <= 75) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS75");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 75 && mqtt_dimmer_state2 <= 80) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS80");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 80 && mqtt_dimmer_state2 <= 85) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS85");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 85 && mqtt_dimmer_state2 <= 90) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS90");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 90 && mqtt_dimmer_state2 <= 95) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS95");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }
    else if ((mqtt_dimmer_state2 > 95 && mqtt_dimmer_state2 <= 100) && mqtt_dimpub2 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer2IS99");
      mqtt_handler();
      mqtt_dimpub2 = false;
    }


    if (mqtt_dimmer_state3 == 0 && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS0");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 0 && mqtt_dimmer_state3 <= 5 ) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS5");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 5 && mqtt_dimmer_state3 <= 10) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS10");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 10 && mqtt_dimmer_state3 <= 15) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS15");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 15 && mqtt_dimmer_state3 <= 20) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS20");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 20 && mqtt_dimmer_state3 <= 25) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS25");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 25 && mqtt_dimmer_state3 <= 30) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS30");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 30 && mqtt_dimmer_state3 <= 35) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS35");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 35 && mqtt_dimmer_state3 <= 40) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS40");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 40 && mqtt_dimmer_state3 <= 45) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS45");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 45 && mqtt_dimmer_state3 <= 50) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS50");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 50 && mqtt_dimmer_state3 <= 55) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS55");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 55 && mqtt_dimmer_state3 <= 60) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS60");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 60 && mqtt_dimmer_state3 <= 65) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS65");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 65 && mqtt_dimmer_state3 <= 70) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS70");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 70 && mqtt_dimmer_state3 <= 75) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS75");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 75 && mqtt_dimmer_state3 <= 80) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS80");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 80 && mqtt_dimmer_state3 <= 85) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS85");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 85 && mqtt_dimmer_state3 <= 90) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS90");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 90 && mqtt_dimmer_state3 <= 95) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS95");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }
    else if ((mqtt_dimmer_state3 > 95 && mqtt_dimmer_state3 <= 100) && mqtt_dimpub3 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer3IS99");
      mqtt_handler();
      mqtt_dimpub3 = false;
    }



    if (mqtt_dimmer_state4 == 0 && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS0");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 0 && mqtt_dimmer_state4 <= 5 ) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS5");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 5 && mqtt_dimmer_state4 <= 10) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS10");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 10 && mqtt_dimmer_state4 <= 15) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS15");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 15 && mqtt_dimmer_state4 <= 20) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS20");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 20 && mqtt_dimmer_state4 <= 25) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS25");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 25 && mqtt_dimmer_state4 <= 30) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS30");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 30 && mqtt_dimmer_state4 <= 35) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS35");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 35 && mqtt_dimmer_state4 <= 40) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS40");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 40 && mqtt_dimmer_state4 <= 45) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS45");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 45 && mqtt_dimmer_state4 <= 50) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS50");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 50 && mqtt_dimmer_state4 <= 55) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS55");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 55 && mqtt_dimmer_state4 <= 60) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS60");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 60 && mqtt_dimmer_state4 <= 65) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS65");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 65 && mqtt_dimmer_state4 <= 70) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS70");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 70 && mqtt_dimmer_state4 <= 75) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS75");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 75 && mqtt_dimmer_state4 <= 80) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS80");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 80 && mqtt_dimmer_state4 <= 85) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS85");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 85 && mqtt_dimmer_state4 <= 90) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS90");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 90 && mqtt_dimmer_state4 <= 95) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS95");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
    else if ((mqtt_dimmer_state4 > 95 && mqtt_dimmer_state4 <= 100) && mqtt_dimpub4 == true)
    {
      mqttClient.publish((char*)pubTopic.c_str(), "Dimmer4IS99");
      mqtt_handler();
      mqtt_dimpub4 = false;
    }
  }

  //Debugln("DEBUG: loop() begin");
  if (configToClear == 1) {
    //Debugln("DEBUG: loop() clear config flag set!");
    clearConfig() ? Serial.println("Config cleared!") : Serial.println("Config could not be cleared");
    delay(1000);
    ESP.reset();
  }
  //Debugln("DEBUG: config reset check passed");
  if (WiFi.status() == WL_CONNECTED && otaFlag) {
    if (otaCount <= 1) {
      Serial.println("OTA mode time out. Reset!");
      setOtaFlag(0);
      ESP.reset();
      delay(100);
    }
    server.handleClient();
    delay(1);
  } else if (WiFi.status() == WL_CONNECTED || webtypeGlob == 1) {
    //Debugln("DEBUG: loop() wifi connected & webServer ");
    if (iotMode == 0 || webtypeGlob == 1) {
      //Debugln("DEBUG: loop() Web mode requesthandling ");
      server.handleClient();
      delay(1);
      if (esid != "" && WiFi.status() != WL_CONNECTED) //wifi reconnect part
      {
        Scan_Wifi_Networks();
      }
    } else if (iotMode == 1 && webtypeGlob != 1 && otaFlag != 1) {
      //Debugln("DEBUG: loop() MQTT mode requesthandling ");
      if (!connectMQTT()) {
        delay(200);
      }
      if (mqttClient.connected()) {
        //Debugln("mqtt handler");
        mqtt_handler();
      } else {
        Debugln("mqtt Not connected!");
      }
    }
  } else {
    Debugln("DEBUG: loop - WiFi not connected");
    delay(1000);
    initWiFi(); //Try to connect again
  }

#ifdef ALEXAEN
  espalexa.loop();
#endif
  delay(1);
  //Debugln("DEBUG: loop() end");
}



#ifdef ALEXAEN
//our callback functions
void firstLightChanged(uint8_t brightness)
{
  //do what you need to do here

  //EXAMPLE
  if (brightness == 255) {
    Serial.println("Dimmer1:99");
  }
  else if (brightness == 0) {
    Serial.println("Dimmer1:0");
  }
  else if (brightness == 1) {
    Serial.println("Dimmer1:0");
  }
  else {
    float mul = 0.388; //  99/255 for values between 0-99
    float bness = (brightness * mul);
    int ss = bness;
    Serial.print("Dimmer1:");
    Serial.println(ss);
  }

}

/*##################################Callback functions for Second dimmer #########################*/
void secondLightChanged(uint8_t brightness)
{

//do what you need to do here

    //EXAMPLE
    if (brightness == 255) {
      Serial.println("Dimmer2:99");
    }
    else if (brightness == 0) {
      Serial.println("Dimmer2:0");
    }
    else if (brightness == 1) {
      Serial.println("Dimmer2:0");
    }
    else {
      float mul=0.388;//  99/255 for values between 0-99
      float bness=(brightness*mul);
      int ss=bness;
      Serial.print("Dimmer2:"); 
      Serial.println(ss);
    }

}

/*##################################Callback functions for Third dimmer #########################*/
void thirdLightChanged(uint8_t brightness)
{

//do what you need to do here

    //EXAMPLE
    if (brightness == 255) {
      Serial.println("Dimmer3:99");
    }
    else if (brightness == 0) {
      Serial.println("Dimmer3:0");
    }
    else if (brightness == 1) {
      Serial.println("Dimmer3:0");
    }
    else {
      float mul=0.388;//  99/255 for values between 0-99
      float bness=(brightness*mul);
      int ss=bness;
      Serial.print("Dimmer3:"); 
      Serial.println(ss);
    }

}

/*##################################Callback functions for Fourth dimmer #########################*/
void fourthLightChanged(uint8_t brightness)
{

//do what you need to do here

    //EXAMPLE
    if (brightness == 255) {
      Serial.println("Dimmer4:99");
    }
    else if (brightness == 0) {
      Serial.println("Dimmer4:0");
    }
    else if (brightness == 1) {
      Serial.println("Dimmer4:0");
    }
    else {
      float mul=0.388;//  99/255 for values between 0-99
      float bness=(brightness*mul);
      int ss=bness;
      Serial.print("Dimmer4:"); 
      Serial.println(ss);
    }

}
#endif
