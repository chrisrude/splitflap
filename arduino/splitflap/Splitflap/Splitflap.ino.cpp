# 1 "/var/folders/xd/x6ppp0s118v4jd6l7qw04djw0000gn/T/tmpntka9j2m"
#include <Arduino.h>
# 1 "/Users/rude/splitflap/arduino/splitflap/Splitflap/Splitflap.ino"
#include <Arduino.h>
#include <Wire.h>
void setup();
void loop();
#line 4 "/Users/rude/splitflap/arduino/splitflap/Splitflap/Splitflap.ino"
void setup() {
  Serial.begin(MONITOR_SPEED);

  Serial.println("hello world");
}

void loop() {
  while (1) {
# 23 "/Users/rude/splitflap/arduino/splitflap/Splitflap/Splitflap.ino"
    Serial.println("loop");
    delay(1000);
  }
}