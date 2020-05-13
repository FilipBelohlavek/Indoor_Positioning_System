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

unsigned long reset_time_now = 0;
unsigned long reset_time = 0;

int test = 0;

int sonic_test = 0;
bool timer_check = false;
bool sonic_flag = false;
bool check_flag = false;

int prev_PIR_State_X = 0;
int prev_PIR_State_Y = 0;

char side_X_str[4];
char side_Y_str[4];

//________________________________________________________________________

void callback(char* topic, byte* payload, unsigned int length);
void deleteMessage();

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

    HA.setConfigTopicX(ips.getXName());
    HA.setConfigTopicY(ips.getYName());
    HA.setPubTopicX(ips.getXName());
    HA.setPubTopicY(ips.getYName());

    Serial.println("Topics set.");

    IPAddress serverIP = HA.findHA(MDNS_NAME, resolver);

    Serial.println("HA IPAddress found.");

    client.setServer(serverIP, 1883);
    client.setCallback(callback);



    digitalWrite(PIN_TRIG, LOW);
    digitalWrite(PIN_TRIG2, LOW);
    digitalWrite(LED_PIN, HIGH);
    delayMicroseconds(10);

    Serial.println("Calibration starting...");

    for(int i = 0; i < 3; i++){
        ips.calibrate(PIN_TRIG, PIN_ECHO);
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

    if(!sonic_flag)
        time_now = millis();
    
    
    ips.readPIRs(PIR_PIN_X, PIR_PIN_Y);
    

    if((prev_PIR_State_X != ips.getPIRStateX()) || (prev_PIR_State_Y != ips.getPIRStateY())){
        Serial.printf("PIR State X: %d\nPIR State Y: %d\n", ips.getPIRStateX(), ips.getPIRStateY());
        Serial.println();

        if(ips.getPIRStateX() || ips.getPIRStateY()){
            digitalWrite(LED_PIN, LOW);

            timer_check = false;
        }
        else{
            Serial.println("---------------");
            digitalWrite(LED_PIN, HIGH);

            if(sonic_flag){
                reset_time = millis();
                Serial.println("Timer RESET");
                timer_check = true;
            }
        }
    }


    if(!sonic_flag){
        test = ips.readMovement(Time, time_now);
    }

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
        sonic_flag = true;
        check_flag = false;
        Serial.println("sonic_flag: TRUE");
        test = 0;
        delay(1000);

    }
    else if(test == 1){
        Serial.println("Timeout error.");
        sonic_flag = true;
        check_flag = false;
        Serial.println("sonic_flag: TRUE");
        test = 0;
        delay(1000);
    }

    if(timer_check){ //counting only when PIRs are LOW, when sonic_flag = true
        reset_time_now = millis();
        Serial.println("Timer_now UPDATED");
    }

    if((reset_time_now - reset_time >= 600) && !check_flag){
        sonic_flag = false;
        check_flag = true;
        timer_check = false;
        reset_time_now = 0;
        reset_time = 0;
        Serial.println("sonic_flag: FALSE");
    }

    if(sonic_flag){
        sonic_test = ips.sonicReadMovement();
        if(sonic_test == 2){
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

            delay(1000);
        }
        else if(sonic_test == 1){
            Serial.println("Timeout error.");
        }
    }


    if(!sonic_flag){

        if(strcmp(HA.incomingMessage, "config") == 0){
            deleteMessage();  
            ips.openConfig();     
        }
        else if((HA.incomingMessage[0] == 'x') || (HA.incomingMessage[0] == 'X')){
            long buffer_int;

            buffer_int = strtol(HA.incomingMessage, NULL, 10);
            if(buffer_int > 99){
                Serial.println("Invalid input, maximum value = 99");
            }
            else{
                ips.setSideX(buffer_int);
                sprintf(side_X_str, "%d", ips.getSideX());
                client.publish(HA.getPubTopicX(), side_X_str);
            }
            deleteMessage();
        }
        else if((HA.incomingMessage[0] == 'y') || (HA.incomingMessage[0] == 'Y')){
            long buffer_int;

            buffer_int = strtol(HA.incomingMessage, NULL, 10);
            if(buffer_int > 99){
                Serial.println("Invalid input, maximum value = 99");
            }
            else{
                ips.setSideY(buffer_int);
                sprintf(side_Y_str, "%d", ips.getSideY());
                client.publish(HA.getPubTopicY(), side_Y_str);
            }
            deleteMessage();
        }
    }


    prev_PIR_State_X = ips.getPIRStateX();
    prev_PIR_State_Y = ips.getPIRStateY();
}




//________________________________________________________________________

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length){

    unsigned int len;

    deleteMessage();

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for(unsigned int i = 0; i < length; i++){
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if(length > sizeof(HA.incomingMessage))
        len = sizeof(HA.incomingMessage);
    else
        len = length;

    for(unsigned int i = 0; i < len; i++){
        HA.incomingMessage[i] = (char)payload[i];
    }

    if(strcmp(topic, HA.getPubTopicX()) == 0){
        unsigned long buffer;
        buffer = strtol(HA.incomingMessage, NULL, 10);
        ips.setSideX(buffer);
    }
    else if(strcmp(topic, HA.getConfigTopicY()) == 0){
        unsigned long buffer;
        buffer = strtol(HA.incomingMessage, NULL, 10);
        ips.setSideY(buffer);
    }
}

//delete message
void deleteMessage(){
    for(unsigned int i = 0; i < sizeof(HA.incomingMessage); ++i)
        HA.incomingMessage[i] = (char)0;
}


//Filip Belohlavek 2020