#if 0 /* FILE */
/* accel raw data to degree(roll, pitch, yaw) */

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
constexpr uint8_t AC_FS_SEL = MPU9250_ACCEL_FS_2;

constexpr float COMPLE_ALPHA = 0.96;

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
        mpu.setFullScaleAccelRange(AC_FS_SEL);

        mpu.setXGyroOffsetUser(54);
        mpu.setYGyroOffsetUser(32);
        mpu.setZGyroOffsetUser(32);

        mpu.setXAccelOffset(5573);
        mpu.setYAccelOffset(3483);
        mpu.setZAccelOffset(10176);
    }

    // pinMode(MPU_INT, INPUT);
    // attachInterrupt(MPU_INT, ISR, RISING);
}

static inline
float gyro_raw2degree(int16_t raw, uint32_t us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

static inline
float accel_raw2roll(int16_t ax, int16_t ay, int16_t az) {
    return degrees(atan2(ay, sqrt(sq(ax) + sq(az))));
}

static inline
float accel_raw2pitch(int16_t ax, int16_t ay, int16_t az) {
    return degrees(atan2(-ax, sqrt(sq(ay) + sq(az))));
}

void loop() {
    static uint32_t prev_us;
    static float roll, pitch, yaw;

    float groll, gpitch, gyaw;
    float aroll, apitch;

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    aroll   = accel_raw2roll(ax, ay, az);
    apitch  = accel_raw2pitch(ax, ay, az);

    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    uint32_t curr_us = micros();
    uint32_t diff_us = curr_us - prev_us;
    groll   = gyro_raw2degree(gx, diff_us) * (az >= 0 ? 1 : -1);
    gpitch  = gyro_raw2degree(gy, diff_us) * (az >= 0 ? 1 : -1);
    gyaw    = gyro_raw2degree(gz, diff_us);
    prev_us = curr_us;

    roll    = COMPLE_ALPHA * (roll + groll) + (1 - COMPLE_ALPHA) * aroll;
    pitch   = COMPLE_ALPHA * (pitch + gpitch) + (1 - COMPLE_ALPHA) * apitch;
    yaw     = (yaw + gyaw);
    // roll += groll;
    // pitch += gpitch;
    // yaw += gyaw;

    Serial.print(roll); Serial.print(' ');
    Serial.print(pitch); Serial.print(' ');
    Serial.print(yaw); Serial.println();
}

#endif /* FILE */