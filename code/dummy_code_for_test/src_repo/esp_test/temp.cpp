#if 0

#include <Arduino.h>

#define LED BUILTIN_LED

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
}

void loop() {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
}

#endif