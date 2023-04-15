#if 1 /* FILE */
/* gyro raw data to degree(roll, pitch, yaw) */

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
#include <BleMouse.h>
#include <algorithm>

#define my_assert(expr, fmt, ...)\
if (!(expr)) {\
    log_e(fmt, ##__VA_ARGS__);\
    while (1) { yield(); }\
}

#define MPU_INT 15

#define LED     LED_BUILTIN
#define LED_ON  LOW
#define LED_OFF HIGH

template<typename _Tp>
constexpr const _Tp&
clamp(const _Tp& __val, const _Tp& __lo, const _Tp& __hi)
{
    __glibcxx_assert(!(__hi < __lo));
    return (__val < __lo) ? __lo : (__hi < __val) ? __hi : __val;
}

void mpu_loop();

static BleMouse mouse;
static MPU9250 mpu;
constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

void setup() {
    Serial.begin(115200);

    mouse.begin();

    Wire.begin();
    Wire.setClock(400000);

    digitalWrite(LED, LED_OFF);
    pinMode(LED, OUTPUT);

    /* MPU9250 INIT */ {
        my_assert(mpu.testConnection(), "mpu connection failed! (%d)", (int)mpu.getDeviceID());
        mpu.reset();
        mpu.initialize();

        mpu.setFullScaleGyroRange(GY_FS_SEL);

        mpu.setXGyroOffsetUser(54);
        mpu.setYGyroOffsetUser(32);
        mpu.setZGyroOffsetUser(32);
    }



    // pinMode(MPU_INT, INPUT);
    // attachInterrupt(MPU_INT, ISR, RISING);
}

void loop() {
    static uint32_t prev_us, tick_us;
    static float pitch, yaw;
    static bool is_first = true;

    if (mouse.isConnected()) {

        if (is_first) {
            is_first = false;
            pitch = yaw = 0;
            tick_us = prev_us = micros();
            return;
        }

        uint32_t curr_us = micros();
        uint32_t diff_us = curr_us - prev_us;
        prev_us = curr_us;

        int16_t gx, gy, gz;
        mpu.getRotation(&gx, &gy, &gz);

        pitch   += gyro_raw2degree(gy, diff_us) * 8;
        yaw     += gyro_raw2degree(gz, diff_us) * 8;

        if (curr_us - tick_us >= uint32_t(1e4)) { // 10ms
            tick_us = curr_us;

            auto p = (signed char)clamp<int32_t>(pitch, -128, 127);
            auto y = (signed char)clamp<int32_t>(yaw, -128, 127);

            pitch -= p;
            yaw -= y;

            mouse.move(-y, p);
        }

    } else {
        is_first = true;
    }

}

#endif /* FILE */