#include <Arduino.h>
#include <HomeAssistant.h>
#include <IPS.h>

WiFiClient espIPS;
PubSubClient client(espIPS);

WiFiUDP udp;
mDNSResolver::Resolver resolver(udp);

HomeAssistant HA;
IndoorPositioningSystem ips;

//________________________________________________________________________

unsigned long time_now = 0;
unsigned long Time = 0;

int test = 0;

char side_X_str[4];
char side_Y_str[4];

//________________________________________________________________________

void callback(char* topic, byte* payload, unsigned int length);

//________________________________________________________________________

void setup(){

    pinMode(PIR_PIN_X, INPUT);
    pinMode(PIR_PIN_Y, INPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_TRIG2, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_ECHO2, INPUT);
    pinMode(LED_PIN, OUTPUT);

    delay(10);
    Serial.begin(115200);
    Serial.println();

    ips.setupDevice();

    Serial.println("setupDevice() ended.");

    HA.setPubTopicX(ips.getXName());
    HA.setPubTopicY(ips.getYName());

    Serial.println("Topics set.");

    IPAddress serverIP = HA.findHA(MDNS_NAME, resolver);

    Serial.println("HA IPAddress found.");

    client.setServer(serverIP, 1883);
    client.setCallback(callback);



    digitalWrite(PIN_TRIG, LOW);
    digitalWrite(PIN_TRIG2, LOW);
    delayMicroseconds(10);

    Serial.println("Calibration starting...");

    for(int i = 0; i < 3; i++){
        //average_echo = calibrate(PIN_TRIG, PIN_ECHO);
        ips.calibrate(PIN_TRIG, PIN_ECHO);
        //average_echo2 = calibrate(PIN_TRIG2, PIN_ECHO2);
        ips.calibrate(PIN_TRIG2, PIN_ECHO2);
        if((ips.getAverageEcho() < 10000) && (ips.getAverageEcho2() < 10000)){
            Serial.println("READY");
            break;
        }
        else if((i == 2) && ((ips.getAverageEcho() > 10000) || (ips.getAverageEcho2() > 10000))){
            Serial.println("There's a problem in your setup. This device might not function properly.");
        }
        else{
            Serial.println("Calibration unsuccesful. Trying again.");
        }

    }
    

}


//________________________________________________________________________

void loop(){

    if(!client.connected()){
        HA.reconnect(client);
        HA.connectToHA(client, ips.getXName(), ips.getYName());
    }

    client.loop();
    resolver.loop();

    time_now = millis();
    
    ips.readPIRs(PIR_PIN_X, PIR_PIN_Y);

    if(ips.getPIRStateX() || ips.getPIRStateY()){
        digitalWrite(LED_PIN, LOW);
    }
    else{
        digitalWrite(LED_PIN, HIGH);
    }

    test = ips.readMovement(Time, time_now);

    if(test == 2){
        Serial.print("Side X: ");
        Serial.println(ips.getSideX());
        Serial.print("Side Y: ");
        Serial.println(ips.getSideY());
        Serial.println();

        sprintf(side_X_str, "%d", ips.getSideX());
        sprintf(side_Y_str, "%d", ips.getSideY());

        client.publish(HA.getPubTopicX(), side_X_str);
        delay(10);
        client.publish(HA.getPubTopicY(), side_Y_str);
    }
    else if(test == 1){
        Serial.println("Timeout error.");
    }

    if(strcmp(HA.incomingMessage, "config") == 0){
        ips.openConfig();

        for(unsigned int i = 0; i < sizeof(HA.incomingMessage); ++i)
            HA.incomingMessage[i] = (char)0;        
    }
}


//________________________________________________________________________

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length){

    for(unsigned int i = 0; i < sizeof(HA.incomingMessage); ++i)
        HA.incomingMessage[i] = (char)0;

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for(unsigned int i = 0; i < length; i++){
        Serial.print((char)payload[i]);
    }
    Serial.println();

    for(unsigned int i = 0; i < length; i++){
        HA.incomingMessage[i] = (char)payload[i];
    }
}