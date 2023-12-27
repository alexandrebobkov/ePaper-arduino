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

const uint32_t partial_update_period_s = 2;
const uint32_t full_update_period_s = 1*60*60;
uint32_t previous_time, next_time, total_seconds, seconds, minutes, hours, days, actual;

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

void TaskRefreshDisplayCode (void* parameters) {
  for (;;) {

    display.update();
    vTaskDelay(50000);
  }
}

void TaskUpdateDisplayCode (void* parameters) {
  for (;;) {
    actual = millis();
    previous_time = actual;
    next_time += uint32_t(partial_update_period_s * 1000);
    total_seconds += partial_update_period_s;
    seconds = total_seconds % 60;
    minutes = (total_seconds / 60) % 60;
    hours = (total_seconds / 3600) % 24;
    days = (total_seconds / 3600) / 24;
    uint16_t box_x = 10;
    uint16_t box_y = 50;
    uint16_t box_w = 170;
    uint16_t box_h = 70;
    uint16_t cursor_y = box_y + 16;
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(10, 20);
    display.println("Workdesk Automation");
    display.setTextColor(GxEPD_BLACK);
    display.drawRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.setCursor(box_x, cursor_y);
    display.print(days); display.print("d "); print02d(hours); display.print(":"); print02d(minutes); display.print(":"); print02d(seconds);
    display.update();
    vTaskDelay(10000);
  }
}

void setup() {

  Serial.begin(115200);

  // Setup built-in LED and assign status LED task
  pinMode(buildinLED, OUTPUT);
  xTaskCreatePinnedToCore(TaskStatusLEDCode, "Status LED", 1000, NULL, 2, &TaskStatusLED, 0);

  display.init(115200); // enable diagnostic output on Serial
  display.fillScreen(GxEPD_WHITE);

  xTaskCreatePinnedToCore(TaskUpdateDisplayCode, "Update ePaper", 1000, NULL, 2, &TaskUpdateDisplay, 0);
  //xTaskCreatePinnedToCore(TaskRefreshDisplayCode, "Refresh ePaper", 1000, NULL, 2, &TaskRefreshDisplay, 0);
  interval = 250;
  Serial.println("setup done");
}

void loop() {}

void print02d(uint32_t d)
{
  if (d < 10) display.print("0");
  display.print(d);
}