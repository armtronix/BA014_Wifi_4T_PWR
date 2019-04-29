/*Code for Wifi 4T_1A_PWR board
  The Board has one Triacs and it is dimmable

  This code is for Atmega328p
  Firmware Version: 0.1
  Hardware Version: 0.1

  Code Edited By :Naren
  Date: 11/02/19  //Description  modified the frequency step to 85 and dimming width to 100
  Last Edited By:Karthik B
  Date: 14/03/2019

// J1 header Programming pins 

VCC 5V . . GND
TXDE   . . RXDA
RXDE   . . TXDA
DTRE   . . DTRA
RTSE   . . GND


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

#define HUMANPRESSDELAY 50 // the delay in ms untill the press should be handled as a normal push by human. Button debounce.

unsigned long count_regulator1 = 0; //Button press time counter
unsigned long dimval1 = 0; //Button press time counter
int button_press_flag1 = 1;
byte curBrightness1 = 0;

unsigned long count_regulator2 = 0; //Button press time counter
unsigned long dimval2 = 0; //Button press time counter
int button_press_flag2 = 1;
byte curBrightness2 = 0;

unsigned long count_regulator3 = 0; //Button press time counter
unsigned long dimval3 = 0; //Button press time counter
int button_press_flag3 = 1;
byte curBrightness3 = 0;

unsigned long count_regulator4 = 0; //Button press time counter
unsigned long dimval4 = 0; //Button press time counter
int button_press_flag4 = 1;
byte curBrightness4 = 0;
byte i = 0;
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

  pinMode(V_Switch_1, INPUT_PULLUP); //Dimmer_1 V_Switch input
  pinMode(V_Switch_2, INPUT_PULLUP); //Dimmer_2 V_Switch input
  pinMode(V_Switch_3, INPUT_PULLUP); //Dimmer_3 V_Switch input
  pinMode(V_Switch_4, INPUT_PULLUP); //Dimmer_4 V_Switch input

  attachInterrupt(ZCD_INT, zero_cross_detect, CHANGE);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);

  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);


}


void btn_handle()
{
  if (count_regulator1 <= 9)
  {
    count_regulator1 = 0;
  }

  if (!digitalRead(V_Switch_1))
  {
    if (button_press_flag1 == 1)
    {
      button_press_flag1 = 0;
      if (count_regulator1 <= 9)
      {
        count_regulator1 = count_regulator1 + 10;
      }
      else
      {
        count_regulator1 = count_regulator1 + 10;
      }
      if (count_regulator1 <= 90)
      {
        //Serial.print("Reg1 VAL:");
        //Serial.println(count_regulator1);
        int_regulator1 = count_regulator1;
      }
      else if (count_regulator1 <= 100 && count_regulator1 > 90 )
      {
        dimval1 = count_regulator1 - 1;
        //Serial.print("Reg1 VAL:");
        //Serial.println(dimval1);
        int_regulator1 = dimval1;
      }
      else
      {
        count_regulator1 = 0;
        //Serial.print("Reg1 VAL:");
        //Serial.println(count_regulator1);
        int_regulator1 = count_regulator1;
      }
    }
  }
  else
  {
    button_press_flag1 = 1;
    if (count_regulator1 > 1 && count_regulator1 < HUMANPRESSDELAY / 5)
    {
      if (count_regulator1 <= 99)
      {
        //Serial.println(count_regulator1);
      }
      else
      {
        count_regulator1 = 0;
      }
    }
  }
  //######

  if (count_regulator2 <= 9)
  {
    count_regulator2 = 0;
  }

  if (!digitalRead(V_Switch_2))
  {
    if (button_press_flag2 == 1)
    {
      button_press_flag2 = 0;
      if (count_regulator2 <= 9)
      {
        count_regulator2 = count_regulator2 + 10;
      }
      else
      {
        count_regulator2 = count_regulator2 + 10;
      }
      if (count_regulator2 <= 90)
      {
        //Serial.print("Reg2 VAL:");
        //Serial.println(count_regulator2);
        int_regulator2 = count_regulator2;
      }
      else if (count_regulator2 <= 100 && count_regulator2 > 90 )
      {
        dimval2 = count_regulator2 - 1;
        //Serial.print("Reg2 VAL:");
        //Serial.println(dimval2);
        int_regulator2 = dimval2;
      }
      else
      {
        count_regulator2 = 0;
        //Serial.print("Reg2 VAL:");
        //Serial.println(count_regulator2);
        int_regulator2 = count_regulator2;
      }
    }
  }
  else
  {
    button_press_flag2 = 1;
    if (count_regulator2 > 1 && count_regulator2 < HUMANPRESSDELAY / 5)
    {
      if (count_regulator2 <= 99)
      {
        //Serial.println(count_regulator2);
      }
      else
      {
        count_regulator2 = 0;
      }
    }
  }
  //#######

  if (count_regulator3 <= 9)
  {
    count_regulator3 = 0;
  }

  if (!digitalRead(V_Switch_3))
  {
    if (button_press_flag3 == 1)
    {
      button_press_flag3 = 0;
      if (count_regulator3 <= 9)
      {
        count_regulator3 = count_regulator3 + 10;
      }
      else
      {
        count_regulator3 = count_regulator3 + 10;
      }
      if (count_regulator3 <= 90)
      {
        //Serial.print("Reg3 VAL:");
        //Serial.println(count_regulator3);
        int_regulator3 = count_regulator3;
      }
      else if (count_regulator3 <= 100 && count_regulator3 > 90 )
      {
        dimval3 = count_regulator3 - 1;
        //Serial.print("Reg3 VAL:");
        //Serial.println(dimval3);
        int_regulator3 = dimval3;
      }
      else
      {
        count_regulator3 = 0;
        //Serial.print("Reg3 VAL:");
        //Serial.println(count_regulator3);
        int_regulator3 = count_regulator3;
      }
    }
  }
  else
  {
    button_press_flag3 = 1;
    if (count_regulator3 > 1 && count_regulator3 < HUMANPRESSDELAY / 5)
    {
      if (count_regulator3 <= 99)
      {
        //Serial.println(count_regulator3);
      }
      else
      {
        count_regulator3 = 0;
      }
    }
  }

  //#######

  if (count_regulator4 <= 9)
  {
    count_regulator4 = 0;
  }

  if (!digitalRead(V_Switch_4))
  {
    if (button_press_flag4 == 1)
    {
      button_press_flag4 = 0;
      if (count_regulator4 <= 9)
      {
        count_regulator4 = count_regulator4 + 10;
      }
      else
      {
        count_regulator4 = count_regulator4 + 10;
      }
      if (count_regulator4 <= 90)
      {
        //Serial.print("Reg4 VAL:");
        //Serial.println(count_regulator4);
        int_regulator4 = count_regulator4;
      }
      else if (count_regulator4 <= 100 && count_regulator4 > 90 )
      {
        dimval4 = count_regulator4 - 1;
        //Serial.print("Reg4 VAL:");
        //Serial.println(dimval4);
        int_regulator4 = dimval4;
      }
      else
      {
        count_regulator4 = 0;
        //Serial.print("Reg4 VAL:");
        //Serial.println(count_regulator4);
        int_regulator4 = count_regulator4;
      }
    }
  }
  else
  {
    button_press_flag4 = 1;
    if (count_regulator4 > 1 && count_regulator4 < HUMANPRESSDELAY / 5)
    {
      if (count_regulator4 <= 99)
      {
        //Serial.println(count_regulator4);
      }
      else
      {
        count_regulator4 = 0;
      }
    }
  }


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

  i++;

  if (i >= 100)
  {
    btn_handle();

    regulator_value_temp1 = "Dimmer1:" + String((int_regulator1));
    regulator_value_temp2 = "Dimmer2:" + String((int_regulator2));
    regulator_value_temp3 = "Dimmer3:" + String((int_regulator3));
    regulator_value_temp4 = "Dimmer4:" + String((int_regulator4));
    i = 0;
  }


  /*############### Flag setting for Dimmable Triac ONE through Pot ###############*/

  if (regulator_value_temp1 != regulator_value1)
  {
    regulator_value1 = regulator_value_temp1;
    regulator_value_changed1 = true;
  }
  else
  {
    regulator_value_changed1 = false;
  }

  /*############### Flag setting for Dimmable Triac two through Pot ###############*/

  if (regulator_value_temp2 != regulator_value2)
  {
    regulator_value2 = regulator_value_temp2;
    regulator_value_changed2 = true;
  }
  else
  {
    regulator_value_changed2 = false;
  }
  /*############### Flag setting for Dimmable Triac three through Pot ###############*/

  if (regulator_value_temp3 != regulator_value3)
  {
    regulator_value3 = regulator_value_temp3;
    regulator_value_changed3 = true;
  }
  else
  {
    regulator_value_changed3 = false;
  }
  /*############### Flag setting for Dimmable Triac four through Pot ###############*/

  if (regulator_value_temp4 != regulator_value4)
  {
    regulator_value4 = regulator_value_temp4;
    regulator_value_changed4 = true;
  }
  else
  {
    regulator_value_changed4 = false;
  }
  //################################################################################


  /*############### Uart Data ###########################*/

  if (Serial.available() > 0)
  { // is a character available

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

  if ( Dimmer_value_temp1 != Dimmer_value1)
  {
    Dimmer_value1 = Dimmer_value_temp1;
    dimmer_value_changed1 = true;
  }
  else
  {
    dimmer_value_changed1 = false;
  }

  /*################## Flag setting for Dimmable Triac two through uart ##################################*/

  if ( Dimmer_value_temp2 != Dimmer_value2)
  {
    Dimmer_value2 = Dimmer_value_temp2;
    dimmer_value_changed2 = true;
  }
  else
  {
    dimmer_value_changed2 = false;
  }

  /*################## Flag setting for Dimmable Triac three through uart ##################################*/

  if ( Dimmer_value_temp3 != Dimmer_value3)
  {
    Dimmer_value3 = Dimmer_value_temp3;
    dimmer_value_changed3 = true;
  }
  else
  {
    dimmer_value_changed3 = false;
  }

  /*################## Flag setting for Dimmable Triac four through uart ##################################*/

  if ( Dimmer_value_temp4 != Dimmer_value4)
  {
    Dimmer_value4 = Dimmer_value_temp4;
    dimmer_value_changed4 = true;
  }
  else
  {
    dimmer_value_changed4 = false;
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
    Serial.println("Regulator value to " + regulator_value1);
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
    Serial.println("Regulator value to " + regulator_value2);
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
    Serial.println("Regulator value to " + regulator_value3);
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
    Serial.println("Regulator value to " + regulator_value4);
    dimvalue4 = regulator_value4.substring(8, 10).toInt();
  }

  //################################################################################

  if (dimmer_status == true)
  {
    Serial.println("D:" + zero_adj(dimvalue1) + "," + zero_adj(dimvalue2) + "," + zero_adj(dimvalue3) + "," + zero_adj(dimvalue4));
    dimmer_status = false;
  }
}

//end off loop

String zero_adj(int val)
{
  if (val >= 10)
  {
    return String(val);
  }
  else
  {
    return "0" + String(val);
  }
}





