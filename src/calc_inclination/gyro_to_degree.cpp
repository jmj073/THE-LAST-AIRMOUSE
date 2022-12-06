#if 0 /* FILE */
/* gyro raw data to degree(roll, pitch, yaw) */

#include <Arduino.h>

#include <MPU9250.h>
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
constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

void setup() {
    Serial.begin(115200);

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

/** Raw data to degree
 * dps means degree per second
 * @param raw gyro raw data
 * @param us micro second
 * @return unit is degree
 */
static inline
float gyro_raw2degree(int16_t raw, uint32_t us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

void loop() {
    static float roll, pitch, yaw;
    static uint32_t prev_us;
    static bool is_first = true;

    uint32_t curr_us = micros();

    if (is_first) {
        is_first = false;
    } else {
        int16_t gx, gy, gz;
        uint32_t diff_us = curr_us - prev_us;
        mpu.getRotation(&gx, &gy, &gz);

        roll += gyro_raw2degree(gx, diff_us);
        pitch += gyro_raw2degree(gy, diff_us);
        yaw += gyro_raw2degree(gz, diff_us);
    }

    Serial.print(roll); Serial.print(' ');
    Serial.print(pitch); Serial.print(' ');
    Serial.print(yaw); Serial.print(' ');
    Serial.println();

    prev_us = curr_us;
}

#endif /* FILE */