/*
Indoor Positioning System library
*/

#ifndef IPS_h
#define IPS_h

#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#define PIR_PIN_X D7
#define PIR_PIN_Y D1
#define PIN_TRIG D6
#define PIN_TRIG2 D8
#define PIN_ECHO D5
#define PIN_ECHO2 D2
#define LED_PIN D4

class IndoorPositioningSystem{

    private:
        unsigned int side_X = 0;
        unsigned int side_Y = 0;
        int PIR_State_X = 0;
        int PIR_State_Y = 0;
        unsigned long average_echo = 0;
        unsigned long average_echo2 = 0;
        char x_name[30] = "X side";
        char y_name[30] = "Y side";

    public:
        //calibrates HC-SR04 sensor: returns average echo value from 100 measurements
        unsigned long calibrate(int TRIG_PIN, int ECHO_PIN);

        //used in: mergeSort()
        void merge(int arr[], int l, int m, int r);

        //used in: calibrate()
        void mergeSort(int arr[], int l, int r);

        //Senses direction of movement
        int readMovement(unsigned long &TIME, unsigned long &TIME_NOW);

        //used in: readMovement()
        int motionCheck(unsigned long average, bool checkx, bool checky);

        //Initial automatic device setup, side names, WiFi credentials
        void setupDevice();
        
        //On demand device setup, side names, WiFi credentials
        void openConfig();

        void setSideX(unsigned int x);

        //reads how many people are on side X
        unsigned int getSideX();

        void setSideY(unsigned int y);

        //reads how many people are on side Y
        unsigned int getSideY();

        //used in: calibrate()
        void setAverageEcho(unsigned long ae);

        //returns AverageEcho
        unsigned long getAverageEcho();

        //used in: calibrate()
        void setAverageEcho2(unsigned long ae2);

        //returns AverageEcho2
        unsigned long getAverageEcho2();

        //returns name of X side
        char* getXName();

        //returns name of Y side
        char* getYName();
        
        //reads PIR outputs and saves them
        void readPIRs(int pir_pin_x, int pir_pin_y){
            this->PIR_State_X = digitalRead(pir_pin_x);
            this->PIR_State_Y = digitalRead(pir_pin_y);
        }

        //returns PIR_State_X
        int getPIRStateX(){
            return this->PIR_State_X;
        }

        //returns PIR_State_Y
        int getPIRStateY(){
            return this->PIR_State_Y;
        }
};



#endif