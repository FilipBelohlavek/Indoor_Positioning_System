#include <IPS.h>

unsigned long IndoorPositioningSystem::calibrate(int TRIG_PIN, int ECHO_PIN){
    if(TRIG_PIN == PIN_TRIG)
        Serial.println("ECHO calibration starting.");
    else if(TRIG_PIN == PIN_TRIG2)
        Serial.println("ECHO2 calibratrion starting");

    unsigned long calib = 0;
    int arr[100];

    for(int i = 0; i < 100; i++){
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        arr[i] = pulseIn(ECHO_PIN, HIGH);
        delay(1);        
    }

    this->mergeSort(arr, 0, 99);

    // counting the average value without lowest 10 and highest 10 values
    for(int i = 10; i < 90; i++){
        Serial.println(arr[i]);
        calib = calib + arr[i];
    }

    if(TRIG_PIN == PIN_TRIG){
        Serial.print("Average ECHO: ");
        this->setAverageEcho(calib/160);
        Serial.println(this->getAverageEcho());
    }

    else if(TRIG_PIN == PIN_TRIG2){
        Serial.print("Average ECHO2: ");
        this->setAverageEcho2(calib/160);
        Serial.println(this->getAverageEcho2());
    }


    //Serial.println(calib/160);   /80 /2 -> divide by 2

    return calib/160;
}

int IndoorPositioningSystem::readMovement(unsigned long &TIME, unsigned long &TIME_NOW){

    static bool checkX = false;
    static bool checkY = false;
    int motion = 0;

    if((!this->PIR_State_X != !this->PIR_State_Y) && !(checkX || checkY)){ // if((PIR_State_X XOR PIR_State_Y) AND NOT(check_X OR check_Y))
        if(this->PIR_State_X){    //checking which one is TRUE
            checkX = true;
        }
        else if(this->PIR_State_Y){
            checkY = true;
        }

        TIME = TIME_NOW; 
    }

    if(checkX || checkY){
        if((TIME_NOW - TIME) >= 500){ //counting 500ms time limit
            checkX = false;
            checkY = false;
            return 1; //return 1 if timeout 500ms
        }
        else if(this->PIR_State_X && this->PIR_State_Y){
            if(checkX){
                motion = this->motionCheck(this->average_echo, checkX, checkY);
                Serial.println(motion);
                if(motion){
                    this->side_Y = this->side_Y + 1;
                    if(this->side_X > 0){
                        this->side_X = this->side_X - 1;
                    }
                }
            }
            else if(checkY){
                motion = this->motionCheck(this->average_echo2, checkX, checkY);
                Serial.println(motion);
                if(motion){
                    this->side_X = this->side_X + 1;
                    if(this->side_Y > 0){
                        this->side_Y = this->side_Y - 1;
                    }
                }
            }
            
            delay(TIME_NOW - TIME);

            checkX = false;
            checkY = false;
            return 2;
        }
    }
    return 0;
}

int IndoorPositioningSystem::motionCheck(unsigned long average, bool checkx, bool checky){

    unsigned long echo = 0;

    int TRIG_PIN;
    int ECHO_PIN;

    if(checkx){
        TRIG_PIN = PIN_TRIG;
        ECHO_PIN = PIN_ECHO;
    }
    else if(checky){
        TRIG_PIN = PIN_TRIG2;
        ECHO_PIN = PIN_ECHO2;
    }
    else{
        return -1;
    }
    

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    echo = pulseIn(ECHO_PIN, HIGH);
    echo = echo/2;

    if(TRIG_PIN == PIN_TRIG)
        Serial.print("ECHO: ");
    else if(TRIG_PIN == PIN_TRIG2)
        Serial.print("ECHO2: ");

    Serial.println(echo);

    if(echo > 14500)
        return 2;
    else if(average >= echo){
        echo = average - echo;
    }
    else if(average < echo){
        echo = echo - average;
    }
    
    if(echo > 1000){
        return 1;
    }
    else{
        for(int i = 0; i < 20; i++){
            delay(1);
            digitalWrite(TRIG_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(TRIG_PIN, LOW);

            echo = pulseIn(ECHO_PIN, HIGH);
            echo = echo/2;

            if(TRIG_PIN == PIN_TRIG)
                Serial.print("ECHO: ");
            else if(TRIG_PIN == PIN_TRIG2)
                Serial.print("ECHO2: ");

            Serial.println(echo);

            if(echo > 14500){
                return 2;
            }
            else if(average >= echo){
                echo = average - echo;
            }
            else if(average < echo){
                echo = echo - average;
            }
    
            if(echo > 600){
                return 1;
            }
        }
        return 0;
    }

}

void IndoorPositioningSystem::setSideX(unsigned int x){
    this->side_X = x;
}

unsigned int IndoorPositioningSystem::getSideX(){
    return this->side_X;
}

void IndoorPositioningSystem::setSideY(unsigned int y){
    this->side_Y = y;
}

unsigned int IndoorPositioningSystem::getSideY(){
    return this->side_Y;
}

void IndoorPositioningSystem::setAverageEcho(unsigned long ae){
    this->average_echo = ae;
}

unsigned long IndoorPositioningSystem::getAverageEcho(){
    return this->average_echo;
}

void IndoorPositioningSystem::setAverageEcho2(unsigned long ae2){
    this->average_echo2 = ae2;
}

unsigned long IndoorPositioningSystem::getAverageEcho2(){
    return this->average_echo2;
}

char* IndoorPositioningSystem::getXName(){
    return this->x_name;
}

char* IndoorPositioningSystem::getYName(){
    return this->y_name;
}

void IndoorPositioningSystem::setupDevice(){
    WiFiManager wm;

    bool empty = false;

    //setup custom parameters in WifiManager
    WiFiManagerParameter custom_x_name("x", "X side name", this->x_name, sizeof(this->x_name));
    WiFiManagerParameter custom_y_name("y", "Y side name", this->y_name, sizeof(this->y_name));

    wm.addParameter(&custom_x_name);
    wm.addParameter(&custom_y_name);

    //SPIFFS
    Serial.println("Mounting FS...");
    if(SPIFFS.begin()){
        Serial.println("File system mounted,");
        File file = SPIFFS.open("/sides.json", "r");
        if(file){
            Serial.println("File opened.");
            size_t size = file.size();
            std::unique_ptr<char[]> buffer(new char[size]);
            file.readBytes(buffer.get(), size);
            
            DynamicJsonDocument read_doc(128);
            if(deserializeJson(read_doc, buffer.get()) == DeserializationError::Ok){
                strncpy(this->x_name, read_doc["x"], sizeof(this->x_name));
                strncpy(this->y_name, read_doc["y"], sizeof(this->y_name));
                Serial.printf("X side: %s\n", this->x_name);
                Serial.printf("Y side: %s\n", this->y_name);
                wm.autoConnect("IPS - setup");
            }
            else{
                Serial.println("Deserialization failed,");
                empty = true;
            }
        }
        else{
            Serial.println("Failed to open file.");
            empty = true;
        }
        file.close();
    }
    else{
        Serial.println("Failed to mount FS.");
    }

    if(empty){
        Serial.println("Starting config...");
        //start config portal
        wm.startConfigPortal("IPS - setup");
        Serial.println();
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        //get custom parameters
        //strncpy -> buffer overflow protection
        strncpy(this->x_name, custom_x_name.getValue(), sizeof(this->x_name));
        strncpy(this->y_name, custom_y_name.getValue(), sizeof(this->y_name));

        File file2 = SPIFFS.open("/sides.json", "w");
        if(file2){
            Serial.println("File2 opened.");
            Serial.printf("X side: %s\n", this->x_name);
            Serial.printf("Y side: %s\n", this->y_name);
            
            DynamicJsonDocument write_doc(128);

            write_doc["x"] = this->x_name;
            write_doc["y"] = this->y_name;

            serializeJson(write_doc, file2);
        }
        else{
            Serial.println("Failed to open file for writing!!!");
        }
        Serial.println("Closing file...");
        file2.close();
    }
}

void IndoorPositioningSystem::openConfig(){

    WiFiManager wm;

     //setup custom parameters in WifiManager
    WiFiManagerParameter custom_x_name("x", "X side name", this->x_name, 20);
    WiFiManagerParameter custom_y_name("y", "Y side name", this->y_name, 20);

    wm.addParameter(&custom_x_name);
    wm.addParameter(&custom_y_name);

    Serial.println("Starting config...");
    //start config portal
    wm.startConfigPortal("ILS - setup");
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //get custom parameters
    //strncpy -> buffer overflow protection
    strncpy(this->x_name, custom_x_name.getValue(), sizeof(this->x_name));
    strncpy(this->y_name, custom_y_name.getValue(), sizeof(this->y_name));

    File file2 = SPIFFS.open("/sides.json", "w");
    if(file2){
        Serial.println("File2 opened.");
        Serial.printf("X side: %s\n", this->x_name);
        Serial.printf("Y side: %s\n", this->y_name);
         
        DynamicJsonDocument write_doc(128);

        write_doc["x"] = this->x_name;
        write_doc["y"] = this->y_name;

        serializeJson(write_doc, file2);
    }
    else{
        Serial.println("Failed to open file for writing!!!");
    }
    file2.close();
}



// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
void IndoorPositioningSystem::merge(int arr[], int l, int m, int r) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 
  
    /* create temp arrays */
    int L[n1], R[n2]; 
  
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1+ j]; 
  
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray 
    j = 0; // Initial index of second subarray 
    k = l; // Initial index of merged subarray 
    while (i < n1 && j < n2) 
    { 
        if (L[i] <= R[j]) 
        { 
            arr[k] = L[i]; 
            i++; 
        } 
        else
        { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    /* Copy the remaining elements of L[], if there 
       are any */
    while (i < n1) 
    { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    /* Copy the remaining elements of R[], if there 
       are any */
    while (j < n2) 
    { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 
} 
  
/* l is for left index and r is right index of the 
   sub-array of arr to be sorted */
void IndoorPositioningSystem::mergeSort(int arr[], int l, int r) 
{ 
    if (l < r) 
    { 
        // Same as (l+r)/2, but avoids overflow for 
        // large l and h 
        int m = l+(r-l)/2; 
  
        // Sort first and second halves 
        this->mergeSort(arr, l, m); 
        this->mergeSort(arr, m+1, r); 
  
        this->merge(arr, l, m, r); 
    } 
} 