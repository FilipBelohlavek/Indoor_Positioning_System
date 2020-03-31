#ifndef HomeAssistant_h
#define HomeAssistant_h

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <StreamString.h>
#include <mDNSResolver.h>

#define RGBW 1
#define OUTLET 2
#define SENSOR 3
//#define HUMIDIFIER 4
//#define WINDOW 5
#define IPS 6

/*
 * Define MQTT broker parameters here
 */
#define mqtt_user "mqtt"
#define mqtt_password "mqtt"
#define MDNS_NAME "hassio.local"

/*
 * Define device type here
 */
//#define DEVICE_TYPE RGBW
//#define DEVICE_TYPE OUTLET
//#define DEVICE_TYPE SENSOR
//#define DEVICE_TYPE HUMIDIFIER
//#define DEVICE_TYPE WINDOW
#define DEVICE_TYPE IPS  /* Indoor_Positioning_System*/


#if DEVICE_TYPE == RGBW
  #define INCOMING_PACKET_BUFFER_SIZE 250
  #define pub_topic "light/rgbLEDstrip"
  #define sub_topic "control/rgbLEDstrip"
  #define config_topic "homeassistant/light/LEDstrip/config"
  
  class HomeAssistant{
  
    private:
      int red = 0;
      int green = 0;
      int blue = 0;
      int white = 0;
      int brightness = 0;
      int colorTemp = 0;
      bool state = false;
      
    public:
      void processJson(char* incomingMessage);
      void sendState(PubSubClient CLIENT, int r, int g, int b, int w, int br, int ct, bool st);
      void connectToHA(PubSubClient CLIENT);
      
      void setR(int r);
      void setG(int g);
      void setB(int b);
      void setW(int w);
      void setBrightness(int br);
      void setColorTemp(int ct);
      void setState(bool st);
      int getR();
      int getG();
      int getB();
      int getW();
      int getBrightness();
      int getColorTemp();
      bool getState();
};

/*WiFiClient espRGBW;
PubSubClient client(espRGBW);*/
#endif

#if DEVICE_TYPE == OUTLET
  #define pub_topic "relay/outlet1"
  #define sub_topic "control/outlet1"
  #define config_topic "homeassistant/switch/outlet/config"

  class HomeAssistant{
    private:

    public:
      void connectToHA(PubSubClient CLIENT);
  };
#endif

#if DEVICE_TYPE == SENSOR
  #define temp_topic "sensor/temp"
  #define hum_topic "sensor/hum"
  #define light_topic "sensor/light"
  #define temp_config "homeassistant/sensor/temperature/config"
  #define hum_config "homeassistant/sensor/humidity/config"
  #define light_config "homeassistant/sensor/light/config"

  class HomeAssistant{
    private:

    public:
      void connectToHA(PubSubClient CLIENT);
  };
 #endif


#ifndef INCOMING_PACKET_BUFFER_SIZE
#define INCOMING_PACKET_BUFFER_SIZE 50
#endif

#if DEVICE_TYPE == IPS

//has to be configured
  #define CONFIG_X "homeassistant/sensor/room_x/config"
  #define CONFIG_Y "homeassistant/sensor/room_y/config"

  class HomeAssistant{
    private:
      char pub_topic_x[35];
      char pub_topic_y[35];
      IPAddress server_ip;
      

    public:
      char incomingMessage[INCOMING_PACKET_BUFFER_SIZE];      

      //configures device in HomeAssistant
      void connectToHA(PubSubClient CLIENT, char* nameX, char* nameY);

      //finds HA IP address on local network
      IPAddress findHA(const char* NAME, mDNSResolver::Resolver RESOLVER);

      //reconnects to HA and to MQTT Broker
      void reconnect(PubSubClient CLIENT);

      //sets pub_topic_x to ILS/"nameX"
      void setPubTopicX(char* sideX);

      //sets pub_topic_y to ILS/"nameY"
      void setPubTopicY(char* sideY);

      //returns pub_topic_x
      char* getPubTopicX();

      //returns pub_topic_y
      char* getPubTopicY();

      //sets HomeAssistant IP address
      void setIP(IPAddress IP);

      //returns HomeAssistant IP address
      IPAddress getIP();


       
  };

#endif


#endif
