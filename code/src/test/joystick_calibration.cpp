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
        // x += joystickGetX();
        // y += joystickGetY();
    }

    Serial.printf("%ld %ld\r\n", x / 1000, y / 1000);
    delay(1000);
}

#endif

#if 0

#include "settings.h"
#include <Arduino.h>
#include <BleCombo.h>
#include "joystick.h"
#include "measure.h"

static BleCombo combo(DEVICE_NAME);

void setup() {
    Serial.begin(115200);
    MyJoystick::getInstance().begin();
    // Serial.println(analogRead(_VBAT));
    combo.begin();
    while (!combo.isConnected()) yield();
}

void loop() {
    if (!combo.isConnected()) {
        ESP.restart();
    }

    auto& joystick = MyJoystick::getInstance();

    static Measure<int> measurex(1000);
    static Measure<int> measurey(1000);
    measurex.appendValue(abs(joystick.getx()));
    measurey.appendValue(abs(joystick.gety()));

    static unsigned long prev_ms;
    auto curr_ms = millis();
    if (curr_ms - prev_ms >= 20) {
        prev_ms = curr_ms;
        combo.move(0, 0);
    }
}

#endif