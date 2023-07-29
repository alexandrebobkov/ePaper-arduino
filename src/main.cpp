/*
  Written for board Espressive ESP32-WROOM-32
    
  ePaper embeded system program written in style adopted for learning.
  Adopted & written by: Alexander Bobkov
  July 28, 2023
*/

// Define modules
#define RTC
#define MICRO_SD
#define BMP280
#define BME280

#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "WiFi.h"
//#include <WiFi.h>
//#include <Wire.h> 
//#include <GxEPD.h>
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
//#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
//#include <Fonts/FreeMonoBold9pt7b.h>
//#include <Fonts/FreeMonoBold12pt7b.h>
//#include <Fonts/FreeMonoBold18pt7b.h>
//#include <Fonts/FreeMonoBold24pt7b.h>
//#include <GxIO/GxIO_SPI/GxIO_SPI.h>
//#include <GxIO/GxIO.h>

#include <SPI.h>
//#include <Adafruit_GFX.h>

#ifdef RTC
#include <RTClib.h>
#endif

//#include <Adafruit_Sensor.h>
#ifdef BME280
#include <Adafruit_BME280.h>
#endif
#ifdef BMP280
#include <Adafruit_BMP280.h>
#endif
//#include <Timelib.h>
//#include <ErriezDS3231.h>
//#include "GxIO.h"
#include "mqtt.h"
//#include "automation.h"
#include "automation-0.h"
//#include "dashboard-0.h"
//#include "dashboard.h"
//#include "recorder.h"


struct Data {
  const char* temp;
  const int* v;
  int * humidity;
};




#ifdef RTC
RTC_DS3231 rtc;
#endif


#ifdef BME280
// WaveShare BME280
Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
#endif

// BMP280
#ifdef BMP280
#define BMP_SCK   (18)
#define BMP_MISO  (19)
#define BMP_MOSI  (23)
#define BMP_CS    (5)
Adafruit_BMP280 bmp(BMP_CS);
#endif

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/*// Define output pins
const int output_2 = 2;//4;   // built-in LED pin #
// output pins that will be used to control relay; for now they control LEDs
const int output_1 = 22;//19;    // Pin 19 conflicts with ssd card module
const int output_22 = 22;   // Pin 22
const int output_23 = 21;*/
int sensor_values[68];
int sensor_val = 0;

char aws_msg[25] = "";
char info_ip_addr[16] = "000.000.000.000";
char display_msg[4][50] = {"", "", "", ""};
float temp = 0.0;
float humidity = 0.0;
float pressure = 0.0;

// Publishes value to MQTT  
int mqtt_temp;
/*
int bme_humidity;
int bme_temperature;
int bme_pressure;
*/
int humidity;
int temperature;
int pressure;

//int min;
int r;
char cstr[16];

String full_date;

//void drawLogo(File f);

//AWSIoT
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Mosquitto
WiFiClient espClient;
PubSubClient mosquitto(espClient);

// Section of code that processes JSON command(s) received from AWS IoT
/*void messageHandler(char* topic, byte* payload, unsigned int length)
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
  // on-off cycle
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
}*/

// Dummy task. Runs to blink built-in LED. Indicates that board has started
void Task2code (void * parameters) {  
  Serial.print("Task 2 running on core # ");
    Serial.println(xPortGetCoreID());
    int i = 0;
    int c = 0, v = 10;

    for (;;)
    {
      if ((i>=0) && (i<68))
        {
          sensor_values[i] = analogRead(LIGHT_SENSOR_PIN);
          //analogWrite(RGB_BLUE_PIN, (sensor_values[i]/4095)*200);
          i++;
        }
        else
        {
          i = 0;
          vTaskResume(Task3);
          //vTaskResume(TaskSd);
        }
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
    }
}

void LampTaskCode (void * parameters)
{
  //int i = 0;
  
  for (;;)
  {
    int analogValue = analogRead(LIGHT_SENSOR_PIN);
    if (analogValue < ANALOG_THRESHOLD)
    {
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(10000);
    }

    /*DateTime now = rtc.now();
    full_date = now.timestamp();
    String date = now.timestamp();
    Serial.println(date);
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
  /*Serial.println();
  Serial.print("Temperature: ");
  rtc_temp = rtc.getTemperature();
  //Serial.println(rtc.getTemperature(), DEC);
  Serial.println(rtc_temp, DEC);*/

  // WaveShare BME280
  /*Serial.println("\n==== BME-280 =============");
  Serial.print("Temperature = ");
  Serial.println(bme.readTemperature());
  humidity = (float)bme.readHumidity();
  Serial.print("Humidity = ");
  //Serial.println(bme.readHumidity());
  Serial.print(humidity);
  Serial.println("%");
  pressure = (float)bme.readPressure() / 100.0F;
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  // WaveShare BME280
  Serial.println("\n==== BMP-280 =============");
  Serial.print("Temperature = ");
  Serial.println(bmp.readTemperature());
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  // Publishes value to MQTT  
  temp = (float)rtc.getTemperature();
  bme_humidity = (float)bme.readHumidity();
  bme_temperature = (float)bme.readTemperature();
  bme_pressure = (float)bme.readPressure() / 100.0F;
  //min = now.minute();
  r = random();
  //int analogValue = analogRead(LIGHT_SENSOR_PIN);*/
  


    vTaskDelay(1000);
  }
}

/*void showUpdate(char ip[], const char text[], const GFXfont* f) {
  //const char header[25] = "Networks IV\n";
  const char header[25] = "Workdesk Automation\n"; 
  //const char ip[25] = "IP: 10.100.50.20";
  const char ip_addr[] = "121.21.10.20";
  //const char footer[] = "\nWireless\nAutomation Board\n\nControlled via Cloud";
  const char footer[] = "\nSensors and Variables";
  const char message[] = "MQTT CH-1: Relay 1 ON";
  
  //display.updateWindow(70,20,300,400,false);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextColor(GxEPD_DARKGREY); //
  display.setFont(f);
  display.setCursor(10, 20);
  display.println(header);
  display.setFont(&FreeMonoBold9pt7b);
  //display.print("Updated: ");
  display.println(full_date);
  //display.setTextColor(GxEPD_LIGHTGREY);
  int x, y;
  int n = 0;
  for (x = 10; x < 280; x+=4) {
    for (y = 240; y < 380; y +=4)
      display.drawPixel(x, y, GxEPD_BLACK);      
    n++;
  }

  for (int i = 0; i < 68; i++)
    display.fillCircle(10+i*4, 380-(sensor_values[i]/30), 2, GxEPD_RED);

  display.drawRect(2, 232, 12+x-10, 12+y-240, GxEPD_RED);
  Serial.print("\nArray: ");
  Serial.println(n);

  //display.drawRect(2,250,298,148, GxEPD_RED);
  //display.setFont(&FreeMonoBold9pt7b);
  display.print("IP: ");
  display.println(ip);
  //display.println(text);  
  display.println(message);
  display.setTextColor(GxEPD_RED);
  display.setFont(&FreeMonoBold9pt7b);
  // Display sensors and variables
  display.println(footer);
  display.setTextColor(GxEPD_BLACK);
  // Display light sensor reading
  int v = analogRead(LIGHT_SENSOR_PIN);
  char cstr[8];//16];
  display.println(itoa(v, cstr, 10));
  // Display temperature sensor reading
  display.print("Temperature: ");  
  char temp_cstr[8];//16];
  display.print(itoa(temp, temp_cstr, 10));
  //display.print(itoa(humidity, temp_cstr, 10));
  display.println("C");

  // Display humidity sensor reading
  display.print("Humidity: ");
  char h_cstr[8];
  display.print(itoa(humidity, h_cstr, 10));
  display.println("%");

  // Display pressure sensor reading
  display.print("Pressure: ");
  char p_cstr[8];
  display.print(itoa(pressure, p_cstr, 10));
  display.print(" kPa");
  
  display.update(); 
}

// Display information on ePaper display.
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
}*/

// Task for handling wireless connection
void TaskConnection (void * parameters) {
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
    // Update IP address for displaying.
    String lan_addr = WiFi.localIP().toString();
    lan_addr.toCharArray(info_ip_addr, lan_addr.length()+1);
    vTaskSuspend(NULL);
  }
}

void TaskSdCode (void* parameters) {
    vTaskSuspend(NULL);
}

void mosquito_callback (char* topic, byte* message, unsigned int length)
{
  Serial.print("\nMessage arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i=0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output")
  {
    Serial.print("Main. Changing output to: ");
    if (messageTemp == "on")
    {
      Serial.println("Turn switch ON!\n");
      digitalWrite(SWITCH_1, LOW);    // Active level LOW
    }
    else if (messageTemp == "off")
    {
      Serial.println("Turn switch OFF!\n");
      digitalWrite(SWITCH_1, HIGH);
    }
  }
}

void setupMQTT() {
  mosquitto.setServer(mqtt_server, 1883);
  // set the callback function
  mosquitto.setCallback(mosquito_callback);
}

void reconnect()
{
  while (!mosquitto.connected())
  {
    if (mosquitto.connect("ESP32Client"))
    {
      Serial.println("connected");
      mosquitto.subscribe("esp32/output");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");  
  Serial.println("setup done");

  // WaveShare BME280
  #ifdef BME280
  unsigned status = bme.begin(); 
  if (!status) {
    Serial.println("Could not find a valid BME/BMP280 sensor, check wiring!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    Serial.print("   ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("   ID of 0x60 represents a BME 280.\n");
    Serial.print("   ID of 0x61 represents a BME 680.\n");
    while (1);
  }
  else {
    humidity = bme.readHumidity();
    pressure = bme.readPressure()  / 100.0F;
  }
  #endif

  #ifdef BMP280
  // BMP280
  /*unsigned status_bmp280;
  status_bmp280 = bmp.begin();
  if (!status_bmp280) {
    Serial.println("Could not find BMP280");
    Serial.println(bme.sensorID(),16);
    while (1);
  }*/
  #endif

  // Initialize RTC module, if defined
  #ifdef RTC
  rtc.begin();  
  if (! rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  temp = rtc.getTemperature();

  #endif  
  // Uncomment when compiling for the first time
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  /*if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date &amp; time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date &amp; time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }*/

  String hostname = "ESP32LF";
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
  
  Serial.println("Connecting to Wi-Fi ...");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nCONNECTED\nIP: ");
  
  Serial.println(WiFi.localIP());
  
  // MOSQUITTO MQTT
  Serial.println("Connecting to Mosquitto");
  //mosquitto.publish(MQTT_IOT_CHANNEL_0, "HI");
  mosquitto.setServer(mqtt_server, 1883);
  mosquitto.setCallback(mosquito_callback);
  if(mosquitto.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    mosquitto.subscribe("esp32/output");
    mosquitto.setCallback(mosquito_callback);
  }
  else
    Serial.print("Mosquitto state: ");
    Serial.println(mosquitto.state());
}



void loop()
{
  #ifdef RTC
  DateTime now = rtc.now();
  full_date = now.timestamp();
  String date = now.timestamp();
  Serial.println(date);
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.print("Temperature: ");
  temp = rtc.getTemperature();
  //Serial.println(rtc.getTemperature(), DEC);
  Serial.println(temp, DEC);
  #endif

  #ifdef BME280
  // WaveShare BME280
  Serial.println("\n==== BME-280 =============");
  Serial.print("Temperature = ");
  Serial.println(bme.readTemperature());
  humidity = (float)bme.readHumidity();
  Serial.print("Humidity = ");
  //Serial.println(bme.readHumidity());
  Serial.print(humidity);
  Serial.println("%");
  pressure = (float)bme.readPressure() / 100.0F;
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  // WaveShare BME280
  Serial.println("\n==== BMP-280 =============");
  Serial.print("Temperature = ");
  Serial.println(bmp.readTemperature());
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.println("\n==== MQTT =============");
  // Publishes value to MQTT  
  /*
  int temp = (float)rtc.getTemperature();
  int bme_humidity = (float)bme.readHumidity();
  int bme_temperature = (float)bme.readTemperature();
  int bme_pressure = (float)bme.readPressure() / 100.0F;
  */
  int temp = (float)rtc.getTemperature();
  humidity = (float)bme.readHumidity();
  temperature = (float)bme.readTemperature();
  pressure = (float)bme.readPressure() / 100.0F;
  //int min = now.minute();
  //int r = random();
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  char cstr[16];
  #endif

  // Mosquitto
  mosquitto.publish(MQTT_IOT_CHANNEL_1, itoa(temp, cstr, 10));
  mosquitto.publish(MQTT_IOT_CHANNEL_TEMPERATURE, itoa(temperature, cstr, 10));
  mosquitto.publish(MQTT_IOT_CHANNEL_PRESSURE, itoa(pressure, cstr, 10));
  mosquitto.publish(MQTT_IOT_CHANNEL_HUMIDITY, itoa(humidity, cstr, 10));
  mosquitto.publish(MQTT_IOT_CHANNEL_0, "10");
  Serial.println("test_topic: 10");
  delay(500);
  mosquitto.publish(MQTT_IOT_CHANNEL_0, "3");
  Serial.println("test_topic: 3");
  Serial.println(mosquitto.state());
  delay(500);
  client.loop();
  mosquitto.loop();
}

