/*
   Copyright 2020 Scott Bezek and the splitflap contributors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#include "config.h"

#include "../core/splitflap_task.h"
#include "display_task.h"
#include "serial_task.h"

SplitflapTask splitflapTask(1, LedMode::AUTO);

#if ENABLE_DISPLAY
DisplayTask displayTask(splitflapTask, 0);
#endif

SerialTask serialTask(displayTask, splitflapTask, 0);


#ifdef CHAINLINK_BASE
#include "../base/base_supervisor_task.h"
BaseSupervisorTask baseSupervisorTask(splitflapTask, serialTask, 0);
#endif

// #if MQTT
// #include "mqtt_task.h"
// MQTTTask mqttTask(splitflapTask, serialTask, 0);
// #endif

// #if HTTP
// #include "http_task.h"
// HTTPTask httpTask(splitflapTask, displayTask, serialTask, 0);
// #endif

#if HTTP_SERVER
#include "http_server_task.h"
HTTPServerTask httpServerTask(splitflapTask, displayTask, serialTask, 0);
#endif


#include <EasyButton.h>

#define RESET_BUTTON_PIN 43

EasyButton reset_button(RESET_BUTTON_PIN);


void handle_reset() {
  serialTask.log("Resetting from button");
  splitflapTask.resetAll();
}


void buttonISR()
{
  reset_button.read();
}


void setup() {
    Serial.begin();
    delay(2000);
    // Serial.println("hello world");
    // delay(2000);

   splitflapTask.begin();

  #if ENABLE_DISPLAY
  displayTask.begin();
  #endif

    serialTask.begin();

  #if MQTT
  mqttTask.begin();
  #endif

  #if HTTP
  httpTask.begin();
  #endif

  #if HTTP_SERVER
  httpServerTask.begin();
  #endif

  #ifdef CHAINLINK_BASE
  baseSupervisorTask.begin();
  #endif

  reset_button.begin();
  reset_button.onPressed(handle_reset);

  reset_button.enableInterrupt(buttonISR);


  // Delete the default Arduino loopTask to free up Core 1
  vTaskDelete(NULL);
}


void loop() {
    // This loop is intentionally empty. All tasks are run in the background.
    assert(false);
}
