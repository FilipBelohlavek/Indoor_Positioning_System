#include <HomeAssistant.h>


#if DEVICE_TYPE == RGBW

void HomeAssistant::processJson(char* incomingMessage){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, incomingMessage);

 
  if(doc.containsKey("brightness")){
    this->brightness = doc["brightness"];
  }

  JsonObject color = doc["color"];
  
  if(color.containsKey("r")){
    this->red = color["r"];
    this->white = 0;
  }

  if(color.containsKey("g")){
    this->green = color["g"];
    this->white = 0;
  }

  if(color.containsKey("b")){
    this->blue = color["b"];
    this->white = 0;
  }

  if(doc.containsKey("white_value")){
    this->white = doc["white_value"];
    this->red = 0;
    this->green = 0;
    this->blue = 0;
  }

  if(doc.containsKey("color_temp")){
    this->colorTemp = doc["color_temp"];
  }

  if(strcmp(doc["state"], "ON") == 0){
    this->state = true;
  }
  else if(strcmp(doc["state"], "OFF") == 0){
    this->state = false;
  }

  Serial.println("");
  Serial.print("Brightness: ");
  Serial.println(this->brightness);
  Serial.print("Red: ");
  Serial.println(this->red);
  Serial.print("Green: ");
  Serial.println(this->green);
  Serial.print("Blue: ");
  Serial.println(this->blue);
  Serial.print("White: ");
  Serial.println(this->white);
  Serial.print("ColorTemp: ");
  Serial.println(this->colorTemp);
  Serial.print("State: ");
  if(this->state){
    Serial.println("ON");
  }
  else if(!(this->state)){
    Serial.println("OFF");
  }
  
}

void HomeAssistant::sendState(PubSubClient CLIENT, int r, int g, int b, int w, int br, int ct, bool st){
  char outputState[300];
  DynamicJsonDocument doc(512);

  doc["brightness"] = br;

  JsonObject color = doc.createNestedObject("color");
  color["r"] = r;
  color["g"] = g;
  color["b"] = b;

  doc["white_value"] = w;
  doc["color_temp"] = ct;

  if(st){
    doc["state"] = "ON";
  }
  else if(!st){
    doc["state"] = "OFF";
  }

  serializeJson(doc, outputState);
  CLIENT.publish(pub_topic, outputState);
}


void HomeAssistant::connectToHA(PubSubClient CLIENT){
  char configMessage[512];
  DynamicJsonDocument doc(512);

  doc["name"] = "LED strip";
  doc["platform"] = "mqtt";
  doc["schema"] = "json";
  doc["stat_t"] = pub_topic;
  doc["cmd_t"] = sub_topic;
  doc["brightness"] = "true";
  doc["rgb"] = "true";
  doc["white_value"] = "true";
  doc["color_temp"] = "true";

  serializeJson(doc, configMessage);
  CLIENT.publish(config_topic, configMessage);
}


void HomeAssistant::setR(int r){
  this->red = r;
}

void HomeAssistant::setG(int g){
  this->green = g;
}

void HomeAssistant::setB(int b){
  this->blue = b;
}

void HomeAssistant::setW(int w){
  this->white = w;
}

void HomeAssistant::setBrightness(int br){
  this->brightness = br;
}

void HomeAssistant::setColorTemp(int ct){
  this->colorTemp = ct;
}

void HomeAssistant::setState(bool st){
  this->state = st;
}

int HomeAssistant::getR(){
  return this->red;
}

int HomeAssistant::getG(){
  return this->green;
}

int HomeAssistant::getB(){
  return this->blue;
}

int HomeAssistant::getW(){
  return this->white;
}

int HomeAssistant::getBrightness(){
  return this->brightness;
}

int HomeAssistant::getColorTemp(){
  return this->colorTemp;
}

bool HomeAssistant::getState(){
  return this->state;
}

#endif

#if DEVICE_TYPE == OUTLET

void HomeAssistant::connectToHA(PubSubClient CLIENT){
  char configMessage[512];
  DynamicJsonDocument doc(512);

  doc["name"] = "Outlet";
  doc["platform"] = "mqtt";
  doc["stat_t"] = pub_topic;
  doc["cmd_t"] = sub_topic;

  serializeJson(doc, configMessage);
  client.publish(config_topic, configMessage);
}

#endif

#if DEVICE_TYPE == SENSOR

void HomeAssistant::connectToHA(PubSubClient CLIENT){
  char temp_configMessage[256];
  char hum_configMessage[256];
  char light_configMessage[256];
  DynamicJsonDocument temp_doc(256);
  DynamicJsonDocument hum_doc(256);
  DynamicJsonDocument light_doc(256);

  temp_doc["dev_cla"] = "temperature";
  temp_doc["name"] = "Teplota";
  temp_doc["stat_t"] = temp_topic;
  temp_doc["unit_of_meas"] = "°C";

  serializeJson(temp_doc, temp_configMessage);
  CLIENT.publish(temp_config, temp_configMessage);

  hum_doc["dev_cla"] = "humidity";
  hum_doc["name"] = "Vlhkost";
  hum_doc["stat_t"] = hum_topic;
  hum_doc["unit_of_meas"] = "%";

  serializeJson(hum_doc, hum_configMessage);
  CLIENT.publish(hum_config, hum_configMessage);
  
  light_doc["name"] = "Light";
  light_doc["stat_t"] = light_topic;
  light_doc["unit_of_meas"] = "%";

  serializeJson(light_doc, light_configMessage);
  CLIENT.publish(light_config, light_configMessage);
}

#endif


#if DEVICE_TYPE == IPS

void HomeAssistant::connectToHA(PubSubClient CLIENT, char* nameX, char* nameY){
  char x_configMessage[256];
  char y_configMessage[256];
  DynamicJsonDocument x_doc(256);
  DynamicJsonDocument y_doc(256);

  x_doc["name"] = nameX;
  x_doc["stat_t"] = this->getPubTopicX();
  serializeJson(x_doc, x_configMessage);
  CLIENT.publish(CONFIG_X, x_configMessage);

  y_doc["name"] = nameY;
  y_doc["stat_t"] = this->getPubTopicY();
  serializeJson(y_doc, y_configMessage);
  CLIENT.publish(CONFIG_Y, y_configMessage);  
}

IPAddress HomeAssistant::findHA(const char* NAME, mDNSResolver::Resolver RESOLVER){

  RESOLVER.setLocalIP(WiFi.localIP());
  IPAddress ip = RESOLVER.search(NAME);
  if(ip != INADDR_NONE){
    this->server_ip = ip;
    return ip;
  }
  else{
    return 0;
  }

}

void HomeAssistant::reconnect(PubSubClient CLIENT){
  while(!CLIENT.connected()){
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    if(CLIENT.connect(clientId.c_str(), mqtt_user, mqtt_password)){
      Serial.println("connected");
      CLIENT.subscribe("IPS/control");
    }
    else{
      Serial.print("failed, rc=");
      Serial.print(CLIENT.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void HomeAssistant::setPubTopicX(char* sideX){
  strncpy(this->pub_topic_x, "IPS/", sizeof(this->pub_topic_x));
  strncat(this->pub_topic_x, sideX, sizeof(this->pub_topic_x));
}

void HomeAssistant::setPubTopicY(char* sideY){
  strncpy(this->pub_topic_y, "IPS/", sizeof(this->pub_topic_y));
  strncat(this->pub_topic_y, sideY, sizeof(this->pub_topic_y));
}

char* HomeAssistant::getPubTopicX(){
  return this->pub_topic_x;
}

char* HomeAssistant::getPubTopicY(){
  return this->pub_topic_y;
}

void HomeAssistant::setIP(IPAddress IP){
  this->server_ip = IP;
}

IPAddress HomeAssistant::getIP(){
  return this->server_ip;
}


#endif