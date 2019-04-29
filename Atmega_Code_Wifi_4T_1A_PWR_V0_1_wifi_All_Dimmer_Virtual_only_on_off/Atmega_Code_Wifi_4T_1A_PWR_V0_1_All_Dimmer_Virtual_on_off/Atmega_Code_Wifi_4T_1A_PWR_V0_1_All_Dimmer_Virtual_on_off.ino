/*Code for Wifi 4T_1A_PWR board
  The Board has one Triacs and it is dimmable

  This code is for Atmega328p
  Firmware Version: 0.1
  Hardware Version: 0.1

  Code Edited By :Naren
  Date: 11/02/19  //Description  modified the frequency step to 85 and dimming width to 100
  Last Edited By:Karthik B
  Date: 14/03/2019


#############J1 header Programming pins################# 

VCC 5V . . GND
TXDE   . . RXDA
RXDE   . . TXDA
DTRE   . . DTRA
RTSE   . . GND
  
#########################################################
*/
#include  <TimerOne.h>
#define version_no "FVer 0.1 ,HVer 0.1"

/*one line function*/
#define toggle(x)  digitalWrite(x, (digitalRead(x)^1)) ;
#define toggle_with_delay(x,y)  digitalWrite(x, (digitalRead(x)^1)) ; delay(y);

/*Dimmer pin no.*/
#define DIMMER_1 8 //Arduino Pin 10   
#define DIMMER_2 9 //Arduino Pin 8
#define DIMMER_3 10 //Arduino Pin 9
#define DIMMER_4 6 //Arduino Pin 6

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

int dimming1 = 100;  //100
int dimming2 = 100;  //100
int dimming3 = 100;  //100
int dimming4 = 100;  //100

int freqStep = 85;//85,82,80 //75*5 as prescalar is 16 for 80MHZ //77 for the led


/*Serial Data variables*/
String serialReceived;

String Dimmer_value_temp1;
String Dimmer_value_temp2;
String Dimmer_value_temp3;
String Dimmer_value_temp4;

String Dimmer_value1;
String Dimmer_value2;
String Dimmer_value3;
String Dimmer_value4;

String regulator_value_temp1;
String regulator_value_temp2;
String regulator_value_temp3;
String regulator_value_temp4;

/*POT Variable */
String regulator_value1;
String regulator_value2;
String regulator_value3;
String regulator_value4;

/*ZCD Variables */

volatile int dim_value1 = 0;
volatile int dim_value2 = 0;
volatile int dim_value3 = 0;
volatile int dim_value4 = 0;

volatile boolean zero_cross1 = 0;
volatile boolean zero_cross2 = 0;
volatile boolean zero_cross3 = 0;
volatile boolean zero_cross4 = 0;

volatile int int_regulator1;
volatile int int_regulator2;
volatile int int_regulator3;
volatile int int_regulator4;


/*Flags for Dimmer virtual switch concept */
volatile boolean dimmer_value_changed1 = false;
volatile boolean dimmer_value_changed2 = false;
volatile boolean dimmer_value_changed3 = false;
volatile boolean dimmer_value_changed4 = false;

volatile boolean regulator_value_changed1 = false;
volatile boolean regulator_value_changed2 = false;
volatile boolean regulator_value_changed3 = false;
volatile boolean regulator_value_changed4 = false;

volatile boolean dimmer_status = false;
volatile boolean mqttconnected = false;

int dimvalue1;
int dimvalue2;
int dimvalue3;
int dimvalue4;

void setup()
{
  delay(100);
  Serial.begin(115200);
  Serial.println("WiFi-4T channel 1A PWR board");
  Serial.println(version_no);

  pinMode(DIMMER_1, OUTPUT); //Dimmer_1 output
  pinMode(DIMMER_2, OUTPUT); //Dimmer_2 output
  pinMode(DIMMER_3, OUTPUT); //Dimmer_3 output
  pinMode(DIMMER_4, OUTPUT); //Dimmer_4 output

  pinMode(LED_1, OUTPUT);  //LED_1 output
  pinMode(LED_2, OUTPUT);  //LED_2 output

  pinMode(V_Switch_1, INPUT); //Dimmer_1 V_Switch input
  pinMode(V_Switch_2, INPUT); //Dimmer_2 V_Switch input
  pinMode(V_Switch_3, INPUT); //Dimmer_3 V_Switch input
  pinMode(V_Switch_4, INPUT); //Dimmer_4 V_Switch input

  attachInterrupt(ZCD_INT, zero_cross_detect, CHANGE);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);

  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);


}


/*ZCD Interrupt Function*/
void zero_cross_detect()
{
  zero_cross1 = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  zero_cross2 = true;
  zero_cross3 = true;
  zero_cross4 = true;
  dim_value1 = 0;
  dim_value2 = 0;
  dim_value3 = 0;
  dim_value4 = 0;
  digitalWrite(DIMMER_1, LOW);      // turn off TRIAC (and AC)
  digitalWrite(DIMMER_2, LOW);      // turn off TRIAC (and AC)
  digitalWrite(DIMMER_3, LOW);      // turn off TRIAC (and AC)
  digitalWrite(DIMMER_4, LOW);      // turn off TRIAC (and AC)
}

/*Timer Interrupt Function used to trigger the triac for Dimming*/
void dim_check()
{
  /*For Dimmer */
  if (zero_cross1 == true)
   {

      //#######Dimmer_one##########
      if (dim_value1 >= dimming1)
        {
           digitalWrite(DIMMER_1, HIGH); // turn on Triac
           dim_value1 = 0; // reset time step counter
           zero_cross1 = false; //reset zero cross detection
        }
        else
        {
        dim_value1++; // increment time step counter
        }
   }

   if (zero_cross2 == true)
   {
      //#######Dimmer_two##########
      if (dim_value2 >= dimming2)
        {
           digitalWrite(DIMMER_2, HIGH); // turn on Triac
           dim_value2 = 0; // reset time step counter
           zero_cross2 = false; //reset zero cross detection
        }
        else
        {
        dim_value2++; // increment time step counter
        }
   }    

   if (zero_cross3 == true)
   {
      //#######Dimmer_three##########
      if (dim_value3 >= dimming3)
        {
           digitalWrite(DIMMER_3, HIGH); // turn on Triac
           dim_value3 = 0; // reset time step counter
           zero_cross3 = false; //reset zero cross detection
        }
        else
        {
        dim_value3++; // increment time step counter
        }
   }

   if (zero_cross4 == true)
   {
      //#######Dimmer_four##########
      if (dim_value4 >= dimming4)
        {
           digitalWrite(DIMMER_4, HIGH); // turn on Triac
           dim_value4 = 0; // reset time step counter
           zero_cross4 = false; //reset zero cross detection
        }
        else
        {
        dim_value4++; // increment time step counter
        }
    }
}


void loop()
{


//////#######################READ V_SWITCH_1##################################

int_regulator1=analogRead(V_Switch_1);  // added on 06/03/19          
   int data1 ;
   //Serial.println(int_regulator1);
  if (int_regulator1 >=500)
   {
    data1 = 0;//  x* (99/1024)  x is adc data
    //Serial.println(data1);
   }
   else if (int_regulator1 <499)
   {
    data1 = 99;//  x* (99/1024)  x is adc data
    //Serial.println(data1);
   }
   regulator_value_temp1="Dimmer1:"+String(data1);
   //Serial.println("Regulator value to "+regulator_value_temp1);


//#######################READ V_SWITCH_2##################################

int_regulator2=analogRead(V_Switch_2);  // added on 06/03/19          
   int data2 ;
  // Serial.println(int_regulator2);
  if (int_regulator2 >=500)
   {
    data2 = 0;//  x* (99/1024)  x is adc data
    //Serial.println(data2);
   }
   else if (int_regulator2 <160)
   {
    data2 = 99;//  x* (99/1024)  x is adc data
    //Serial.println(data2);
   }
   regulator_value_temp2="Dimmer2:"+String(data2);
   //Serial.println("Regulator value to "+regulator_value_temp2);
//#######################READ V_SWITCH_3##################################
int_regulator3=analogRead(V_Switch_3);  // added on 06/03/19          
   int data3 ;
  // Serial.println(int_regulator3);
  if (int_regulator3 >=500)
   {
    data3 = 0;//  x* (99/1024)  x is adc data
    //Serial.println(data3);
   }
   
   else if (int_regulator3 <499)
   {
    data3 = 99;//  x* (99/1024)  x is adc data
    //Serial.println(data3);
   }
   regulator_value_temp3="Dimmer3:"+String(data3);
   //Serial.println("Regulator value to "+regulator_value_temp3);


//#######################READ V_SWITCH_4##################################

int_regulator4 = analogRead(V_Switch_4);  // added on 06/03/19          
   int data4 ;
  // Serial.println(int_regulator4);
  if (int_regulator4 >=500)
   {
    data4 = 0;//  x* (99/1024)  x is adc data
    //Serial.println(data4);
   }
   else if (int_regulator4 <499)
   {
    data4 = 99;//  x* (99/1024)  x is adc data
    //Serial.println(data4);
   }
   regulator_value_temp4="Dimmer4:"+String(data4);
   //Serial.println("Regulator value to "+regulator_value_temp4);

/*############### Flag setting for Dimmable Triac ONE through Pot ###############*/

  if(regulator_value_temp1!=regulator_value1)
  {
    regulator_value1=regulator_value_temp1;
    regulator_value_changed1 =true;
  }
  else
  {
    regulator_value_changed1 =false;
  }

/*############### Flag setting for Dimmable Triac two through Pot ###############*/

  if(regulator_value_temp2!=regulator_value2)
  {
    regulator_value2=regulator_value_temp2;
    regulator_value_changed2 =true;
  }
  else
  {
    regulator_value_changed2 =false;
  }
/*############### Flag setting for Dimmable Triac three through Pot ###############*/

  if(regulator_value_temp3!=regulator_value3)
  {
    regulator_value3=regulator_value_temp3;
    regulator_value_changed3 =true;
  }
  else
  {
    regulator_value_changed3 =false;
  }
/*############### Flag setting for Dimmable Triac four through Pot ###############*/

  if(regulator_value_temp4!=regulator_value4)
  {
    regulator_value4=regulator_value_temp4;
    regulator_value_changed4 =true;
  }
  else
  {
    regulator_value_changed4 =false;
  }
//################################################################################


/*############### Uart Data ###########################*/
  
  if (Serial.available() > 0) 
  {   // is a character available
    
    serialReceived = Serial.readStringUntil('\n');
//  Serial.println(serialReceived);
     
     if (serialReceived.substring(0, 7) == "status:")
    {
      dimmer_status = true;     
    }
/*############### Dimmer_1 ###########################*/

    if (serialReceived.substring(0, 8) == "Dimmer1:")
    {
      Dimmer_value_temp1 = serialReceived;     
    }

/*############### Dimmer_2 ###########################*/
    if (serialReceived.substring(0, 8) == "Dimmer2:")
    {
      Dimmer_value_temp2 = serialReceived;     
    }
/*############### Dimmer_3 ###########################*/
    if (serialReceived.substring(0, 8) == "Dimmer3:")
    {
      Dimmer_value_temp3 = serialReceived;     
    }
/*############### Dimmer_4 ###########################*/
    if (serialReceived.substring(0, 8) == "Dimmer4:")
    {
      Dimmer_value_temp4 = serialReceived;     
    }
    
/*############### Led indication###########################*/
if (serialReceived.substring(0, 1) == ".")  //Wifi_Connecting
    {
      digitalWrite(LED_2, HIGH);
      toggle_with_delay(LED_1, 100);
    }

    if (serialReceived.substring(0, 21) == "Old Network Not Found")
    {
      digitalWrite(LED_2, HIGH);
      toggle_with_delay(LED_1, 1000);
    }
    if (serialReceived.substring(0, 11) == "192.168.4.1") // wif config mode
    {
      Serial.println("Esp hosting");
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_2, HIGH);
    }
    if (serialReceived.substring(0, 14) == "WiFi connected")
    {
      Serial.println("Wifi Connected");
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_1, LOW);
    }

  }


  /*################## Flag setting for Dimmable Triac ONE through uart ##################################*/

  if( Dimmer_value_temp1!=Dimmer_value1)
  {
    Dimmer_value1=Dimmer_value_temp1;
    dimmer_value_changed1 =true;
  }
  else 
  {
    dimmer_value_changed1 =false;
  }

  /*################## Flag setting for Dimmable Triac two through uart ##################################*/

  if( Dimmer_value_temp2!=Dimmer_value2)
  {
    Dimmer_value2=Dimmer_value_temp2;
    dimmer_value_changed2 =true;
  }
  else 
  {
    dimmer_value_changed2 =false;
  }

  /*################## Flag setting for Dimmable Triac three through uart ##################################*/

  if( Dimmer_value_temp3!=Dimmer_value3)
  {
    Dimmer_value3=Dimmer_value_temp3;
    dimmer_value_changed3 =true;
  }
  else 
  {
    dimmer_value_changed3 =false;
  }  

  /*################## Flag setting for Dimmable Triac four through uart ##################################*/

  if( Dimmer_value_temp4!=Dimmer_value4)
  {
    Dimmer_value4=Dimmer_value_temp4;
    dimmer_value_changed4 =true;
  }
  else 
  {
    dimmer_value_changed4 =false;
  }
  //####################################################################################################

  /*####################### Dimmable Triac 1 ##################################*/
  
  if (Dimmer_value1.substring(0, 8) == "Dimmer1:" && dimmer_value_changed1 == true )
  {
    dimming1 = Dimmer_width - Dimmer_value1.substring(8, 10).toInt();
    delay(5);
    //Serial.println("Uart value to "+Dimmer_value1);
    dimvalue1 = Dimmer_value1.substring(8, 10).toInt();
  }

  if (regulator_value1.substring(0, 8) == "Dimmer1:" && regulator_value_changed1 == true )
  {
    dimming1 = Dimmer_width - regulator_value1.substring(8, 10).toInt();
    delay(5);
    Serial.println("Regulator value to "+regulator_value1);
    dimvalue1 = regulator_value1.substring(8, 10).toInt();
  }

  /*####################### Dimmable Triac 2 ##################################*/
  
  if (Dimmer_value2.substring(0, 8) == "Dimmer2:" && dimmer_value_changed2 == true )
  {
    dimming2 = Dimmer_width - Dimmer_value2.substring(8, 10).toInt();
    delay(5);
   //Serial.println("Uart value to "+Dimmer_value2);
    dimvalue2 = Dimmer_value2.substring(8, 10).toInt();
  }

  if (regulator_value2.substring(0, 8) == "Dimmer2:" && regulator_value_changed2 == true )
  {
    dimming2 = Dimmer_width - regulator_value2.substring(8, 10).toInt();
    delay(5);
    Serial.println("Regulator value to "+regulator_value2);
    dimvalue2 = regulator_value2.substring(8, 10).toInt();
  }

    /*####################### Dimmable Triac 3 ##################################*/
  
  if (Dimmer_value3.substring(0, 8) == "Dimmer3:" && dimmer_value_changed3 == true )
  {
    dimming3 = Dimmer_width - Dimmer_value3.substring(8, 10).toInt();
    delay(5);
    //Serial.println("Uart value to "+Dimmer_value3);
    dimvalue3 = Dimmer_value3.substring(8, 10).toInt();
  }

  if (regulator_value3.substring(0, 8) == "Dimmer3:" && regulator_value_changed3 == true )
  {
    dimming3 = Dimmer_width - regulator_value3.substring(8, 10).toInt();
    delay(5);
    Serial.println("Regulator value to "+regulator_value3);
    dimvalue3 = regulator_value3.substring(8, 10).toInt();
  }

    /*####################### Dimmable Triac 4 ##################################*/
  
  if (Dimmer_value4.substring(0, 8) == "Dimmer4:" && dimmer_value_changed4 == true )
  {
    dimming4 = Dimmer_width - Dimmer_value4.substring(8, 10).toInt();
    delay(5);
   // Serial.println("Uart value to "+Dimmer_value4);
    dimvalue4 = Dimmer_value4.substring(8, 10).toInt();
  }

  if (regulator_value4.substring(0, 8) == "Dimmer4:" && regulator_value_changed4 == true )
  {
    dimming4 = Dimmer_width - regulator_value4.substring(8, 10).toInt();
    delay(5);
    Serial.println("Regulator value to "+regulator_value4);
    dimvalue4 = regulator_value4.substring(8, 10).toInt();
  }

  //################################################################################

   if(dimmer_status == true)
    {
     Serial.println("D:"+zero_adj(dimvalue1)+","+zero_adj(dimvalue2)+","+zero_adj(dimvalue3)+","+zero_adj(dimvalue4)); 
     dimmer_status = false;  
    }
}

//end off loop

String zero_adj(int val)
{
  if(val>=10)
  {
    return String(val);
  }
  else
  {
    return "0"+String(val);
  }
}





