#include "Automation.h"

Automation::Automation() {

}
void Automation::init() {

}

void Automation::SetOutput(int pin, boolean level) {}

// Dummy task blinking built-in LED
void Automation::IdleTaskCode (void * parameters) {
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