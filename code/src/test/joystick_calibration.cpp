#if 0

#include <Arduino.h>
#include "pins.h"
#include "joystick.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    long x{}, y{};

    for (int i = 0; i < 1000; ++i) {
        x += analogRead(PIN_JOYSTICK_X);
        y += analogRead(PIN_JOYSTICK_Y);
        // x += joystickGetX();
        // y += joystickGetY();
    }

    Serial.printf("%ld %ld\r\n", x / 1000, y / 1000);
    delay(1000);
}

#endif