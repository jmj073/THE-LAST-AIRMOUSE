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
#include "mouse_move/looper.h"
#include "keyboard_looper.h"
#include "measure.h"

#ifdef ENABLE_PROTOTYPE
    #define DEVICE_NAME "prototype"
#else /* ENABLE_PROTOTYPE */
    #define DEVICE_NAME "M416D"
#endif /* ENABLE_PROTOTYPE */

static inline
void battery_level_refresh_periodically();
static void handle_left_click(bool released);
static void handle_right_click(bool released);
static void handle_joystick_btn(bool released);

static BleCombo combo(DEVICE_NAME);
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

    battery_level_refresh_periodically();
}

static
void handle_left_click(bool released) {
    if (released) {
        combo.mouseRelease(MOUSE_LEFT);
    } else {
        DEBUG_PRINTLN("left pressed!");
        combo.mousePress(MOUSE_LEFT);
    }
}

static
void handle_right_click(bool released) {
    if (released) {
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
void handle_joystick_btn(bool released) {
    if (released) return;

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

static inline
int get_battery_level() {
    return 50;
}

static inline
void battery_level_refresh() {
    combo.setBatteryLevel(get_battery_level());
}

static inline constexpr
unsigned long min2ms(unsigned long minute) {
    return minute * 60 * int(1e3);
}

static inline
void battery_level_refresh_periodically() {
    static unsigned long prev_ms;
    auto curr_ms = millis();

    if (curr_ms - prev_ms < min2ms(5)) return;
    battery_level_refresh();
}

#endif /* FILE */