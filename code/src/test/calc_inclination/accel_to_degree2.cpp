#if 0 /* FILE */
/* accel raw data to degree(roll, pitch, yaw) */

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
#include <cmath>

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

        mpu.setXGyroOffsetUser(42);
        mpu.setYGyroOffsetUser(45);
        mpu.setZGyroOffsetUser(28);

        mpu.setXAccelOffset(5583);
        mpu.setYAccelOffset(3458);
        mpu.setZAccelOffset(10169);
    }

    // pinMode(MPU_INT, INPUT);
    // attachInterrupt(MPU_INT, ISR, RISING);
}

static inline
float gyro_raw2degree(int16_t raw, uint32_t us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6f;
}

static inline
float gyro_raw2radian(int16_t raw, uint32_t us) {
    return radians(gyro_raw2degree(raw, us));
}

static inline
float accel_raw2roll(int16_t ax, int16_t ay, int16_t az) {
    return atan2(ay, sqrt(sq(ax) + sq(az)));
}

static inline
float accel_raw2roll2(int16_t ax, int16_t ay, int16_t az) {
    float tmp = atan2(ay, sqrt(sq(ax) + sq(az)));
    return az >= 0 ? tmp : PI - tmp;
}

static inline
float accel_raw2pitch(int16_t ax, int16_t ay, int16_t az) {
    return atan2(-ax, sqrt(sq(ay) + sq(az)));
}

static inline
float accel_raw2pitch2(int16_t ax, int16_t ay, int16_t az) {
    float tmp = abs(atan2(-ax, sqrt(sq(ay) + sq(az))));
    return az >= 0 ? tmp : PI - tmp;
}

void loop() {
    static float groll, gpitch, gyaw;
    static bool is_first = true;
    static uint32_t prev_us;

    /* accel */
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    // float roll   = accel_raw2roll(ax, ay, az);
    float pitch  = accel_raw2pitch(ax, ay, az);
    float roll2  = accel_raw2roll2(ax, ay, az);
    // float pitch2 = accel_raw2pitch2(ax, ay, az);

    uint32_t curr_us = micros();
    if (is_first) {
        is_first = false;
        gpitch = pitch;
    } else {
        int16_t gx, gy, gz;
        uint32_t diff_us = curr_us - prev_us;
        mpu.getRotation(&gx, &gy, &gz);

        float dx = gyro_raw2radian(gx, diff_us);
        float dy = gyro_raw2radian(gy, diff_us);
        float dz = gyro_raw2radian(gz, diff_us);

        gpitch += cos(roll2) * dy + cos(roll2 + PI) * dz;
        // groll  += cos(pitch2) * dx + cos(pitch2 + HALF_PI) * dz;
    }
    prev_us = curr_us;

    /* print */
    // Serial.print(' '); Serial.print(degrees(pitch)); 
    // Serial.print(' '); Serial.print(degrees(roll));
    // Serial.print(' '); Serial.print(degrees(gpitch));
    // Serial.print(' '); Serial.print(degrees(groll));

    // Serial.print(' '); Serial.print(degrees(pitch2));
    Serial.print(' '); Serial.print(degrees(roll2));

    Serial.println();
}

#endif /* FILE */
