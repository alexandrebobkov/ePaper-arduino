#ifndef AUTOMATION_H
#define AUTOMATION_N

#include <SD.h>

class Automation {
    private:
        // Define output pins
        const int output_2  = 2;//4;    // built-in LED pin 
        const int output_1  = 22;//19;  // Pin 19 conflicts with ssd card module
        const int output_22 = 22; 
        const int output_23 = 21;

    public:
        Automation();

        void SetOutput(int pin, boolean level);

};
#endif