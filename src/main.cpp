/*
  Written for board Espressive ESP32-WROOM-32
    
  ePaper embeded system program written in style adopted for learning.
  Adopted & written by: Alexander Bobkov
  Feb 25, 2023
*/

#include "secrets.h"
#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "WiFi.h"
#include <WiFi.h>
#include <Wire.h> 
#include <GxEPD.h>
#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
//#include "GxIO.h"

/*
#include "secrets.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
// Adopted libraries
//#include <GxGDEW042Z15.h>
#include "GxGDEW042Z15.h"                 // ePaper WaveShare screen 4.2" b/w/r
#include "GxIO.h"
#include "GxIO_SPI.h"

//#include <GxGDEW042Z15.h>                 // ePaper WaveShare screen 4.2" b/w/r
//#include <GxIO.h>
//#include <GxIO_SPI.h>

#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "WiFi.h"
#include <WiFi.h>
#include <Wire.h> 
#include <GxEPD.h>
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
//#include <GxIO/GxIO_SPI/GxIO_SPI.h>
//#include <GxIO/GxIO.h>
*/

#if defined(ESP8266)
GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ 4); // default selection of D4(=2), D2(=4)

#elif defined(ESP32)
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

#elif defined(ARDUINO_ARCH_SAMD)
GxIO_Class io(SPI, /*CS=*/ 4, /*DC=*/ 7, /*RST=*/ 6);
GxEPD_Class display(io, /*RST=*/ 6, /*BUSY=*/ 5);

#elif defined(ARDUINO_GENERIC_STM32F103C) && defined(MCU_STM32F103C8)
GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 3, /*RST=*/ 2);
GxEPD_Class display(io, /*RST=*/ 2, /*BUSY=*/ 1);

#elif defined(ARDUINO_GENERIC_STM32F103V) && defined(MCU_STM32F103VB)
GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ PE15, /*RST=*/ PE14); // DC, RST as wired by DESPI-M01
GxEPD_Class display(io, /*RST=*/ PE14, /*BUSY=*/ PE13); // RST, BUSY as wired by DESPI-M01

#elif defined(ARDUINO_AVR_MEGA2560)

// select one, depending on your CS connection
//GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
//GxIO_Class io(SPI, /*CS=*/ 10, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9, CS on 10 (for CS same as on UNO, for SPI on ICSP use)
GxEPD_Class display(io, /*RST=*/ 9, /*BUSY=*/ 7); // default selection of (9), 7
#else

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=*/ 9, /*BUSY=*/ 7); // default selection of (9), 7
#endif

#define AWS_IOT_CHANNEL_1 "iot/ch1"
#define AWS_IOT_CHANNEL_2 "iot/ch2"
#define AWS_IOT_CHANNEL_3 "iot/ch3"
#define AWS_IOT_CHANNEL_4 "iot/ch4"
#define AWS_IOT_CHANNEL_5 "iot/ch5"

#define LIGHT_SENSOR_PIN 34   // analog in pin # for a light sensor
#define LED_PIN 32            // pin # of LED controlled by light sensor
#define ANALOG_THRESHOLD 2050 // threshhold for analog input when logical 0 should become logical 1

struct Data {
  const char* temp;
};

// Define tasks.
TaskHandle_t Task1, Task2, Task3, Task4, Task5;   // For prototyping purposes these tasks control LEDs based on received command

// Define output pins
const int output_2 = 2;//4;   // built-in LED pin #
// output pins that will be used to control relay; for now they control LEDs
const int output_1 = 19;
const int output_22 = 22;   // Pin 22
const int output_23 = 21;

int sensor_val = 0;

char aws_msg[25] = "";
char info_ip_addr[16] = "010.100.050.000";
char display_msg[4][50] = {"", "", "", ""};

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Section of code that processes JSON command(s) received from AWS IoT
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Listening: ");
  Serial.println(topic);
 
 // Controlls dummy task to blink built-in LED
  if (strstr(topic, "iot/ch1")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Channel_1 = doc["status"];
    int ch1 = Channel_1.toInt();
    if(ch1==1) {
      Serial.println("Channel 1: Message received.");
      Serial.println("Channel 1: Status = 1; LED task resume.");  
      vTaskResume(Task2);
    }   
    else if(ch1==0) {
      Serial.println("Channel 1: Message received.");
      Serial.println("Channel 1: Status = 0; LED task suspend.");   
      vTaskSuspend(Task2);  
    }     
  }
  // LED Table Lamp; active LOW
  if (strstr(topic, "iot/ch2")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Channel_2 = doc["status"];
    int ch2 = Channel_2.toInt();
    if(ch2==1) {
      Serial.print("Channel 2: 1");
      if (digitalRead(LED_PIN))         // Turn table lights on only if anbient light is low
        digitalWrite(output_22, LOW);
    }
    else if(ch2==0) {
      Serial.print("Channel 2: 0");   
      digitalWrite(output_22, HIGH);  
    }
  }
  if (strstr(topic, "iot/ch3")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Channel_3 = doc["status"];
    int ch3 = Channel_3.toInt();
    if(ch3==1) {
      Serial.print("Channel 3: 1");  
      vTaskResume(Task1);  
    } 
    else if(ch3==0) {
      Serial.print("Channel 3: 0");  
      vTaskSuspend(Task1);  
    } 
  }
  if (strstr(topic, "iot/ch4")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Channel_4 = doc["status"];
    String Channel_4_msg = doc["message"];
    String Channel_4_ip_addr = doc["ip_addr"];
    int ch4 = Channel_4.toInt();
    if(ch4==1) {
      Serial.println("R4 is ON");
      vTaskResume(Task3);
      Serial.print("Resumed task");
      Serial.println(Channel_4_msg);
      Serial.println(Channel_4_msg);
      Serial.println(Channel_4_ip_addr);
    } 
    else if(ch4==0) {
      Serial.println("R4 is OFF");
      Serial.print("Paused task");
      Serial.println(Channel_4_msg);
    }     
  }
  if (strstr(topic, "iot/ch5")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Channel_5_status = doc["status"];
    int ch5 = Channel_5_status.toInt();
    if(ch5==1) {
      Serial.println("R5 is ON");
      vTaskResume(Task4);
      Serial.print("Resumed task");
      Serial.println(Channel_5_status);
    }     
    else if(ch5==0)
      Serial.print("Channel 5: 0");     
  }
}

void Task1code (void * parameters) {
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

// Dummy task. Runs to blink built-in LED. Indicates that board has started
void Task2code (void * parameters) {  
  Serial.print("Task 2 running on core # ");
    Serial.println(xPortGetCoreID());

    for (;;) {
      // Blinkpattern: 3 quick flashes, pause
      // Task runs forever unless paused/terminated from outside
      // vTaskDelay() to be used as opposed to Delay()
      digitalWrite(output_2, HIGH);
      vTaskDelay(125);
      digitalWrite(output_2, LOW);
      vTaskDelay(125);
      digitalWrite(output_2, HIGH);
      vTaskDelay(125);
      digitalWrite(output_2, LOW);
      vTaskDelay(125);
      digitalWrite(output_2, HIGH);
      vTaskDelay(125);
      digitalWrite(output_2, LOW);
      vTaskDelay(1500); 
      Serial.print("Sensor value: ");
      //client.publish(AWS_IOT_CHANNEL_5, "0");
      Serial.println(analogRead(LIGHT_SENSOR_PIN));  
    }
} 

void showUpdate(char ip[], const char text[], const GFXfont* f) {
  const char header[25] = "Networks IV\n"; 
  //const char ip[25] = "IP: 10.100.50.20";
  const char ip_addr[] = "121.21.10.20";
  //const char footer[] = "\nWireless\nAutomation Board\n\nControlled via Cloud";
  const char footer[] = "\nAutomation Board";
  const char message[] = "Command received:\nRelay 1 ON";
  //strcpy(ip, "10.100.50.16");
  
  //display.updateWindow(70,20,300,400,false);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(80, 20);
  display.println(header);
  //display.setTextColor(GxEPD_LIGHTGREY);
  int x, y;
  for (x = 2; x <= 300; x+=4) {
    for (y=250; y <= 400; y +=4) {
      display.drawPixel(x, y, GxEPD_BLACK);
    }
  }

  display.drawRect(2,250,298,148, GxEPD_RED);
  display.println(ip);
  display.println(text);  
  display.println(message);
  display.setTextColor(GxEPD_RED);
  display.setFont(&FreeMonoBold9pt7b);
  display.println(footer);
  int v = analogRead(LIGHT_SENSOR_PIN);
  char cstr[16];
  display.println(itoa(v, cstr, 10));
  display.setTextColor(GxEPD_BLACK);
  
  display.update();
  //delay(5000);    
}
//void TaskScreen (void * parameters) {
  void Task3code (void * parameters) {  
  Serial.print("Task 3 running on core # ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    Serial.print("Updating screen ...");
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    showUpdate(info_ip_addr, aws_msg, &FreeMonoBold12pt7b);
    vTaskSuspend(NULL);
  } 
} 


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");

  pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);
  pinMode(output_1, OUTPUT);
  digitalWrite(output_1, HIGH);
  pinMode(output_2, OUTPUT);
  digitalWrite(output_2, HIGH);
  pinMode(output_22, OUTPUT);
  digitalWrite(output_22, HIGH);

  // Create thread for task 1
  xTaskCreatePinnedToCore(Task1code, "Task1", 1000, NULL, 2, &Task1, 0);    
  // Create thread for task 2
  xTaskCreatePinnedToCore(Task2code, "Task2", 1000, NULL, 1, &Task2, 1);  
  // Create thread for task 3
  //xTaskCreatePinnedToCore(TaskScreen, "Task3", 1000, NULL, 5, &Task3, 1);
  xTaskCreatePinnedToCore(Task3code, "Task3", 1000, NULL, 5, &Task3, 1);  

  WiFi.mode(WIFI_STA);
  String hostname = "ESP32LF";
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.println("Connecting to Wi-Fi ...");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP: ");
  //Serial.println(WiFi.localIP());
  //info_ip_addr[16] = "000.000.000.000";

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  client.publish(AWS_IOT_CHANNEL_5, "10");
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  //client.setServer(AWS_IOT_ENDPOINT, 443);
 
  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IoT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_CHANNEL_1);
  client.subscribe(AWS_IOT_CHANNEL_2);
  client.subscribe(AWS_IOT_CHANNEL_3);
  client.subscribe(AWS_IOT_CHANNEL_4);
 
  Serial.println("AWS IoT Connected!");  
}

void loop()
{
  // Publishes value to MQTT
  //int r = random();
  //char cstr[16];
  //client.publish(AWS_IOT_CHANNEL_5, itoa(r, cstr, 10));

  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  if (analogValue < ANALOG_THRESHOLD)
    digitalWrite(LED_PIN, LOW);
  else
    digitalWrite(LED_PIN, HIGH);
#if !defined(__AVR)

#else

#endif
  client.loop();
  delay(1000);
}