//Relay
#define Relay1 8  //Gpio 8
#define Relay2 9  //Gpio 9
#define Relay3 10 //Gpio 10
#define Relay4 6  //Gpio 6

//manual switch
#define SWITCH_INPIN1 A0 //switch 1
#define SWITCH_INPIN2 A1 //switch 2
#define SWITCH_INPIN3 A2 //switch 3
#define SWITCH_INPIN4 A3 //switch 4

String serialReceived;
String serialReceived1, serialReceived2, serialReceived3, serialReceived4;
String binarydata;
int setflag1 = 0;
int setflag2 = 0;
int setflag3 = 0;
int setflag4 = 0;

//unsigned long currentmillis;
//unsigned long lastmillis = 0;
//long ontime = 2000;
int relayflag1 = 0 ;
int relayflag2 = 0 ;
int relayflag3 = 0 ;
int relayflag4 = 0 ;

void setup() {

  Serial.begin(19200);
  Serial.println("Wifi 4T Relay Board"); //added on 1-03-18
  pinMode(Relay1, OUTPUT); //relay1 output
  pinMode(Relay2, OUTPUT); //relay2 output
  pinMode(Relay3, OUTPUT); //relay3 output
  pinMode(Relay4, OUTPUT); //relay4 output
  
  pinMode(SWITCH_INPIN1, INPUT); //manual switch 1 input
  pinMode(SWITCH_INPIN2, INPUT); //manual switch 2 input
  pinMode(SWITCH_INPIN3, INPUT); //manual switch 3 input
  pinMode(SWITCH_INPIN4, INPUT); //manual switch 4 input
  
    
  }
void loop() 
{
 
  if (Serial.available() > 0) 
    {   // is a character available
       serialReceived = Serial.readStringUntil('\n');
     if (serialReceived.substring(0, 7) == "Relays:")
      {
        String tempbinary = String((serialReceived.substring(7).toInt()), BIN);
            if (tempbinary.length() == 1)
              {
                binarydata = "000" + tempbinary;
              }
            else if (tempbinary.length() == 2)
              {
                binarydata = "00" + tempbinary;
              }
            else if (tempbinary.length() == 3)
              {
                binarydata = "0" + tempbinary;
              }
            else if (tempbinary.length() == 4)
              {
                binarydata = tempbinary;
              }
              
       }

     if ((binarydata.substring(3, 4) == "1") && (digitalRead(Relay1) == LOW))
       {
           digitalWrite(Relay1, HIGH);
          // Serial.println("HIGH via serial1");
       } 
     else  if ((binarydata.substring(3, 4) == "0") && (digitalRead(Relay1) == HIGH))
       {
         digitalWrite(Relay1, LOW);
        // Serial.println("low via serial1"); 
       }
     if (binarydata.substring(2, 3) == "1" && (digitalRead(Relay2) == LOW))
       {
         digitalWrite(Relay2, HIGH);
       //  Serial.println("HIGH via serial2");
       } 
     else if (binarydata.substring(2, 3) == "0" && (digitalRead(Relay2) == HIGH))
       {
         digitalWrite(Relay2, LOW);
       //  Serial.println("low via serial2"); 
       }
     if (binarydata.substring(1, 2) == "1" && (digitalRead(Relay3) == LOW))
       {
         digitalWrite(Relay3, HIGH);
     //    Serial.println("HIGH via serial3");
       } 
     else if (binarydata.substring(1, 2) == "0" && (digitalRead(Relay3) == HIGH))
       {
         digitalWrite(Relay3, LOW);
    //    Serial.println("low via serial3"); 
       }
     if (binarydata.substring(0, 1) == "1" && (digitalRead(Relay4) == LOW))
      {
         digitalWrite(Relay4, HIGH);
     //    Serial.println("HIGH via serial4");
       } 
     else if (binarydata.substring(0, 1) == "0" && (digitalRead(Relay4) == HIGH))
       {
       digitalWrite(Relay4, LOW);
    //    Serial.println("low via serial4"); 
       }

    }
  
 ///////////////////////////////////////////////////////////////////////////////////////
  if((digitalRead(SWITCH_INPIN1) == LOW) && (setflag1 == 0))
   {   
      if(digitalRead(Relay1) == HIGH)
        {
         relayflag1 = 1;
        }
      else
        {
          relayflag1 = 0;
        }
        setflag1 = 1;
   }
  else if((digitalRead(SWITCH_INPIN1) == HIGH) && (setflag1 == 1))
   {    
      if(digitalRead(Relay1) == HIGH)
        {
         relayflag1 = 1;
        }
      else
        {
          relayflag1 = 0;
        }
        setflag1 = 0;
   }
////////////////////////////////////////////////////////////////////////////////
  if((digitalRead(SWITCH_INPIN2) == LOW) && (setflag2 == 0))
   {  
     if(digitalRead(Relay2) == HIGH)
        {
         relayflag2 = 1;
        }
      else
        {
          relayflag2 = 0;
        }
        setflag2 = 1;
   }
  else if((digitalRead(SWITCH_INPIN2) == HIGH) && (setflag2 == 1))
   {  
     if(digitalRead(Relay2) == HIGH)
      {
       relayflag2 = 1;
      }
     else
      {
        relayflag2 = 0;
      }
        setflag2 = 0;
   }
//////////////////////////////////////////////////////////////////////////////////

  if((digitalRead(SWITCH_INPIN3) == LOW) && (setflag3 == 0))
   {
  
     if(digitalRead(Relay3) == HIGH)
      {
       relayflag3 = 1;
      }
     else
      {
        relayflag3 = 0;
      }
        setflag3 = 1;
   }
  else if((digitalRead(SWITCH_INPIN3) == HIGH) && (setflag3 == 1))
   {  
     if(digitalRead(Relay3) == HIGH)
      {
       relayflag3 = 1;
      }
     else
      {
        relayflag3 = 0;
      }
     setflag3 = 0;
   }
//////////////////////////////////////////////////////////////////////////////////
  if((digitalRead(SWITCH_INPIN4) == LOW) && (setflag4 == 0))
   {   
     if(digitalRead(Relay4) == HIGH)
      {
       relayflag4 = 1;
      }
     else
      {
        relayflag4 = 0;
      }
        setflag4 = 1;
   }
  else if((digitalRead(SWITCH_INPIN4) == HIGH) && (setflag4 == 1))
   { 
     if(digitalRead(Relay4) == HIGH)
      {
       relayflag4 = 1;
      }
     else
      {
        relayflag4 = 0;
      }
        setflag4 = 0;
   }

//////////////////////////////////////////////////////////////////////////////////
 
 if (relayflag1 == 0)
    {
     Serial.println("POWER1 ON");   
     relayflag1 =2;
    }
  else if (relayflag1 == 1)
    {
      Serial.println("POWER1 OFF");     
      relayflag1 =2;
    }
   
 if (relayflag2 == 0)
    {
     Serial.println("POWER2 ON");     
     relayflag2 = 2;
    }
  else if (relayflag2 == 1)
    {
      Serial.println("POWER2 OFF");      
      relayflag2 = 2;
    }

 if (relayflag3 == 0)
    {
     Serial.println("POWER3 ON");     
     relayflag3 = 2;
    }
  else if (relayflag3 == 1)
    {
      Serial.println("POWER3 OFF");      
      relayflag3 = 2;
    }

 if (relayflag4 == 0)
    {
     Serial.println("POWER4 ON");     
     relayflag4 = 2;
    }
  else if (relayflag4 == 1)
    {
      Serial.println("POWER4 OFF");      
      relayflag4 = 2;
    }

}
