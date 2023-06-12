#if 1 /* FILE */

// #define DEBUG
#include "settings.h"

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
// #include <BleMouse.h>
#include <BleCombo.h>

#include "pins.h"
#include "debug.h"
#include "utils.h"
#include "MyButton.h"
#include "mouse_move_looper.h"
#include "keyboard_looper.h"

constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

static inline
float gyro_raw2degree(int16_t raw, unsigned long us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}


static void handle_left_click(bool state);
static void handle_right_click(bool state);
static void handle_joystick_btn(bool state);
static MouseMoveLooper::InputData handle_imu4move_mouse(unsigned long interval);
static KeyboardLooper::InputData handle_joystick_for_keyboard(unsigned long);
static KeyboardLooper::InputData return_zero_for_keyboard(unsigned long) {
    return KeyboardLooper::InputData(0);
}

static BleCombo combo;
static MPU9250 mpu;
static MyButton button_left(PIN_LEFT_CLICK, handle_left_click);
static MyButton button_right(PIN_RIGHT_CLICK, handle_right_click);
static MyButton button_joystick(PIN_JOYSTICK_BTN, handle_joystick_btn);
static MouseMoveLooper mouse_move_looper(handle_imu4move_mouse, &combo);
static KeyboardLooper keyboard_looper(handle_joystick_for_keyboard, &combo);

enum class JoystickMode { KEYBOARD, MOUSE };
static JoystickMode joystick_mode = JoystickMode::KEYBOARD;

static
void setup_mpu() {
    if (!mpu.testConnection()) {
        DEBUG_PRINTF("mpu connection failed! (%d)\n", (int)mpu.getDeviceID());
    }
    mpu.reset();
    mpu.initialize();

    /* GYRO INIT */
    mpu.setFullScaleGyroRange(GY_FS_SEL);

    mpu.setXGyroOffsetUser(54);
    mpu.setYGyroOffsetUser(32);
    mpu.setZGyroOffsetUser(32);

    /* disable accelerometer */
    mpu.setStandbyXAccelEnabled(true);
    mpu.setStandbyYAccelEnabled(true);
    mpu.setStandbyZAccelEnabled(true);

    /* FIFO INIT */
    // mpu.setFIFOMode(true);
    // mpu.setFIFOEnabled(true);
    // mpu.resetFIFO();
    // // mpu.setXGyroFIFOEnabled(true);
    // mpu.setYGyroFIFOEnabled(true);
    // mpu.setZGyroFIFOEnabled(true);
}

void setup() {
    Serial.begin(115200);
    combo.begin();
    // combo.setDelay(0);

    Wire.begin();
    Wire.setClock(400000);

    button_left.set_debounce_time(POLL_MS);
    button_right.set_debounce_time(POLL_MS);
    button_joystick.set_debounce_time(POLL_MS);

    setup_mpu();

    while (!combo.isConnected());
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
MouseMoveLooper::InputData handle_imu4move_mouse(unsigned long interval) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);

    return {
        .yaw   = gyro_raw2degree(gz, interval) * 16,
        .pitch = gyro_raw2degree(gy, interval) * 16,
    };
}

static
MouseMoveLooper::InputData handle_joystick4move_mouse(unsigned long interval) {
    int x = analogRead(PIN_JOYSTICK_X) - 1958;
    int y = analogRead(PIN_JOYSTICK_Y) - 2019;

    return {
        .yaw = (int64_t)interval * y / (1e6f * 3),
        .pitch   = (int64_t)interval * x / (1e6f * 3),
    };
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
            mouse_move_looper.setInputHandler(handle_imu4move_mouse);
            keyboard_looper.setInputHandler(handle_joystick_for_keyboard);
            break;
        case JoystickMode::MOUSE:
            mouse_move_looper.setInputHandler(handle_joystick4move_mouse);
            keyboard_looper.setInputHandler(return_zero_for_keyboard);
            break;
    }
}

static
KeyboardLooper::InputData handle_joystick_for_keyboard(unsigned long) {
    int x = analogRead(PIN_JOYSTICK_X) - 1958;
    int y = analogRead(PIN_JOYSTICK_Y) - 2019;

    uint8_t key{};
    key |= x < -1000 ? KeyboardHandler::KEY_W 
        : (x > 1000 ? KeyboardHandler::KEY_S : 0);

    key |= y < -1000 ? KeyboardHandler::KEY_D
        : (y > 1000 ? KeyboardHandler::KEY_A : 0);

    return KeyboardLooper::InputData(key);
}

#endif /* FILE */