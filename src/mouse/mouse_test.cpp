#if 1 /* FILE*/

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
        mouse.move(127, -100);
    }
    delay(2000);
}

#endif /* FILE*/