#include <Arduino.h>

#define BTN_LEFT    32
#define BTN_RIGHT   33

#define LED BUILTIN_LED

#define JOYSTICK_X      25
#define JOYSTICK_Y      26
#define JOYSTICK_BTN    15

// #define MPU_INT 15

static volatile int flag;

void left_pressed() {
    ++flag;
}

void joystick_pressed() {
    digitalWrite(LED, digitalRead(JOYSTICK_BTN));
}

void setup() {
    Serial.begin(115200);

    /* joystick */
    pinMode(JOYSTICK_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(JOYSTICK_BTN), joystick_pressed, CHANGE);

    /* left & right click */
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_LEFT), left_pressed, FALLING);

    pinMode(LED, OUTPUT);
}

void loop() {
    static int cnt;
    if (cnt < flag) {
        ++cnt;
        Serial.println(cnt);
    }


}