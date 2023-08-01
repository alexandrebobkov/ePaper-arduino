#include "mqtt.h"
//#include "automation-0.h"

void Mosquitto::mosquitoCallBack (char* topic, byte* message, unsigned int length) {
  Serial.print("\nMessage arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String messageTemp;

  for (int i=0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  //if (String(topic) == MQTT_IOT_CHANNEL_OUTPUT_SWITCH_1) {
  if (strcmp(topic, MQTT_IOT_CHANNEL_OUTPUT_SWITCH_1)) {
    if (messageTemp == "on") {
      Serial.println("Switch 1 ON\n");
      digitalWrite(12, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("Switch 1 OFF\n");
      digitalWrite(12, LOW);
    }
  }
  if (String(topic) == MQTT_IOT_CHANNEL_OUTPUT_SWITCH_2) {
    if (messageTemp == "on") {
      Serial.println("Switch 2 ON\n");
      digitalWrite(14, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("Switch 2 OFF\n");
      digitalWrite(14, LOW);
    }
  }
}