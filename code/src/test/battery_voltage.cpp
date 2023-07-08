#if 0

#include <Arduino.h>
#include <cmath>
#include "utils.h"

using namespace std;

void setup() {
    Serial.begin(115200);
}

void loop() {
    auto value = analogRead(_VBAT);
    auto volt = value * 6.6 / 4096;
    Serial.printf("VBAT: %hu, ", value);
    Serial.printf("voltage: %hu, ", analogReadMilliVolts(_VBAT));
    Serial.printf("real: %f, ", volt);
    Serial.printf("percentage: %d", clamp<int>(123 - 123 / pow(1 + pow(volt / 3.7, 80), 0.165), 0, 100));
    Serial.println();
    delay(500);
}

#endif

#if 0

#include <Arduino.h>
#include <cmath>
#include "utils.h"
#include "settings.h"
#include <BleCombo.h>
#include "mouse_move/looper.h"

using namespace std;

static BleCombo combo(DEVICE_NAME);
static MouseMoveLooper mouse_move_looper(
    mouse_move::InputHandler(mouse_move::IMU),
    mouse_move::OutputHandler(&combo)
);

static inline
void busy_sleep_ms(unsigned long ms) {
    auto prev = millis();
    while (millis() - prev < ms);
}

void setup() {
    Serial.begin(115200);

    mouse_move_looper.getInputHandler().begin();
    combo.begin();
    while (!combo.isConnected()) yield();
}

void loop() {
    if (!combo.isConnected()) {
        ESP.restart();
    }

    static unsigned int prev_ms;
    auto curr_ms = millis();

    if (!prev_ms || curr_ms - prev_ms >= 500) {
        prev_ms = curr_ms;

        static int i;
        ++i %= 101;
        combo.setBatteryLevel(i);
        Serial.printf("change battery level: %d\r\n", i);
    }

    busy_sleep_ms(5);
}
#endif