#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <SD.h>

class Automation {
    private:
        // Define output pins
        const int output_2  = 2;    // built-in LED pin 
        const int output_1  = 22;   // Pin 19 conflicts with ssd card module
        const int output_22 = 22; 
        const int output_23 = 21;

        const int switch_1 = 33;      // GPIO 33; PHYSICAL PIN # 3
        const int switch_2 = 27;      // GPIO 27; PHYSICAL PIN # 10

        TaskHandle_t idle_task;

    public:
        Automation();
        void init();
        void SetOutput(int pin, boolean level);

};
#endif