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


        mpu.setXAccelOffset(5573);
        mpu.setYAccelOffset(3483);
        mpu.setZAccelOffset(10176);
    }

    // pinMode(MPU_INT, INPUT);
    // attachInterrupt(MPU_INT, ISR, RISING);
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
    float roll, pitch;

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    roll = accel_raw2roll(ax, ay, az);
    pitch = accel_raw2pitch(ax, ay, az);
    
    Serial.print(roll); Serial.print(' ');
    Serial.print(pitch); Serial.println();
}

#endif /* FILE */