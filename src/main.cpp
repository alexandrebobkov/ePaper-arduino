/*
  ESP32-WROOM-32
  ePaper embeded system program written in style adopted for learning.
*/
#include "secrets.h"
// Adopted libraries
//#include <GxGDEW042Z15.h>
#include "GxGDEW042Z15.h"                 // ePaper WaveShare screen 4.2" b/w/r
#include "GxIO.h"
#include "GxIO_SPI.h"

#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "WiFi.h"
#include <WiFi.h>
#include <Wire.h> 
//#include <GxEPD.h>
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
//#include <GxIO/GxIO_SPI/GxIO_SPI.h>
//#include <GxIO/GxIO.h>

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

#define AWS_IOT_SUBSCRIBE_TOPIC1 "esp32/lamp1"
#define AWS_IOT_SUBSCRIBE_TOPIC2 "esp32/lamp2"
#define AWS_IOT_SUBSCRIBE_TOPIC3 "esp32/lamp3"
#define AWS_IOT_SUBSCRIBE_TOPIC4 "esp32/lamp4"

// Define tasks.
TaskHandle_t Task1, Task2, Task3;

//void Task3 (void *pvParameters);

// Define output pins
const int output_1 = 19;
const int output_2 = 2;//4;
const int output_22 = 22;
const int output_23 = 21;
char aws_msg[25] = "";

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  /*##################### Lamp 1 #####################*/
  /*
  if ( strstr(topic, "esp32/lamp1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay1 = doc["status"];
    int r1 = Relay1.toInt();
    if(r1==1)
    {
      digitalWrite(lamp1, HIGH);
      Serial.print("Lamp1 is ON");
    }
    else if(r1==0)
    {
      digitalWrite(lamp1, LOW);
      Serial.print("Lamp1 is OFF");
    }
  }
 */
/*##################### Lamp 2 #####################*/
  if ( strstr(topic, "esp32/lamp2") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay2 = doc["status"];
    int r2 = Relay2.toInt();
    if(r2==1)
    {
      //digitalWrite(lamp2, HIGH);
      Serial.print("Resume task 2");
      //lcd.clear();
      //lcd.setCursor(0,0);
      //lcd.print("  Task 2 ");
      //lcd.setCursor(0,1);
      //lcd.print("  Resume ");
      vTaskResume(Task2);
      
    }
    else if(r2==0)
    {
      //digitalWrite(lamp2, LOW);
      Serial.print("Suspend task 2");
      //lcd.clear();
      //lcd.setCursor(0,0);
      //lcd.print("  Task 2 ");
      //lcd.setCursor(0,1);
      //lcd.print("  Suspend ");
      vTaskSuspend(Task2);
      
    }
  }
 
/*##################### Lamp 3 #####################*/
  if ( strstr(topic, "esp32/lamp3") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay3 = doc["status"];
    int r3 = Relay3.toInt();
    if(r3==1)
    {
      //digitalWrite(lamp3, LOW);
      Serial.print("Lamp3 is ON");
    }
    else if(r3==0)
    {
      //digitalWrite(lamp3, HIGH);
      Serial.print("Lamp3 is OFF");
    }
  }
 
/*##################### Lamp 4 #####################*/
  if ( strstr(topic, "esp32/lamp4") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay4 = doc["status"];
    String Relay4_msg = doc["message"];
    strcpy(aws_msg, Relay4_msg.c_str());
    int r4 = Relay4.toInt();
    if(r4==1)
    {
      //digitalWrite(lamp4, HIGH);
      Serial.print("Lamp4 is ON");
      vTaskResume(Task3);
      //delay(500);
      //vTaskSuspend(Task3);
      Serial.println("Lamp4 is ON");
      Serial.println(Relay4_msg);
      //lcd.clear();
      //lcd.setCursor(0,0);
      //lcd.print("  LED 4 ");
      //lcd.setCursor(0,1);
      //lcd.print("  ON ");
    }
    else if(r4==0)
    {
      //digitalWrite(lamp4, LOW);
      Serial.print("Lamp4 is OFF");
      Serial.println(Relay4_msg);
      //lcd.clear();
      //lcd.setCursor(0,0);
      //lcd.print("  LED 4 ");
      //lcd.setCursor(0,1);
      //lcd.print("  OFF ");
    }
  }
  Serial.println();
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

void Task2code (void * parameters) {  
  Serial.print("Task 2 running on core # ");
    Serial.println(xPortGetCoreID());

    for (;;) {
    //for (int i=0; i<3; i++) {
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
      //vTaskSuspend(NULL);         
      //vTaskDelete(NULL);   
    }
    //vTaskSuspend(NULL);    
    //vTaskDelete(NULL);  
} 

void showUpdate(const char text[], const GFXfont* f) {
  const char header[25] = "AWS IoT"; 
  const char ip[25] = "IP: ";
  const char ip_addr[] = "0.0.0.0";
  const char footer[] = "\nby: Alexander Bobkov"; 
  
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(100, 20);
  display.println(header);
  display.print(ip);
  display.print(text);  
  display.setTextColor(GxEPD_RED);
  display.println(footer);
  display.update();
  //delay(5000);    
}
void TaskScreen (void * parameters) {  
  Serial.print("Task 3 running on core # ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    Serial.print("Updating screen ...");
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    showUpdate(aws_msg, &FreeMonoBold12pt7b);
      //showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
      //showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
      //display.drawCornerTest();
      //vTaskDelay(30);
      //display.drawPaged(showFontCallback);
      //display.setRotation(0);      
      //display.powerDown();     
    //vTaskDelay(5000);
    //vTaskDelete(NULL);
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

  pinMode(output_1, OUTPUT);
  digitalWrite(output_1, HIGH);
  pinMode(output_2, OUTPUT);
  digitalWrite(output_2, HIGH);

  // Create thread for task 1
  xTaskCreatePinnedToCore(Task1code, "Task1", 1000, NULL, 2, &Task1, 0);    
  // Create thread for task 2
  xTaskCreatePinnedToCore(Task2code, "Task2", 1000, NULL, 1, &Task2, 1);  
  // Create thread for task 3
  xTaskCreatePinnedToCore(TaskScreen, "Task3", 1000, NULL, 5, &Task3, 1);  

  WiFi.mode(WIFI_STA);
  String hostname = "Alex IoT";
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  

  Serial.println("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
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
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC1);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC3);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC4);
 
  Serial.println("AWS IoT Connected!");  
}



void loop()
{
  //showBoat2();
  //delay(2000);  
  //showBitmapExample();
  //delay(2000);
#if !defined(__AVR)
  //drawCornerTest();
  //showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  //showFont("FreeMonoBold12pt7b", &FreeMonoBold12pt7b);
  // //showUpdate("AWS IoT", &FreeMonoBold12pt7b);
  //showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
  //showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
#else
  //display.drawCornerTest();
  // //delay(2000);
  //display.drawPaged(showFontCallback);
#endif
  // //display.powerDown();
  // //delay(10000);
  client.loop();
  delay(1000);
}

/*void showWifiStatus(const char text[], const GFXfont* f) {
  const char wifi_status[25] = "AWS IoT"; 
  const char ip[] = "IP: ";
  const char ip_addr[] = "0.0.0.0";
  const char footer[] = "\nby: Alexander Bobkov";  
  //display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(100, 200);  
  display.setTextColor(GxEPD_RED);
  display.println(text);
  display.update();
}



void showInfo(const char name[], const GFXfont* f) {
  const char header[] = "AWS IoT"; 
  const char ip_addr[] = "IP: ";
  const char footer[] = "\nby: Alexander Bobkov";  
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  // partial update to full screen to preset for partial update of box window
  // (this avoids strange background effects)
  // display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
  display.setFont(f);
  display.setCursor(5, 20);
  //display.println();
  display.println(header);
  //display.println();
  display.println(ip_addr);
  //display.println();
  display.println(name);
  display.setTextColor(GxEPD_RED);
  display.println(footer);
//#if defined(HAS_RED_COLOR)
//  display.setTextColor(GxEPD_RED);
//  display.println(const char ["AWS IoT"]);
//#endif
  display.update();
  delay(5000);  
}

void showFont(const char name[], const GFXfont* f)
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println(name);
#if defined(HAS_RED_COLOR)
  display.setTextColor(GxEPD_RED);
#endif
  display.update();
  delay(5000);
}
*/


/*
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
*/