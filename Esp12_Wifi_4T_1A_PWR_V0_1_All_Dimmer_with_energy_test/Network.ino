void Scan_Wifi_Networks()
{
  WiFi.mode(WIFI_STA); //added on 14/03/19 by naren
  WiFi.disconnect();  //added on 14/03/19 by naren
  delay(10);
  int n = WiFi.scanNetworks();


  if (n <= 0)
  {
    Serial.println("no networks found");
  }
  else if(n > 0)
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      if (esid == WiFi.SSID(i))

      {
        Serial.println("Old Network Found ");
        Do_Connect();
    
      }
      else
      {
        Serial.println("Old Network Not Found");
      }
     
    }
  }
  Serial.println("");
    host= (char*) hostsaved.c_str();//added on 14/03/19 by naren
  Serial.println("");
  WiFi.disconnect();
  delay(10);//changed from 100 to 10
  WiFi.mode(WIFI_AP);
  WiFi.softAP(host);
  WiFi.begin(host); // not sure if need but works
  Serial.print("Searching for network , Also Access point started with name ");
  Serial.println(host);
  inApMode=1;
  launchWeb(1);
}


void Do_Connect()                  // Try to connect to the Found WIFI Network!
{

delay(500);
ESP.wdtDisable();
ESP.reset();
digitalWrite(RESET_PIN, LOW);
  
}
