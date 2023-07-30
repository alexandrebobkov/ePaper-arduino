/*
  Written for board Espressive ESP32-WROOM-32
    
  ePaper embeded system program written in style adopted for learning.
  Adopted & written by: Alexander Bobkov
  July 28, 2023
*/

// Uncomment modules as required
//#define RTC
#define MICRO_SD
#define BMP280      // Adafruit BMP280; temp & pressure
//#define BME280    // Generic BME280; temp, pressure & humidity
//#define AWSIoT



#define MQTT_SSL
//#define HOTSPOT
//#define MQTT

#include "automation-0.h"
#include "mqtt.h"
#include "secrets.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>             // ESP32 library
#include <PubSubClient.h>

// Include libraries based on modules selected
#ifdef RTC
#include <RTClib.h>
#endif
#ifdef BME280
#include <Adafruit_BME280.h>
#endif
#ifdef BMP280
#include <Adafruit_BMP280.h>
#endif

struct {
  float humidity = 0.0;
  float pressure = 0.0;
  float temperature = 0.0;
} sensors_values;

#ifdef RTC
RTC_DS3231 rtc;
#endif

// BME280
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

// Mosquitto
#ifdef MQTT
WiFiClient espClient;
PubSubClient connection(espClient);
Mosquitto mosquitto = Mosquitto();
#endif
#ifdef MQTT_SSL
WiFiClientSecure espClientSSL = WiFiClientSecure();
PubSubClient connection(espClientSSL); //mosquitto_ssl
Mosquitto mosquitto = Mosquitto();
#endif

//AWSIoT
#ifdef AWSIoT
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
#endif

char cstr[16];

void mosquito_callback (char* topic, byte* message, unsigned int length)
{
  mosquitto.mosquito_callback(topic, message, length);
}

void mosquitto_connect ()
{
  /*
    0 => Connected
    -2
    -4


  */
  while (!connection.connected()) {
  Serial.print("MQTT connection state: ");
  Serial.println(connection.state());
  Serial.print("Connecting to Mosquitto at IP: ");
  Serial.print(mqtt_server);
  #ifdef MQTT // MOSQUITTO MQTT port 1883
  Serial.println(":1883");
  connection.setServer(mqtt_server, 1883);
  //connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif
  #ifdef MQTT_SSL // MOSQUITTO MQTT port 8883
  Serial.println(":8883");
  connection.setServer(mqtt_server, 8883);
  espClientSSL.setCACert(NODE_CERT_CA);
  espClientSSL.setCertificate(NODE_CERT_CRT);
  espClientSSL.setPrivateKey(NODE_CERT_PRIVATE);
  connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif
  delay(2000);
  }

  /*Serial.print("MQTT connection state: ");
  Serial.println(connection.state());
  Serial.print("Connecting to Mosquitto at IP: ");
  Serial.print(mqtt_server);
  #ifdef MQTT // MOSQUITTO MQTT port 1883
  Serial.println(":1883");
  connection.setServer(mqtt_server, 1883);
  //connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif
  #ifdef MQTT_SSL // MOSQUITTO MQTT port 8883
  Serial.println(":8883");
  connection.setServer(mqtt_server, 8883);
  espClientSSL.setCACert(NODE_CERT_CA);
  espClientSSL.setCertificate(NODE_CERT_CRT);
  espClientSSL.setPrivateKey(NODE_CERT_PRIVATE);
  connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif*/
}

// set the callback function
/*void setupMQTT() {
  #ifdef MQTT
    mosquitto.setServer(mqtt_server, 1883);
    mosquitto.setCallback(mosquito_callback);
  #endif
  #ifdef MQTT_SSL
    mosquitto_ssl.setServer(mqtt_server, 8883);
    mosquitto_ssl.setCallback(mosquito_callback);
  #endif
}*/
/*void reconnect()
{
  #ifdef MQTT
  while (!mosquitto.connected())
  {
    if (mosquitto.connect("ESP32Client"))
    {
      Serial.println("connected");
      mosquitto.subscribe("esp32/output");
    }
  }
  #endif
  #ifdef MQTT_SSL
  while (!mosquitto_ssl.connected())
  {
    if (mosquitto_ssl.connect("ESP32Client"))
    {
      Serial.println("connected");
      mosquitto_ssl.subscribe("esp32/output");
    }
  }
  #endif
}*/

/*void mqtt_connect(PubSubClient mqtt)
{
  while (!mqtt.connected())
  {
    if (mqtt.connect("ESP32Client"))
    {
      Serial.println("connected");
      mqtt.subscribe("esp32/output");
    }
  }
}*/

/*void WiFiNodeConnected (WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Connected to AP");
}
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
void WiFiNodeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from AP");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Reconnecting ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Mosquitto at IP: ");
  Serial.print(mqtt_server);

  while (!connection.connected())
  {
    if (connection.connect("ESP32Client"))
    {
      Serial.println("connected");
      connection.subscribe("esp32/output");
    }
  }

  #ifdef MQTT // MOSQUITTO MQTT port 1883
    Serial.println(":1883");
    connection.setServer(mqtt_server, 1883);
    connection.setCallback(mosquito_callback);
    if(connection.connect("ESP32")) {
      Serial.println("Mosquitto Connected!");
      connection.subscribe("esp32/output");
      connection.setCallback(mosquito_callback);
    }
    else
      Serial.print("Mosquitto state: ");
    Serial.println(connection.state());
  #endif
  #ifdef MQTT_SSL // MOSQUITTO MQTT port 8883
    Serial.println(":8883");
    connection.setServer(mqtt_server, 8883);
    espClientSSL.setCACert(NODE_CERT_CA);
    espClientSSL.setCertificate(NODE_CERT_CRT);
    espClientSSL.setPrivateKey(NODE_CERT_PRIVATE);
    connection.setCallback(mosquito_callback);
    if(connection.connect("ESP32")) {
      Serial.println("Mosquitto Connected!");
      connection.subscribe("esp32/output");
      connection.setCallback(mosquito_callback);
    }
    else
      Serial.print("Mosquitto state: ");
    Serial.println(connection.state());
  #endif
}*/

void setup()
{
  Serial.begin(115200);
  Serial.println();
  sensors_values.humidity = 0.0;
  sensors_values.pressure = 0.0;
  sensors_values.temperature = 0.0;

  Serial.println("setup");  
  Serial.println("setup done");

  // WaveShare BME280
  #ifdef BME280
  unsigned status = bme.begin(); 
  if (!status) {
    Serial.println("Could not find a valid BME/BMP280 sensor, check wiring!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID());//,16);
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

  // BMP280
  #ifdef BMP280  
  unsigned status_bmp280;
  status_bmp280 = bmp.begin();
  if (!status_bmp280) {
    Serial.println("Could not find BMP280");
    Serial.println(bmp.sensorID(),16);
    while (1);
  }
  else {}
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
  
  Serial.println("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("#");
  }
  Serial.print("\nCONNECTED\nIP: ");  
  Serial.println(WiFi.localIP());
  
  Serial.print("Connecting to Mosquitto at IP: ");
  Serial.print(mqtt_server);
  #ifdef MQTT // MOSQUITTO MQTT port 1883
  Serial.println(":1883");
  connection.setServer(mqtt_server, 1883);
  //connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  else
    Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif
  #ifdef MQTT_SSL // MOSQUITTO MQTT port 8883
  Serial.println(":8883");
  connection.setServer(mqtt_server, 8883);
  espClientSSL.setCACert(NODE_CERT_CA);
  espClientSSL.setCertificate(NODE_CERT_CRT);
  espClientSSL.setPrivateKey(NODE_CERT_PRIVATE);
  connection.setCallback(mosquito_callback);
  if(connection.connect("ESP32")) {
    Serial.println("Mosquitto Connected!");
    connection.subscribe("esp32/output");
    connection.setCallback(mosquito_callback);
  }
  else
    Serial.print("Mosquitto state: ");
  Serial.println(connection.state());
  #endif
  
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
  int temp = (float)rtc.getTemperature();
  humidity = (float)bme.readHumidity();
  temperature = (float)bme.readTemperature();
  pressure = (float)bme.readPressure() / 100.0F;
  //int min = now.minute();
  //int r = random();
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  char cstr[16];
  #endif

  // WaveShare BMP280
  // Save sensors values into data struct
  #ifdef BMP280
  Serial.println("\n==== BMP-280 =============");
  sensors_values.temperature = (float)bmp.readTemperature();
  sensors_values.pressure = (float)bmp.readPressure();
  #endif

  // Display sensors values
  Serial.print("Temperature = ");
  Serial.println(sensors_values.temperature);
  Serial.print("Pressure = ");
  Serial.print(sensors_values.pressure / 100.0F);
  Serial.println(" Pa");

  /*if (!connection.connected()) {  // if MQTT Client not connected
    mqtt_reconnect();
  }*/
  // Mosquitto MQTT
  #ifdef MQTT
  Serial.println("\n==== MQTT =============");
  #endif
  #ifdef MQTT_SSL
  Serial.println("\n==== MQTT SSL =============");
  #endif

  // If MQTT connected
  if (connection.connected())             // connected() == 1 => Connected
  {
    connection.publish(MQTT_IOT_CHANNEL_TEMPERATURE, itoa(sensors_values.temperature, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_PRESSURE, itoa(sensors_values.pressure / 100.0F, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_HUMIDITY, itoa(sensors_values.humidity, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_0, "10");
    Serial.println("test_topic: 10");
    delay(1000);
    connection.publish(MQTT_IOT_CHANNEL_0, "3");
    Serial.println("test_topic: 3");
    Serial.print("MQTT State: ");
    Serial.println(connection.state());       // state() == 0 => Connected to MQTT
    Serial.print("MQTT Connected: ");
    Serial.println(connection.connected());   // connected() == 1 => Connected to MQTT
    Serial.print("Wi-Fi Connection tatus: ");
    Serial.println(WiFi.status());            // status() == 3 => Connected to WiFi
    delay(1000);
    connection.loop();
  }
  else
  {
    mosquitto_connect();
  }
  /*#endif

  #ifdef MQTT_SSL
    Serial.println("\n==== MQTT SSL =============");
    //mosquitto.publish(MQTT_IOT_CHANNEL_1, itoa(temp, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_TEMPERATURE, itoa(sensors_values.temperature, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_PRESSURE, itoa(sensors_values.pressure / 100.0F, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_HUMIDITY, itoa(sensors_values.humidity, cstr, 10));
    connection.publish(MQTT_IOT_CHANNEL_0, "10");
    Serial.println("test_topic: 10");
    delay(500);
    connection.publish(MQTT_IOT_CHANNEL_0, "3");
    Serial.println("test_topic: 3");

    Serial.print("MQTT Status: ");
    Serial.println(connection.state());
    Serial.print("Connection tatus: ");
    Serial.println(WiFi.status());
    delay(500);
    connection.loop();
    #endif*/
}