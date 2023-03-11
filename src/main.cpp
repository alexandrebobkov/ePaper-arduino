/*
  Written for board Espressive ESP32-WROOM-32
    
  ePaper embeded system program written in style adopted for learning.
  Adopted & written by: Alexander Bobkov
  Mar 10, 2023
*/

#include "secrets.h"
#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "WiFi.h"
#include <WiFi.h>
//#include <Wire.h> 
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
#include <SD.h>
#include <SPI.h>
//#include <Adafruit_GFX.h>
#include <RTClib.h>
//#include <ErriezDS3231.h>
//#include "GxIO.h"
#include "mqtt.h"
#include "automation.h"

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

struct Data {
  const char* temp;
  const int* v;
};

// Define tasks.
//TaskHandle_t Task1, Task2, Task3, Task4, Task5;   // For prototyping purposes these tasks control LEDs based on received command
//TaskHandle_t LampTask, StorageCard;

RTC_DS3231 rtc;
//ErriezDS3231 rtc;

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

void printDirectory(File dir, int numTabs);
void drawLogo(File f);

WiFiClientSecure net = WiFiClientSecure();
// AWS IoT
PubSubClient client(net);
// Mosquitto
PubSubClient mosquitto(net);


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

      /*
      Serial.print("[");
      Serial.print(i);
      Serial.print("]; ");
      Serial.print("Sensor value: ");
      //client.publish(AWS_IOT_CHANNEL_5, "0");
      Serial.println(analogRead(LIGHT_SENSOR_PIN));  
      */

      /*if (c > 0 && c < 255)
      {
        analogWrite(RGB_BLUE_PIN, c);
        c += v;
      }
      if (c == 255)
        v = -10;
      if (c == 0)
        v = 10;*/

      /*for (int c = 0; c < 255; c++)
      {
        analogWrite(RGB_BLUE_PIN, c);
        vTaskDelay(50);
      }*/

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
    vTaskDelay(1000);
  }
}

void showUpdate(char ip[], const char text[], const GFXfont* f) {
  const char header[25] = "Networks IV\n"; 
  //const char ip[25] = "IP: 10.100.50.20";
  const char ip_addr[] = "121.21.10.20";
  //const char footer[] = "\nWireless\nAutomation Board\n\nControlled via Cloud";
  const char footer[] = "\nSensors and Variables";
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
  display.print("IP: ");
  display.println(ip);
  display.println(text);  
  display.println(message);
  display.setTextColor(GxEPD_RED);
  display.setFont(&FreeMonoBold9pt7b);
  // Display sensors and variables
  display.println(footer);
  display.setTextColor(GxEPD_BLACK);
  // Display light sensor reading
  int v = analogRead(LIGHT_SENSOR_PIN);
  char cstr[16];
  display.print(itoa(v, cstr, 10));
  // Display temperature sensor reading
  display.print("   ");
  
  char temp_cstr[16];
  display.print(itoa(temp, temp_cstr, 10));
  display.print("C");
  
  display.update();
  //delay(5000);    
}
//void TaskScreen (void * parameters) {

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
}

// WeMos D1 esp8266: D8 as standard
    const int chipSelect = SS;



void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");

  
  if (! rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date &amp; time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date &amp; time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  temp = rtc.getTemperature();

    
  Serial.println("\n======================");
  Serial.print("\nInitializing SD card...");
 
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
 
  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.cardType()) {
    case CARD_NONE:
      Serial.println("NONE");
      break;
    case CARD_MMC:
      Serial.println("MMC");
      break;
    case CARD_SD:
      Serial.println("SD");
      break;
    case CARD_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  Serial.print("Card size:  ");
  Serial.println((float)SD.cardSize()/1000);
 
  Serial.print("Total bytes: ");
  Serial.println(SD.totalBytes());
 
  Serial.print("Used bytes: ");
  Serial.println(SD.usedBytes());
 
  File dir =  SD.open("/");
  drawLogo(SD.open("/picture-001.bmp"));
  delay(5000);
  //printDirectory(dir, 0);

  Serial.println("\n======================");
  

  pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);
  pinMode(output_1, OUTPUT);
  digitalWrite(output_1, HIGH);
  pinMode(output_2, OUTPUT);
  digitalWrite(output_2, HIGH);
  pinMode(output_22, OUTPUT);
  digitalWrite(output_22, HIGH);

  // RGB
  //pinMode(RGB_RED_PIN, OUTPUT);
  //digitalWrite(RGB_RED_PIN, HIGH);

  //pinMode(RGB_BLUE_PIN, OUTPUT);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(RGB_BLUE_PIN, 0);
  //ledcAttachPin(RGB_RED_PIN, 0);
  //digitalWrite(RGB_BLUE_PIN, HIGH);
  //analogWrite(RGB_BLUE_PIN, 255);

  /*pinMode(RGB_BLUE_PIN, OUTPUT);
  digitalWrite(RGB_BLUE_PIN, HIGH);*/
 
  // print the type and size of the first FAT-type volume
//  uint32_t volumesize;
//  Serial.print("Volume type is:    FAT");
//  Serial.println(SDFS.usefatType(), DEC);
 
  

  // Create thread for task 1
  xTaskCreatePinnedToCore(Task1code, "Task1", 1000, NULL, 2, &Task1, 0);    
  // Create thread for task 2
  xTaskCreatePinnedToCore(Task2code, "Task2", 1000, NULL, 1, &Task2, 1);  
  // Create thread for task 3
  //xTaskCreatePinnedToCore(TaskScreen, "Task3", 1000, NULL, 5, &Task3, 1);
  // Display information on ePaper display.
  //xTaskCreatePinnedToCore(Task3code, "Task3", 1000, NULL, 5, &Task3, 1);  
  xTaskCreatePinnedToCore(LampTaskCode, "Lamp Task", 1000, NULL, 5, &LampTask, 0);
  //xTaskCreatePinnedToCore(StorageCardcode, "Storage Card", 1000, NULL, 7, &StorageCard, 1);
  

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
  Serial.print("\nCONNECTED\nIP: ");
  Serial.println(WiFi.localIP());
  // Update IP address for displaying.
  String lan_addr = WiFi.localIP().toString();
  lan_addr.toCharArray(info_ip_addr, lan_addr.length()+1);
  // Call Task to display information on ePaper display.
  xTaskCreatePinnedToCore(Task3code, "Task3", 1000, NULL, 5, &Task3, 1);
  //info_ip_addr[16] = "000.000.000.000";

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  //client.publish(AWS_IOT_CHANNEL_5, "10");
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
  // AWS IoT Subscribe to a topic
  client.subscribe(AWS_IOT_CHANNEL_1);
  client.subscribe(AWS_IOT_CHANNEL_2);
  client.subscribe(AWS_IOT_CHANNEL_3);
  client.subscribe(AWS_IOT_CHANNEL_4);
  client.subscribe(AWS_IOT_CHANNEL_5); 
  Serial.println("AWS IoT Connected!");

  /*
  // MOSQUITTO MQTT
  Serial.println("Connecting to Mosquitto");
  //mosquitto.publish(MQTT_IOT_CHANNEL_0, "HI");
  mosquitto.setServer(NODE_MQTT, 1883);
  while(!mosquitto.connect("Mosquitto"))
  {
    Serial.print(".");
    delay(10);
  }
  if (!mosquitto.connected())
  {
    Serial.println("Mosquitto Timeout");
    return;
  }
  mosquitto.subscribe(MQTT_IOT_CHANNEL_0);
  Serial.println("Mosquitto Connected!");
  */

}

void loop()
{
  DateTime now = rtc.now();
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

  // Publishes value to MQTT  
  int temp = (float)rtc.getTemperature();
  int min = now.minute();
  int r = random();
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  char cstr[16];
  client.publish(AWS_IOT_CHANNEL_5, itoa(min, cstr, 10));
  client.publish(AWS_IOT_CHANNEL_5, itoa(temp, cstr, 10));

  // Mosquitto
  mosquitto.publish(MQTT_IOT_CHANNEL_0, "10");

  
  
/*
  
  if (analogValue < ANALOG_THRESHOLD)
    digitalWrite(LED_PIN, LOW);
  else
    digitalWrite(LED_PIN, HIGH);
    */
#if !defined(__AVR)

#else

#endif
  client.loop();
  for (int d = 20; d <= 255; d++)
  {
    ledcWrite(0, d);
    delay(25);
  }
  for (int d = 255; d >= 20; d--)
  {
    ledcWrite(0, d);
    delay(25);
  }
  delay(1000);
}

void printDirectory(File dir, int numTabs) {
  while (true) {
 
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t lw = entry.getLastWrite();
      struct tm * tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

uint16_t read16(File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

static const uint16_t input_buffer_pixels = 800; // may affect performance
static const uint16_t max_row_width = 1448; // for up to 6" display 1448x1072
static const uint16_t max_palette_pixels = 256; // for depth <= 8
uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint16_t rgb_palette_buffer[max_palette_pixels]; // palette buffer for depth <= 8 for buffered graphics, needed for 7-color display


void drawBitmapFromSD(File file, int16_t x, int16_t y, bool with_color)
//void drawBitmapFromSD(const char *filename, int16_t x, int16_t y, bool with_color)
{
  bool valid = false; // valid format to be handled
  bool flip = true; // bitmap is stored bottom-to-top
  bool has_multicolors = false;//display.epd2.panel == GxEPD2::ACeP565;
  uint32_t startTime = millis();
  if ((x >= display.width()) || (y >= display.height())) return;
  Serial.println();
  Serial.print("Loading image '");
  /*Serial.print(filename);
  Serial.println('\'');
#if defined(ESP32)
  file = SD.open(String("/") + filename, FILE_READ);
  if (!file)
  {
    Serial.print("File not found");
    return;
  }
#else
  file = SD.open(filename);
  if (!file)
  {
    Serial.print("File not found");
    return;
  }
#endif*/
  // Parse BMP header
  if (read16(file) == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file); //(void)creatorBytes; //unused
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width  = read32(file);
    int32_t height = (int32_t) read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      Serial.print("File size: "); Serial.println(fileSize);
      Serial.print("Image Offset: "); Serial.println(imageOffset);
      Serial.print("Header size: "); Serial.println(headerSize);
      Serial.print("Bit Depth: "); Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8) rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0)
      {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.width())  w = display.width()  - x;
      if ((y + h - 1) >= display.height()) h = display.height() - y;
      //if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish = false;
        bool colored = false;
        if (depth == 1) with_color = false;
        if (depth <= 8)
        {
          if (depth < 8) bitmask >>= depth;
          //file.seek(54); //palette is always @ 54
          file.seek(imageOffset - (4 << depth)); //54 for regular, diff for colorsimportant
          for (uint16_t pn = 0; pn < (1 << depth); pn++)
          {
            blue  = file.read();
            green = file.read();
            red   = file.read();
            file.read();
            whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
            rgb_palette_buffer[pn] = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
          }
        }
        
        
          uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
          for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) // for each line
          {
            uint32_t in_remain = rowSize;
            uint32_t in_idx = 0;
            uint32_t in_bytes = 0;
            uint8_t in_byte = 0; // for depth <= 8
            uint8_t in_bits = 0; // for depth <= 8
            uint16_t color = GxEPD_WHITE;
            file.seek(rowPosition);
            for (uint16_t col = 0; col < w; col++) // for each pixel
            {
              // Time to read more pixel data?
              if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
              {
                in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
                in_remain -= in_bytes;
                in_idx = 0;
              }
              switch (depth)
              {
                case 24:
                  blue = input_buffer[in_idx++];
                  green = input_buffer[in_idx++];
                  red = input_buffer[in_idx++];
                  whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                  colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                  color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
                  break;
                case 16:
                  {
                    uint8_t lsb = input_buffer[in_idx++];
                    uint8_t msb = input_buffer[in_idx++];
                    if (format == 0) // 555
                    {
                      blue  = (lsb & 0x1F) << 3;
                      green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                      red   = (msb & 0x7C) << 1;
                      color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
                    }
                    else // 565
                    {
                      blue  = (lsb & 0x1F) << 3;
                      green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                      red   = (msb & 0xF8);
                      color = (msb << 8) | lsb;
                    }
                    whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                    colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                  }
                  break;
                case 1:
                case 4:
                case 8:
                  {
                    if (0 == in_bits)
                    {
                      in_byte = input_buffer[in_idx++];
                      in_bits = 8;
                    }
                    uint16_t pn = (in_byte >> bitshift) & bitmask;
                    whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    in_byte <<= depth;
                    in_bits -= depth;
                    color = rgb_palette_buffer[pn];
                  }
                  break;
              }
              if (with_color && has_multicolors)
              {
                // keep color
              }
              else if (whitish)
              {
                color = GxEPD_WHITE;
              }
              /*else if (colored && with_color)
              {
                color = GxEPD_COLORED;
              }*/
              else
              {
                color = GxEPD_BLACK;
              }
              uint16_t yrow = y + (flip ? h - row - 1 : row);
              display.drawPixel(x + col, yrow, color);
            } // end pixel
          } // end line
          Serial.print("page loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
       // }
        //while (display.nextPage());
        Serial.print("loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
      }
    }
  }
  file.close();
  if (!valid)
  {
    Serial.println("bitmap format not handled.");
  }
}

void drawLogo(File f)
{
  int16_t w2 = display.width() / 2;
  int16_t h2 = display.height() / 2;
  drawBitmapFromSD(f, w2 - 64, h2 - 80, false);
}



