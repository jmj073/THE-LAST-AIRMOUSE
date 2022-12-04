#if 0

#include <Arduino.h>
#include <Wire.h>

#include <MPU9250.h>

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

    digitalWrite(LED, LED_OFF);
    pinMode(LED, OUTPUT);

    pinMode(MPU_INT, INPUT);

    /* MPU9250 INIT */ {
        my_assert(mpu.testConnection(), "mpu connection failed! (%d)", (int)mpu.getDeviceID());
        mpu.reset();
        mpu.initialize();

        /* DLPF */
        mpu.setDLPFMode(6);
        mpu.setAccelDLPFMode(6);

        /* FIFO */
        mpu.resetFIFO();
        mpu.setFIFOEnabled(true);

        /* DMP */
        mpu.setDMPEnabled(true);

        /* I2C MASTER */
        mpu.setI2CMasterModeEnabled(true);
    }

    // Serial.println("ax ay az gx gy gz");
}

void loop() {

    uint16_t fifo_cnt = mpu.getFIFOCount();

    if (fifo_cnt < 6) {
        return;
    }

    uint8_t buf[6];
    mpu.getFIFOBytes(buf, 6);

    for (uint8_t j = 0; j < 6; j += 2) {
        Serial.print(int16_t((buf[j] << 8) | buf[j + 1]));
        Serial.print(' ');
    }
    Serial.println();
}

#endif