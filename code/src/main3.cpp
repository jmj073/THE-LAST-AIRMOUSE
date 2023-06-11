#if 1 /* FILE */
/* gyro raw data to degree(roll, pitch, yaw) */

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

constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

static inline
float gyro_raw2degree(int16_t raw, unsigned long us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

static void do_nothing() { }

static void handle_left_click(bool state);
static void handle_right_click(bool state);
static void handle_joystick_btn(bool state);

static void handle_joystick_for_keyboard();

static MouseMoveLooper::InputData handle_imu4move_mouse(unsigned long interval);

static void (*keyboard_handler)() = handle_joystick_for_keyboard;

static BleCombo combo;
static MPU9250 mpu;

static MyButton button_left(PIN_LEFT_CLICK, handle_left_click);
static MyButton button_right(PIN_RIGHT_CLICK, handle_right_click);
static MyButton button_joystick(PIN_JOYSTICK_BTN, handle_joystick_btn);

static MouseMoveLooper mouse_move_looper(handle_imu4move_mouse, &combo);

static bool IMU_ENABLE = true;

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

    keyboard_handler();

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
void handle_joystick_btn(bool state) {
    if (!state) {
        DEBUG_PRINTLN("joystick button pressed!");
        IMU_ENABLE = !IMU_ENABLE;
        // mouse_move_handler = IMU_ENABLE ? handle_imu : handle_joystick_for_move_mouse;
        mouse_move_looper.setInputHandler(IMU_ENABLE ? handle_imu4move_mouse : handle_joystick4move_mouse);
        keyboard_handler = IMU_ENABLE ? handle_joystick_for_keyboard : do_nothing;
    }
}

static
void handle_joystick_for_keyboard() {
    static char ad = 0;
    static char ws = 0;

    int x = analogRead(PIN_JOYSTICK_X) - 1958;
    int y = analogRead(PIN_JOYSTICK_Y) - 2019;

    char tmp_ws = x < -1000 ? 'w' : (x > 1000 ? 's' : 0);
    if (tmp_ws != ws) {
        if (ws) combo.release(ws);
        ws = tmp_ws;
        if (ws)combo.press(ws);
    }

    char tmp_ad = y < -1000 ? 'd' : (y > 1000 ? 'a' : 0);
    if (tmp_ad != ad) {
        if (ad) combo.release(ad);
        ad = tmp_ad;
        if (ad) combo.press(ad);
    }
}

#endif /* FILE */