#if 0

#include <Arduino.h>
#include "pins.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    long x{}, y{};

    for (int i = 0; i < 1000; ++i) {
        x += analogRead(PIN_JOYSTICK_X);
        y += analogRead(PIN_JOYSTICK_Y);
    }

    Serial.printf("%ld %ld\r\n", x, y);
    delay(1000);
}

#endif