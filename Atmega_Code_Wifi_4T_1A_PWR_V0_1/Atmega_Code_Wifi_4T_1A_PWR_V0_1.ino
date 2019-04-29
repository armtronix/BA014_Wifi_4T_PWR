/*Code for Wifi 4T_1A_PWR board
The Board has one Triacs and it is dimmable 

This code is for Atmega328p 
Firmware Version: 0.1
Hardware Version: 0.1

Code Edited By :Naren
Date: 11/02/19  //Description  modified the frequency step to 85 and dimming width to 100
Last Edited By:Naren N Nayak
Date: 11/02/2019

*/ 
#include  <TimerOne.h>
#define version_no "FVer 0.1 ,HVer 0.1" 

/*one line function*/
#define toggle(x)  digitalWrite(x, (digitalRead(x)^1)) ;
#define toggle_with_delay(x,y)  digitalWrite(x, (digitalRead(x)^1)) ; delay(y);
/*Dimmer pin no.*/
#define DIMMER_1 6 //Arduino Pin 6   

/*Non Dimmer pin no.  */ 
#define TRIAC_1 8 //Arduino Pin 8
#define TRIAC_2 9 //Arduino Pin 9
#define TRIAC_3 10 //Arduino Pin 10

/*LED pin no.  */ 
#define LED_1 4 //Arduino Pin 4
#define LED_2 3 //Arduino Pin 3

/*manual switch */
#define V_Switch_1 A0 //switch //Arduino Pin A0
#define V_Switch_2 A1 //switch //Arduino Pin A1 
#define V_Switch_3 A2 //switch //Arduino Pin A2 
#define V_Switch_4 A3 //switch //Arduino Pin A3 

/* ZCD */

#define ZCD_INT 0  //Arduino GPIO2 
#define Dimmer_width 100 //100

int dimming = 100;  //100
int freqStep = 85;//85,82,80 //75*5 as prescalar is 16 for 80MHZ //77 for the led

/*Serial Data variables*/
String serialReceived;
String serialReceived1;
String serialReceived2;
String serialReceived3;
String Dimmer_value_temp;
String Dimmer_value;
String regulator_value_temp;
/*POT Variable */
String regulator_value;

/*ZCD Variables */

volatile int dim_value = 0;
volatile boolean zero_cross = 0;
volatile float int_regulator;
/*Flags for Dimmer virtual switch concept */
volatile boolean dimmer_value_changed =false; 
volatile boolean regulator_value_changed =false;
volatile boolean dimmer_status =false;
volatile boolean mqttconnected =false;
int dimvalue;

void setup() 
{
  delay(100);
  Serial.begin(115200);
  Serial.println("WiFi-4T channel 1A PWR board");
  Serial.println(version_no);
  
  pinMode(DIMMER_1, OUTPUT); //Dimmer_1 output
  pinMode(TRIAC_1, OUTPUT);  //Triac_1 output
  pinMode(TRIAC_2, OUTPUT);  //Triac_2 output
  pinMode(TRIAC_3, OUTPUT);  //Triac_3 output
  pinMode(LED_1, OUTPUT);  //LED_1 output
  pinMode(LED_2, OUTPUT);  //LED_2 output
  
  pinMode(V_Switch_1,INPUT);  //Triac_1 V_Switch input
  pinMode(V_Switch_2,INPUT);  //Triac_2 V_Switch input
  pinMode(V_Switch_3,INPUT);  //Triac_3 V_Switch input
  pinMode(V_Switch_4,INPUT);  //Dimmer_1 V_Switch input
  
  attachInterrupt(ZCD_INT, zero_cross_detect, CHANGE);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);

  digitalWrite(LED_1, HIGH); 
  digitalWrite(LED_2, HIGH); 
}

/*ZCD Interrupt Function*/
void zero_cross_detect() 
{
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  dim_value = 0;
  digitalWrite(DIMMER_1, LOW);      // turn off TRIAC (and AC)
}

/*Timer Interrupt Function used to trigger the triac for Dimming*/
void dim_check() 
{
  /*For Dimmer */
  if (zero_cross == true) 
  {
    if (dim_value >= dimming) 
    {
      digitalWrite(DIMMER_1, HIGH); // turn on Triac 
      dim_value = 0; // reset time step counter
      zero_cross = false; //reset zero cross detection
    }
    else 
    {
      dim_value++; // increment time step counter
    }
  }
}


void loop() 
{
   int_regulator=analogRead(V_Switch_4);
   double mul =0.096679687; // (99/1024)
   float data = int_regulator*mul; //  x* (99/1024)  x is adc data
   regulator_value_temp="Dimmer:"+String(((int)data));
// Serial.println("Regulator value to "+regulator_value_temp);
/*############### Flag setting for Dimmable Triac through Pot ###############*/

  if(regulator_value_temp!=regulator_value)
  {
    regulator_value=regulator_value_temp;
    regulator_value_changed =true;
  }
  else
  {
    regulator_value_changed =false;
  }
/*############### Uart Data ###########################*/
  
  if (Serial.available() > 0) 
  {   // is a character available
    
    serialReceived = Serial.readStringUntil('\n');
//  Serial.println(serialReceived);
/*############### Dimmer_1 ###########################*/
     if (serialReceived.substring(0, 7) == "status:")
    {
      dimmer_status = true;     
    }
    if (serialReceived.substring(0, 7) == "Dimmer:")
    {
      Dimmer_value_temp = serialReceived;     
    }
    
 /*############### Triac_1 ###########################*/   
     if (serialReceived.substring(0, 33) == "R_1 switched via web request to 1")
    {
      serialReceived1 = serialReceived;
    }
    
    if (serialReceived.substring(0, 33) == "R_1 switched via web request to 0")
    {
      serialReceived1 = serialReceived;
    }

/*############### Triac_2 ###########################*/
    if (serialReceived.substring(0, 33) == "R_2 switched via web request to 1")
    {
      serialReceived2 = serialReceived;
    }
    
    if (serialReceived.substring(0, 33) == "R_2 switched via web request to 0")
    {
      serialReceived2 = serialReceived;
    }
/*############### Triac_3 ###########################*/
    if (serialReceived.substring(0, 33) == "R_3 switched via web request to 1")
    {
      serialReceived3 = serialReceived;
    }
    
    if (serialReceived.substring(0, 33) == "R_3 switched via web request to 0")
    {
      serialReceived3 = serialReceived;
    }

/*############### Led indication###########################*/
    if (serialReceived.substring(0, 15) == "Wifi_Connecting")
    {
      digitalWrite(LED_2, LOW); 
      toggle_with_delay(LED_1,100);
    }
    
    if (serialReceived.substring(0, 17) == "Wifi_Disconnected")
    {
      digitalWrite(LED_1, LOW); 
      digitalWrite(LED_2, HIGH); 
    }
    if (serialReceived.substring(0, 19) == "Wifi_Not_Configured")
    {
      digitalWrite(LED_1, LOW); 
      toggle_with_delay(LED_2,100);
    }
    if (serialReceived.substring(0, 14) == "Wifi_Connected")
    {
      digitalWrite(LED_2, LOW); 
      digitalWrite(LED_1, HIGH); 
    }

  }

/*################## Flag setting for Dimmable Triac through uart ##################################*/

  if( Dimmer_value_temp!=Dimmer_value)
  {
    Dimmer_value=Dimmer_value_temp;
    dimmer_value_changed =true;
  }
  else 
  {
    dimmer_value_changed =false;
  }

/*#####################  Triac_1 ##############################*/
  
  if (((serialReceived1.substring(0, 33) == "R_1 switched via web request to 1") && (!(digitalRead(V_Switch_1)))) || ((!(serialReceived1.substring(0, 33) == "R_1 switched via web request to 1")) && ((digitalRead(V_Switch_1))))) //exor logic
  {
    if(digitalRead(TRIAC_1)==HIGH)
    {
     // Serial.println("Load 1 is OFF");
    }
    digitalWrite(TRIAC_1, LOW);    
  }
  else
  {
    if(digitalRead(TRIAC_1)==LOW)
    {
      //Serial.println("Load 1 is ON");
    }
    digitalWrite(TRIAC_1, HIGH); 
  }

/*#####################  Triac_2 ##############################*/
  
  if (((serialReceived2.substring(0, 33) == "R_2 switched via web request to 1") && (!(digitalRead(V_Switch_2)))) || ((!(serialReceived2.substring(0, 33) == "R_2 switched via web request to 1")) && ((digitalRead(V_Switch_2))))) //exor logic
  {
    if(digitalRead(TRIAC_2)==HIGH)
    {
     // Serial.println("Load 2 is OFF");
    }
    digitalWrite(TRIAC_2, LOW);    
  }
  else
  {
    if(digitalRead(TRIAC_2)==LOW)
    {
      //Serial.println("Load 2 is ON");
    }
    digitalWrite(TRIAC_2, HIGH); 
  }

/*#####################  Triac_3 ##############################*/
  
  if (((serialReceived3.substring(0, 33) == "R_3 switched via web request to 1") && (!(digitalRead(V_Switch_3)))) || ((!(serialReceived3.substring(0, 33) == "R_3 switched via web request to 1")) && ((digitalRead(V_Switch_3))))) //exor logic
  {
    if(digitalRead(TRIAC_3)==HIGH)
    {
      //Serial.println("Load 3 is OFF");
    }
    digitalWrite(TRIAC_3, LOW);    
  }
  else
  {
    if(digitalRead(TRIAC_3)==LOW)
    {
      //Serial.println("Load 3 is ON");
    }
    digitalWrite(TRIAC_3, HIGH); 
  }

  
/*####################### Dimmable Triac ##################################*/
  
  if (Dimmer_value.substring(0, 7) == "Dimmer:" && dimmer_value_changed == true )
  {
   // int sensorValue = map(Dimmer_value.substring(7, 9).toInt(), 0, 99, 0, 128);
   // Serial.println("map value sensor "+String(sensorValue));
//    dimming = Dimmer_width - sensorValue;
    dimming = Dimmer_width - Dimmer_value.substring(7, 9).toInt();
    delay(5);
   // Serial.println("Uart value to "+Dimmer_value);
    dimvalue = Dimmer_value.substring(7, 9).toInt();
  }

  if (regulator_value.substring(0, 7) == "Dimmer:" && regulator_value_changed == true )
  {
  
    dimming = Dimmer_width - regulator_value.substring(7, 9).toInt();
    delay(5);
    //Serial.println("Regulator value to "+regulator_value);
    dimvalue = regulator_value.substring(7, 9).toInt();
  }

  if(dimmer_status == true)
    {
     dim_status(); 
     dimmer_status = false;  
    }


}

void dim_status() 
{
  //to add 0 so that string matches channel of triac D:10xxx
  if(dimvalue>=10)
  {
   Serial.println("D:"+String(dimvalue)+digitalRead(TRIAC_1)+digitalRead(TRIAC_2)+digitalRead(TRIAC_3)); 
  }
  else //to add 0 so that string matches channel of triac D01xxx
  {
   Serial.println("D:0"+String(dimvalue)+digitalRead(TRIAC_1)+digitalRead(TRIAC_2)+digitalRead(TRIAC_3)); 
  }
  
}
