#if 0 /* FILE*/

#include <Arduino.h>
#include <BleMouse.h>

static BleMouse  mouse;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    mouse.begin();
}

void loop() {
    if(mouse.isConnected()) {
        mouse.move(10, -10);
    }
    delay(500);
}

#endif /* FILE*/