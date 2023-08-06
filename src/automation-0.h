#include <SPI.h>
#include <GxIO.h>
#include <WiFi.h>





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