#include <SPI.h>
#include <GxIO.h>
#include <WiFi.h>
//#include "secrets.h"

// Uncomment corresponding board
//#define devkit_30pin_001
#define devkit_36pin_001

/*
#### ESP32 DEVKIT V1.1 DIY MALL           ####
#### 30 PINS                              ####
#### DEVELOPMENT BOARD SUPPORTED PIN OUTS ####
*/
#ifdef devkit_30pin_001
#define PING_PIN 33           // D33 pin # of audio ping
#define LED_PIN 32            // pin # of LED controlled by light sensor
#endif
/*
##############################################
*/
/*
#### ESP32 DEVKIT V1.1 DIY MALL           ####
#### 36 PINS                              ####
#### DEVELOPMENT BOARD SUPPORTED PIN OUTS ####
----------------------------------------------
Nominal | Physical  |   Description
Pin     |           |
----------------------------------------------
              2         Built-in LED
D0            5
D15           6
D2            7         Same as built-in LED
D4            8
D5            11
D18           12        Not supported
D19           13        Not supported
D21           14        Not supported
D22           17        Not supported
D23           18        Not supported
D34           22        Not supported
D35           23        Not supported
D32           24        Not supported
D33           25
D25           26
D26           27
D27           28
D17           29
D14           30
D12           31
D13           32
----------------------------------------------
*/
#ifdef devkit_36pin_001
#define LED       2
#define D0        5
#define D15       6
#define PING_PIN 33           // D33 pin # of audio ping
#define LED_PIN 32            // pin # of LED controlled by light sensor
#endif
/*
##############################################
*/

#define LIGHT_SENSOR_PIN 34   // analog in pin # for a light sensor
#define LED_PIN 32            // pin # of LED controlled by light sensor
#define ANALOG_THRESHOLD 1800 // threshhold for analog input when logical 0 should become logical 1



// RGB LED
//#define RGB_RED_PIN 11    // D14
//#define RGB_BLUE_PIN 13  // D13
//#define RGB_GREEN_PIN 12   // D12
#define RGB_R_PIN 11  // D14
#define RGB_B_PIN 13  // D13
#define RGB_G_PIN 12  // D12

#define SWITCH_1 33   // GPIO 33; PHYSICAL PIN # 3
#define SWITCH_2 27   // GPIO 27; PHYSICAL PIN # 10
//#define SWITCH_1 7  PIN 7 SAUSES STALLS

/*#define ESP_INR_FLAG_DEFAULT 0
#define LED_PIN_OVERRIDE 27
#define PUSH_BUTTON_PIN 33*/




// Define tasks.
TaskHandle_t Task0, TaskLed;     // Dummy built-in LED blink cycle
TaskHandle_t TaskWiFi;  // Task maintaining wireless connection
TaskHandle_t TaskSd;    // Task to write sensors vlues to a file stored on SD card
TaskHandle_t Task1, Task2, Task3, Task4, Task5;   // For prototyping purposes these tasks control LEDs based on received command
TaskHandle_t LampTask, StorageCard, Connection;

// Interrupt for push button
TaskHandle_t LED_OVERRIDE = NULL;

//WiFiClientSecure wifi_conn = WiFiClientSecure();

// Define output pins
const int output_2  = 2;//4;    // built-in LED pin 
const int output_1  = 22;//19;  // Pin 19 conflicts with ssd card module
const int output_22 = 22; 
const int output_23 = 21;

bool led_status = false;

/*void IRAM_ATTR button_isr_handler () {
  xTaskResumeFromISR(LED_OVERRIDE);
}
void interrupt_task (void* arg) {
  led_status = false;
  while (true) {
    vTaskSuspend(NULL);
    led_status = !led_status;
    //gpio_set_level(LED_PIN_OVERRIDE, led_status);
    digitalWrite(LED_PIN_OVERRIDE, led_status);
  }
}*/

void mqtt_message_handler (char* topic, byte* message, unsigned int length)
{
  Serial.print("\nMessage arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i=0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Automation. Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on\n");
      //digitalWrite(SWITCH_2, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off\n");
      //digitalWrite(SWITCH_2, LOW);
    }
  }
}

// Dummy task blinking built-in LED
void Task0code (void * parameters) {
    Serial.print("Task 0 running on core # ");
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
// Dummy task for breathing LED
void TaskLedCode (void * parameters) {
    int *sensor = (int*)parameters;

    Serial.print("Task LED running on core # ");
    Serial.println(xPortGetCoreID());
    Serial.print("Value passed to a task: ");
    Serial.println(*(int*)parameters);

    for (;;) {
      for (int d = 20; d <= 255; d++) {
        ledcWrite(0, d);
        vTaskDelay(25/portTICK_RATE_MS);
      }
      for (int d = 255; d >= 20; d--) {
        ledcWrite(0, d);
        vTaskDelay(25/portTICK_RATE_MS);
      }             
    }
}



/*void TaskWiFiCode (void* parameters)
{
  for (;;) {
    WiFi.mode(WIFI_STA);
    String hostname = "ESP32LF";
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi ...");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("\nCONNECTED\nIP: ");
    Serial.println(WiFi.localIP());
    vTaskSuspend(NULL);
  }
}*/