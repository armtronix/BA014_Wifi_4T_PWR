bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  int otaFlagC = json["otaFlag"];
  String esidC = json["esid"];
  String epassC = json["epass"];
  int iotModeC = json["iotMode"];
  String pubTopicC = json["pubTopic"];
  String subTopicC = json["subTopic"];
  String mqttServerC = json["mqttServer"];
  String mqtt_userC = json["mqtt_user"];      //added on 28/07/2018
  String mqtt_passwdC = json["mqtt_passwd"];  //added on 28/07/2018
  String mqtt_portC = json["mqtt_port"];  //added on 28/07/2018
  String SptempC  = json["Sptemp"];  //added on 28/07/2018
  String SvtempC  = json["Svtemp"];  //added on 28/07/2018
  String tempsetmulCurrentC=json["setmulCurrentJson"];
  String tempsetmulVoltageC=json["setmulVoltageJson"];
  String tempsetmulPowerC= json["setmulPowerJson"];
  double setmulCurrentC = tempsetmulCurrentC.toFloat();
  double setmulVoltageC = tempsetmulVoltageC.toFloat();
  double setmulPowerC = tempsetmulPowerC.toFloat();
#ifdef ALEXAEN
  String firstNameC = json["firstName"];
  String secondNameC = json["secondName"];
  String thirdNameC = json["thirdName"];
  String fourthNameC = json["fourthName"];
#endif


  // Real world application would store these values in some variables for
  // later use.
  otaFlag = otaFlagC;
  esid = esidC;
  epass = epassC;
  iotMode = iotModeC;
  pubTopic = pubTopicC;
  subTopic = subTopicC;
  mqttServer = mqttServerC;
  mqtt_user = mqtt_userC; //added on 28/07/2018
  mqtt_passwd = mqtt_passwdC; //added on 28/07/2018
  mqtt_port = mqtt_portC; //added on 28/07/2018
  Sptemp=SptempC;  //added on 28/07/2018
  Svtemp=SvtempC;   //added on 28/07/2018
  setmulCurrent =setmulCurrentC;
  setmulVoltage = setmulVoltageC;
  setmulPower = setmulPowerC;
  
#ifdef ALEXAEN
  firstName = firstNameC;
  secondName = secondNameC;
  thirdName = thirdNameC;
  fourthName = fourthNameC;
#endif

  Serial.print("otaFlag: ");
  Serial.println(otaFlag);
  Serial.print("esid: ");
  Serial.println(esid);
  Serial.print("epass: ");
  Serial.println(epass);
  Serial.print("iotMode: ");
  Serial.println(iotMode);
  Serial.print("pubTopic: ");
  Serial.println(pubTopic);
  Serial.print("subTopic: ");
  Serial.println(subTopic);
  Serial.print("mqttServer: ");
  Serial.println(mqttServer);
  Serial.print("mqtt_user: ");  //added on 28/07/2018
  Serial.println(mqtt_user);    //added on 28/07/2018
  Serial.print("mqtt_passwd: "); //added on 28/07/2018
  Serial.println(mqtt_passwd);  //added on 28/07/2018
  Serial.print("mqtt_port: "); //added on 28/07/2018
  Serial.println(mqtt_port);  //added on 28/07/2018
 // Serial.print("Power:"); //added on 28/07/2018
 // Serial.println(P);  //added on 28/07/2018
 // Serial.print("Input voltage::"); //added on 28/07/2018
 // Serial.println(V);  //added on 28/07/2018
  Serial.print("setmulCurrent: ");
  Serial.println(setmulCurrent);
  Serial.print("setmulVoltage: ");
  Serial.println(setmulVoltage);
  Serial.print("setmulPower: ");
  Serial.println(setmulPower);
#ifdef ALEXAEN
  Serial.print("Device one: ");
  Serial.println(firstName);
  Serial.print("Device two: ");
  Serial.println(secondName);
  Serial.print("Device three: ");
  Serial.println(thirdName);
  Serial.print("Device four: ");
  Serial.println(fourthName);
#endif
  Serial.print("esid: ");
  Serial.println(esid);


  return true;
}

bool saveConfig() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["otaFlag"] = otaFlag;
  json["esid"] = esid;
  json["epass"] = epass;
  json["iotMode"] = iotMode;
  json["pubTopic"] = pubTopic;
  json["subTopic"] = subTopic;
  json["mqttServer"] = mqttServer;
  json["mqtt_user"] = mqtt_user; //added on 28/07/2018
  json["mqtt_passwd"] = mqtt_passwd; //added on 28/07/2018
  json["mqtt_port"] = mqtt_port; //added on 28/07/2018
  json["Sptemp"] = Sptemp; //added on 28/07/2018
  json["Svtemp"] = Svtemp; //added on 28/07/2018
  json["setmulCurrentJson"] = String(setmulCurrent);
  json["setmulVoltageJson"] = String(setmulVoltage);
  json["setmulPowerJson"] = String(setmulPower);

#ifdef ALEXAEN
  json["firstName"] = firstName;
  json["secondName"] = secondName;
  json["thirdName"] = thirdName;
  json["fourthName"] = fourthName;
#endif

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}


void setOtaFlag(int intOta) {
  otaFlag = intOta;
  saveConfig();
  yield();
}

bool clearConfig() {
  Debugln("DEBUG: In config clear!");
  return SPIFFS.format();
}
