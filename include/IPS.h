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
        bool reset = false;

    public:
        //calibrates HC-SR04 sensor: returns average echo value from 100 measurements
        unsigned long calibrate(int TRIG_PIN, int ECHO_PIN);

        //used in: mergeSort()
        void merge(int arr[], int l, int m, int r);

        //used in: calibrate()
        void mergeSort(int arr[], int l, int r);

        //reads direction of movement
        int readMovement(unsigned long &TIME, unsigned long &TIME_NOW);

        //used in: readMovement(), checks if movement is present with HC-SR04 ultrasonic sensor
        int motionCheck(unsigned long average, int TRIG_PIN, int ECHO_PIN, bool sonic);

        //uses HC-SR04 to read direction of movement
        int sonicReadMovement();

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
        void readPIRs(int pir_pin_x, int pir_pin_y);

        //returns PIR_State_X
        int getPIRStateX();

        //returns PIR_State_Y
        int getPIRStateY();
        
        //returns 1 if signal reflected, returns 2 if signal didn't reflect, returns 0 if reading is average
        int readHCSR04(int TRIG_PIN, int ECHO_PIN);
};



#endif


//Filip Belohlavek 2020