boolean connectMQTT() {
  if (mqttClient.connected()) {
    return true;
  }

  Serial.print("Connecting to MQTT server ");
  Serial.print(mqttServer);
  Serial.print(" as ");
  Serial.println(host);

   if (mqttClient.connect(host, (char*)mqtt_user.c_str(), (char*)mqtt_passwd.c_str(), (char*)pubTopic.c_str(), 0, 0, (char*)mqtt_will_msg.c_str())) //added on 28/07/18
  {
    Serial.println("Connected to MQTT broker with authentication");
    if (mqttClient.subscribe((char*)subTopic.c_str()))
    {
      Serial.println("Subsribed to topic.1");
    }
    else
    {
      Serial.println("NOT subsribed to topic 1");
    }
    mqttClient.loop();


    return true;
  }
  else if (mqttClient.connect(host))//added on 31/07/18
  {
    Serial.println("Connected to MQTT broker without authentication");
    if (mqttClient.subscribe((char*)subTopic.c_str()))
    {
      Serial.println("Subsribed to topic.1");
    }
    else
    {
      Serial.println("NOT subsribed to topic 1");
    }
    mqttClient.loop();


    return true;
  }
  else {
    Serial.println("MQTT connect failed! ");
    return false;
  }
}

void disconnectMQTT() {
  mqttClient.disconnect();
}

void mqtt_handler() {
//  if (toPub == 1) {
//    Debugln("DEBUG: Publishing state via MQTT");
//    if (pubState()) {
//      toPub = 0;
//    }
//  }
  mqttClient.loop();
  delay(100); //let things happen in background
}

void mqtt_arrived(char* subTopic, byte* payload, unsigned int length) { // handle messages arrived
  int i = 0;
//  Serial.print("MQTT message arrived:  topic: " + String(subTopic));
  // create character buffer with ending null terminator (string)

  for (i = 0; i < length; i++) {
    buf[i] = payload[i];
  }
  buf[i] = '\0';
  String msgString = String(buf);
  //Serial.println(" message: " + msgString);

  if (msgString == "Reset")
  {
    clearConfig();
    delay(10);
    Serial.println("Done, restarting!");
    ESP.reset();
    ESP.restart();
  }

  
  if (msgString.substring(0,5) == "D1_ON") 
  {
    Serial.println("Dimmer1:99");
  } 
  else if (msgString.substring(0,6) == "D1_OFF") 
  {
    Serial.println("Dimmer1:0");
  }
  else if (msgString.substring(0,8) == "Dimmer1:")
  { 
    Serial.print("Dimmer1:");
    Serial.println(msgString.substring(8,10));    
  }


  if (msgString.substring(0,5) == "D2_ON") 
  {
    Serial.println("Dimmer2:99");
  } 
  else if (msgString.substring(0,6) == "D2_OFF") 
  {
    Serial.println("Dimmer2:0");
  }
  else if (msgString.substring(0,8) == "Dimmer2:")
  { 
    Serial.print("Dimmer2:");
    Serial.println(msgString.substring(8,10));    
  }

    if (msgString.substring(0,5) == "D3_ON") 
  {
    Serial.println("Dimmer3:99");
  } 
  else if (msgString.substring(0,6) == "D3_OFF") 
  {
    Serial.println("Dimmer3:0");
  }
  else if (msgString.substring(0,8) == "Dimmer3:")
  { 
    Serial.print("Dimmer3:");
    Serial.println(msgString.substring(8,10));    
  }

    if (msgString.substring(0,5) == "D4_ON") 
  {
    Serial.println("Dimmer4:99");
  } 
  else if (msgString.substring(0,6) == "D4_OFF") 
  {
    Serial.println("Dimmer4:0");
  }
  else if (msgString.substring(0,8) == "Dimmer4:")
  { 
    Serial.print("Dimmer4:");
    Serial.println(msgString.substring(8,10));    
  }


  
  else if (msgString.substring(0,8) == "P_STATUS") 
  {
    mqtt_powpub = true;
    //Serial.println("mqtt_powpub is true");
  }
  else if (msgString.substring(0,6) == "P_CAL:")  //P_CAL:xxx,xxx
  { 
    String  tempString_mqtt =msgString;
  //float P,V;
  int ind1; 
  int ind2;
  ind1=tempString_mqtt.indexOf(':');
  ind2=tempString_mqtt.indexOf(',');
  P=tempString_mqtt.substring(ind1+1,ind2+1).toFloat();
  V=tempString_mqtt.substring(ind2+1).toFloat();
  //Serial.println(P);
  //Serial.println(V);
  calflag=1;
   Serial.print("Settings written ");
   saveConfig()? Serial.println("sucessfully.") : Serial.println("not succesfully!");;
   Serial.print("Calibrated");
   
  }
}

//boolean pubState() { //Publish the current state of the light
//  if (!connectMQTT()) {
//    delay(100);
//    if (!connectMQTT) {
//      Serial.println("Could not connect MQTT.");
//      Serial.println("Publish state NOK");
//      return false;
//    }
//  }
//  if (mqttClient.connected()) 
//  {
//    Serial.println("To publish state " + state );
//    if (mqttClient.publish((char*)pubTopic.c_str(), (char*) state.c_str())) {
//      Serial.println("Publish state OK");
//      return true;
//    } 
//    else 
//    {
//      Serial.println("Publish state NOK");
//      return false;
//    }
//  } 
//  else 
//  {
//    Serial.println("Publish state NOK");
//    Serial.println("No MQTT connection.");
//  }
//}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
