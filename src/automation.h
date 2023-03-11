#include <SPI.h>
#include <GxIO.h>

#define LIGHT_SENSOR_PIN 34   // analog in pin # for a light sensor
#define LED_PIN 32            // pin # of LED controlled by light sensor
#define ANALOG_THRESHOLD 1800 // threshhold for analog input when logical 0 should become logical 1

// RGB LED
#define RGB_RED_PIN 11    // D14
#define RGB_BLUE_PIN 13  // D13
#define RGB_GREEN_PIN 12   // D12
#define RGB_R_PIN 11  // D14
#define RGB_B_PIN 13  // D13
#define RGB_G_PIN 12  // D12

// Define tasks.
TaskHandle_t Task0;
TaskHandle_t Task1, Task2, Task3, Task4, Task5;   // For prototyping purposes these tasks control LEDs based on received command
TaskHandle_t LampTask, StorageCard;

// Define output pins
const int output_2  = 2;//4;    // built-in LED pin 
const int output_1  = 22;//19;  // Pin 19 conflicts with ssd card module
const int output_22 = 22; 
const int output_23 = 21;

void Task0code (void * parameters) {
    Serial.print("Task 1 running on core # ");
    Serial.println(xPortGetCoreID());

    for (;;) {
      digitalWrite(output_1, HIGH);
      vTaskDelay(250);
      digitalWrite(output_1, LOW);
      vTaskDelay(250);
      digitalWrite(output_1, HIGH);
      vTaskDelay(250);
      digitalWrite(output_1, LOW);
      vTaskDelay(1500);                
    }
}