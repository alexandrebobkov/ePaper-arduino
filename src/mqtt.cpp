#include "mqtt.h"
//#include "automation-0.h"

void Mosquitto::mosquito_callback (char* topic, byte* message, unsigned int length) {
  //Serial.print("\nMessage arrived on topic: ");
  //Serial.print(topic);
  //Serial.print(". Message: ");
  String messageTemp;

  /*for (int i=0; i < length; i++)
  {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();*/

  if (String(topic) == "node1/output/sw1")
  {
    if (messageTemp == "on")
    {
      Serial.println("Switch 1 ON\n");
      digitalWrite(14, LOW);
    }
    else if (messageTemp == "off")
    {
      Serial.println("Switch 1 OFF\n");
      digitalWrite(14, HIGH);
    }
  }
  if (String(topic) == "node1/output/sw2")
  {
    if (messageTemp == "on")
    {
      Serial.println("Switch 2 ON\n");
      digitalWrite(12, LOW);
    }
    else if (messageTemp == "off")
    {
      Serial.println("Switch 2 OFF\n");
      digitalWrite(12, HIGH);
    }
  }
}