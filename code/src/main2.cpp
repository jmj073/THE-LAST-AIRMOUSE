#if 0 /* FILE */
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

constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

static inline
float gyro_raw2degree(int16_t raw, uint32_t us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

static void handle_left_click(bool state);
static void handle_right_click(bool state);
static void handle_joystick_btn(bool state);
static void handle_imu();
static void do_nothing() { }
static void handle_joystick_for_move_mouse();
static void handle_joystick_for_keyboard();
static void move_mouse();

static void (*mouse_move_handler)() = handle_imu;
static void (*keyboard_handler)() = handle_joystick_for_keyboard;

static BleCombo combo;
static MPU9250 mpu;

static MyButton button_left(PIN_LEFT_CLICK, handle_left_click);
static MyButton button_right(PIN_RIGHT_CLICK, handle_right_click);
static MyButton button_joystick(PIN_JOYSTICK_BTN, handle_joystick_btn);

static double PITCH, YAW;
static unsigned long MOUSE_MOVE_PREV_US;
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
    MOUSE_MOVE_PREV_US = micros();
}

void loop() {
    if (!combo.isConnected()) {
        ESP.restart();
    }

    button_left.loop();
    button_right.loop();    
    button_joystick.loop();

    mouse_move_handler();
    keyboard_handler();

    move_mouse();

    
}





static
void move_mouse() {
    static uint32_t prev_ms;
    
    uint32_t curr_ms = millis();
    if (curr_ms - prev_ms >= 10) { // 10ms
        prev_ms = curr_ms;

        auto p = (signed char)std::clamp<int32_t>(PITCH, -128, 127);
        auto y = (signed char)std::clamp<int32_t>(YAW, -128, 127);

        PITCH -= p;
        YAW -= y;

        combo.move(-y, p);
    }
}

static
void handle_imu() {
    unsigned long curr_us = micros();

    unsigned long diff_us = curr_us - MOUSE_MOVE_PREV_US;
    MOUSE_MOVE_PREV_US = curr_us;

    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);

    PITCH   += gyro_raw2degree(gy, diff_us) * 16;
    YAW     += gyro_raw2degree(gz, diff_us) * 16;
}

/*
static
void handle_mouse_move() {
    if (mpu.getIntFIFOBufferOverflowStatus()) {
        mpu.resetFIFO();
        DEBUG_PRINTLN("fifo overflow!");
        MOUSE_MOVE_PREV_US = micros();
        return;
    }
    auto fifo_count = mpu.getFIFOCount();
    if (fifo_count < 4) return;

    digitalWrite(LED, LED_ON);

    uint32_t curr_us = micros();
    uint32_t diff_us = curr_us - MOUSE_MOVE_PREV_US;
    MOUSE_MOVE_PREV_US = curr_us;

    uint8_t data[4];
    mpu.getFIFOBytes(data, 4);

    int16_t gy, gz;
    gy = ((int16_t)data[0] << 8) | data[1];
    gz = ((int16_t)data[2] << 8) | data[3];

    PITCH   += gyro_raw2degree(gy, diff_us) * 16;
    YAW     += gyro_raw2degree(gz, diff_us) * 16;
}
*/

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
        mouse_move_handler = IMU_ENABLE ? handle_imu : handle_joystick_for_move_mouse;
        keyboard_handler = IMU_ENABLE ? handle_joystick_for_keyboard : do_nothing;
    }
}

static
void handle_joystick_for_move_mouse() {
    uint32_t curr_us = micros();

    uint32_t diff_us = curr_us - MOUSE_MOVE_PREV_US;
    MOUSE_MOVE_PREV_US = curr_us;
    
    int x = analogRead(PIN_JOYSTICK_X) - 1958;
    int y = analogRead(PIN_JOYSTICK_Y) - 2019;
    
    PITCH += (int64_t)diff_us * x / (1e6 * 3);
    YAW += (int64_t)diff_us * y / (1e6 * 3);
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