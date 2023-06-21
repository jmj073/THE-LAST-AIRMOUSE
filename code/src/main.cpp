#if 1 /* FILE */

#include "settings.h"

#include <Arduino.h>
#include <MPU9250.h>
#include <Wire.h>
#include <BleCombo.h>

#include "pins.h"
#include "debug.h"
#include "utils.h"
#include "MyButton.h"
#include "mouse_move_looper.h"
#include "keyboard_looper.h"

static void handle_left_click(bool state);
static void handle_right_click(bool state);
static void handle_joystick_btn(bool state);

static BleCombo combo;
static MyButton button_left(PIN_LEFT_CLICK, handle_left_click);
static MyButton button_right(PIN_RIGHT_CLICK, handle_right_click);
static MyButton button_joystick(PIN_JOYSTICK_BTN, handle_joystick_btn);
static MouseMoveLooper mouse_move_looper(
    mouse_move::InputHandler(mouse_move::IMU),
    mouse_move::OutputHandler(&combo)
);
static KeyboardLooper keyboard_looper({}, keyboard::OutputHandler(&combo));

enum class JoystickMode { KEYBOARD, MOUSE };
static JoystickMode joystick_mode = JoystickMode::KEYBOARD;

void setup() {
    Serial.begin(115200);
#ifdef DEBUG
    Serial.setDebugOutput(true);
#endif /* DEBUG*/
    combo.begin();
    mouse_move_looper.getInputHandler().begin();

    button_left.set_debounce_time(POLL_MS);
    button_right.set_debounce_time(POLL_MS);
    button_joystick.set_debounce_time(POLL_MS);
    keyboard_looper.getInputHandler().inputEnable();

    while (!combo.isConnected()) yield();
}

void loop() {
    if (!combo.isConnected()) {
        ESP.restart();
    }

    button_left.loop();
    button_right.loop();
    button_joystick.loop();

    keyboard_looper.loop();

    mouse_move_looper.loop();
    mouse_move_looper.getOutputHandler().moveMouse();
}

static
void handle_left_click(bool state) {
    if (state) {
        combo.mouseRelease(MOUSE_LEFT);
    } else {
        DEBUG_PRINTLN("left pressed!");
        combo.mousePress(MOUSE_LEFT);
    }
}

static
void handle_right_click(bool state) {
    if (state) {
        combo.mouseRelease(MOUSE_RIGHT);
    } else {
        DEBUG_PRINTLN("right pressed!");
        combo.mousePress(MOUSE_RIGHT);
    }
}

static
void toggle_joystick_mode() {
    switch (joystick_mode) {
        case JoystickMode::KEYBOARD:
            joystick_mode = JoystickMode::MOUSE;
            break;
        case JoystickMode::MOUSE:
            joystick_mode = JoystickMode::KEYBOARD;
            break;
    }
}

static
void handle_joystick_btn(bool state) {
    if (state) return;

    DEBUG_PRINTLN("joystick button pressed!");

    toggle_joystick_mode();

    switch (joystick_mode) {
        case JoystickMode::KEYBOARD:
            mouse_move_looper.getInputHandler().setInputMode(mouse_move::IMU);
            keyboard_looper.getInputHandler().inputEnable();
            break;
        case JoystickMode::MOUSE:
            mouse_move_looper.getInputHandler().setInputMode(mouse_move::JOYSTICK);
            keyboard_looper.getInputHandler().inputDisable();
            break;
    }
}

#endif /* FILE */