#if 0

#include <Arduino.h>

#include <MPU9250.h>
#include <MPU9250 calibrator.h>
#include <Wire.h>

#define my_assert(expr, fmt, ...)\
if (!(expr)) {\
    log_e(fmt, ##__VA_ARGS__);\
    while (1) { yield(); }\
}

#define MPU_INT 19

#define LED     LED_BUILTIN
#define LED_ON  LOW
#define LED_OFF HIGH

static MPU9250 mpu;

void setup() {
    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000);

    /* MPU9250 INIT */ {
        my_assert(mpu.testConnection(), "mpu connection failed! (%d)", (int)mpu.getDeviceID());
        mpu.reset();
        mpu.initialize();

        // mpu.setDLPFMode(6);
        // mpu.setAccelDLPFMode(6);
    }
}

#define MEAN_SIZE 1000

void loop() {
    delay(200);

    static MPU9250Calibrator calibrator(&mpu);

    calibrator.calibrate6();
}

#endif