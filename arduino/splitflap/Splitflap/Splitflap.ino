#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(MONITOR_SPEED);

  Serial.println("hello world");
}

void loop() {
  while (1) {
// #if SENSOR_TEST
    // sensor_test_iteration();
// #else
//     run_iteration();
// #endif

    // #if defined(ESP8266) || defined(ESP32)
    // // Yield to avoid triggering Soft WDT
    // yield();
    // #endif

    Serial.println("loop");
    delay(1000);
  }
}
