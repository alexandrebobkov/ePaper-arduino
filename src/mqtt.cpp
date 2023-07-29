#include "mqtt.h"

void Mosquitto::mosquito_callback (char* topic, byte* message, unsigned int length) {
  //Serial.print("\nMessage arrived on topic: ");
  //Serial.print(topic);
  //Serial.print(". Message: ");
  String messageTemp;

  for (int i=0; i < length; i++)
  {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  if (String(topic) == "esp32/output")
  {
    //Serial.print("Main. Changing output to: ");
    if (messageTemp == "on")
    {
      //Serial.println("Turn switch ON!\n");
      //digitalWrite(SWITCH_1, LOW);    // Active level LOW
    }
    else if (messageTemp == "off")
    {
      //Serial.println("Turn switch OFF!\n");
      //digitalWrite(SWITCH_1, HIGH);
    }
  }
}