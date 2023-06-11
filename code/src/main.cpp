#if 0 /* FILE */
/* gyro raw data to degree(roll, pitch, yaw) */

// #define DEBUG

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
#include <BleMouse.h>
// #include <BleKeyboard.h>

#include "pins.h"
#include "debug.h"
#include "utils.h"
#include "MyButton.h"

#define LED     LED_BUILTIN
#define LED_ON  LOW
#define LED_OFF HIGH

#define POLL_MS 15

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

// static BleKeyboard keyboard;
static BleMouse mouse;
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

static
void setup_pin() {
    digitalWrite(LED, LED_OFF);
    pinMode(LED, OUTPUT);

    pinMode(PIN_LEFT_CLICK, INPUT_PULLUP);
    pinMode(PIN_RIGHT_CLICK, INPUT_PULLUP);
    pinMode(PIN_JOYSTICK_BTN, INPUT_PULLUP);
}

void setup() {
    Serial.begin(115200);
    mouse.begin();
    // keyboard.begin();

    Wire.begin();
    Wire.setClock(400000);

    setup_pin();
    setup_mpu();

    while (!mouse.isConnected());
    MOUSE_MOVE_PREV_US = micros();
}

void loop() {
    if (!mouse.isConnected()) {
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

        mouse.move(-y, p);
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
        mouse.release(MOUSE_LEFT);
    } else {
        DEBUG_PRINTLN("left pressed!");
        mouse.press(MOUSE_LEFT);
    }
}

static
void handle_right_click(bool state) {
    if (state) {
        mouse.release(MOUSE_LEFT);
    } else {
        DEBUG_PRINTLN("left pressed!");
        mouse.press(MOUSE_LEFT);
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

    if (x < -1000) {
        if (ws == 's') Serial.println("release s");
        if (ws != 'w') Serial.println("press w");
        ws = 'w';
    } else if (x > 1000) {
        if (ws == 'w') Serial.println("release w");
        if (ws != 's') Serial.println("press s");
        ws = 's';
    } else {
        if (ws) Serial.printf("release %c\n", ws);
        ws = 0;
    }

    if (y < -1000) {
        if (ad == 'a') Serial.println("release a");
        if (ad != 'd') Serial.println("press d");
        ad = 'd';
    } else if (y > 1000) {
        if (ad == 'd') Serial.println("release d");
        if (ad != 'a') Serial.println("press a");
        ad = 'a';
    } else {
        if (ad) Serial.printf("release %c\n", ad);
        ad = 0;
    }
}

#endif /* FILE */