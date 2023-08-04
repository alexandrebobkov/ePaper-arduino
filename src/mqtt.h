#ifndef MQTT_H
#define MQTT_H

#include <SD.h>

// AWS IoT MQTT channels
#define AWS_IOT_CHANNEL_1 "iot/ch1"
#define AWS_IOT_CHANNEL_2 "iot/ch2"
#define AWS_IOT_CHANNEL_3 "iot/ch3"
#define AWS_IOT_CHANNEL_4 "iot/ch4"
#define AWS_IOT_CHANNEL_5 "iot/ch5"

// MQTT Mosquitto
#define MQTT_IOT_CHANNEL_TEMPERATURE "esp32/sensors/temperature"
#define MQTT_IOT_CHANNEL_PRESSURE "esp32/sensors/pressure"
#define MQTT_IOT_CHANNEL_HUMIDITY "esp32/sensors/humidity"

//#define MQTT_IOT_CHANNEL_SWITCH_1 "node1/output/sw1"
//#define MQTT_IOT_CHANNEL_SWITCH_2 "node1/output/sw2"
#define MQTT_IOT_CHANNEL_OUTPUT_PULSE       "node1/output/pulse"
#define MQTT_IOT_CHANNEL_OUTPUT_SWITCH_1    "node1/output/sw1"
#define MQTT_IOT_CHANNEL_OUTPUT_SWITCH_2    "node1/sw2"
#define MQTT_IOT_CHANNEL_OUTPUT_PWM_1       "node1/pwm1"

#define MQTT_IOT_CHANNEL_0 "test_topic"
#define MQTT_IOT_CHANNEL_1 "esp32/sensors/temp"
#define MQQT_IOT_CHANNEL_9 "esp32/output"

class Mosquitto
{
    public:
        void mosquitoCallBack (char* topic, byte* message, unsigned int length);
};

#endif