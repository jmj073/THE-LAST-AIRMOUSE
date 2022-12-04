#if 0

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

void setup() {
    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000);

    digitalWrite(LED, LED_OFF);
    pinMode(LED, OUTPUT);

    pinMode(MPU_INT, INPUT);

    my_assert(mpu.testConnection(), "mpu connection failed! (%d)", (int)mpu.getDeviceID());
    mpu.reset();
    mpu.initialize();
    mpu.setIntDataReadyEnabled(true);
    mpu.setInterruptLatch(true);
    mpu.setInterruptLatchClear(true);

    // Serial.println("ax ay az gx gy gz");

    delay(100);
}

void loop() {
    bool data_ready = digitalRead(MPU_INT);

    if (!data_ready) {
        digitalWrite(LED, LED_ON);
        return;
    }

    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);

    // Serial.printf("%hd ", gx);
    // Serial.printf("%hd ", gy);
    // Serial.printf("%hd ", gz);

    // Serial.println();
}

#endif