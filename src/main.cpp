/*
  Written for a development board Espressif DevKit ESP32-WROOM-32
    
  ePaper display.
  Adopted & written by: Alexander Bobkov
  
  Dec 27, 2023
*/
#include <Arduino.h>
#include "ePaper.h"

// Define tasks
TaskHandle_t TaskStatusLED;
TaskHandle_t TaskUpdateDisplay, TaskRefreshDisplay;

// Define GPIO pins
const int buildinLED  = 2;
int interval = 150;

uint16_t box_x = 10;
uint16_t box_y = 50;
uint16_t box_w = 170;
uint16_t box_h = 70;

void print02d(uint32_t d);

void TaskStatusLEDCode (void* parameters) {
  for (;;) {
    digitalWrite(buildinLED, HIGH);
    vTaskDelay(interval);
    digitalWrite(buildinLED, LOW);
    vTaskDelay(interval);
    digitalWrite(buildinLED, HIGH);
    vTaskDelay(interval);
    digitalWrite(buildinLED, LOW);
    vTaskDelay(interval*6);                
  }
}

void setup() {

  Serial.begin(115200);

  // Setup built-in LED and assign status LED task
  pinMode(buildinLED, OUTPUT);
  xTaskCreatePinnedToCore(TaskStatusLEDCode, "Status LED", 1000, NULL, 2, &TaskStatusLED, 0);

  display.init(115200); // enable diagnostic output on Serial
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_RED);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(10, 20);
  display.println("Workdesk Automation");
  display.setTextColor(GxEPD_BLACK);
  display.drawRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
  display.update();

  interval = 250;
  Serial.println("setup done");
}

void loop() {}