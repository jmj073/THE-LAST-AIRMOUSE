# if 0

#include <Arduino.h>
// #include <BleMouse.h>

// static BleMouse mouse;

void setup() {
    Serial.begin(115200);   
    // mouse.begin();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    static bool flag;

    Serial.printf("%d\n", (int)analogRead(_VBAT));
    digitalWrite(LED_BUILTIN, flag ^= 1);
    delay(500);

}

#endif