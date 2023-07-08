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

#define ACCEL_OFFSET_X 5583
#define ACCEL_OFFSET_Y 3458
#define ACCEL_OFFSET_Z 10169

static MPU9250 mpu;
constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;
constexpr uint8_t AC_FS_SEL = MPU9250_ACCEL_FS_2;

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


        mpu.setXAccelOffset(ACCEL_OFFSET_X);
        mpu.setYAccelOffset(ACCEL_OFFSET_Y);
        mpu.setZAccelOffset(ACCEL_OFFSET_Z);
    }

    // pinMode(MPU_INT, INPUT);
    // attachInterrupt(MPU_INT, ISR, RISING);
}

static inline
float accel_raw2roll(int16_t ax, int16_t ay, int16_t az) {
    return atan2(ay, sqrt(sq(ax) + sq(az)));
}

static inline
float accel_raw2roll2(int16_t ax, int16_t ay, int16_t az) {
    auto d = sqrt(sq(ax) + sq(az));
    return atan2(ay, az < 0 ? -d : d);
}

static inline
float accel_raw2roll3(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2roll2(ax, ay, az);
    return tmp + (tmp < 0 ? TWO_PI : 0); // mod
}

static inline
float accel_raw2pitch(int16_t ax, int16_t ay, int16_t az) {
    return atan2(-ax, sqrt(sq(ay) + sq(az)));
}

static inline
float accel_raw2pitch2(int16_t ax, int16_t ay, int16_t az) {
    auto d = sqrt(sq(ay) + sq(az));
    return atan2(-ax, az < 0 ? -d : d);
}

static inline
float accel_raw2pitch3(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2pitch2(ax, ay, az);
    return tmp + (tmp < 0 ? TWO_PI : 0); // mod
}

void loop() {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    // Serial.print(accel_raw2roll(ax, ay, az)); Serial.print(' ');
    // Serial.print(accel_raw2pitch(ax, ay, az)); Serial.print(' ');

    Serial.print(degrees(accel_raw2roll3(ax, ay, az))); Serial.print(' ');
    Serial.print(degrees(accel_raw2pitch3(ax, ay, az))); Serial.print(' ');

    // Serial.print(ay); Serial.print(' ');
    // Serial.print(az); Serial.print(' ');
    
    Serial.println();
}

#endif /* FILE */