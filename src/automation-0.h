#include <SPI.h>
#include <GxIO.h>

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




// Define tasks.
TaskHandle_t Task0;     // Dummy built-in LED blink cycle
TaskHandle_t TaskWiFi;  // Task maintaining wireless connection
TaskHandle_t Task1, Task2, Task3, Task4, Task5;   // For prototyping purposes these tasks control LEDs based on received command
TaskHandle_t LampTask, StorageCard;

WiFiClientSecure wifi_conn = WiFiClientSecure();

    // Define output pins
    const int output_2  = 2;//4;    // built-in LED pin 
    const int output_1  = 22;//19;  // Pin 19 conflicts with ssd card module
    const int output_22 = 22; 
    const int output_23 = 21;



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

void TaskWiFiCode (void* parameters)
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
}
