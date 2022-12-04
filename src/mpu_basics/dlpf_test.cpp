#if 0

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>

static MPU9250 mpu;

void setup() {
    // esp_log_level_set("*", ESP_LOG_NONE);

    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000);

    mpu.initialize();
    if (!mpu.testConnection()) {
        log_e("mpu connection failed! (%d)", (int)mpu.getDeviceID());
        while (1) { yield(); }
    }

    mpu.setDLPFMode(6);
}

void loop() {
    // int16_t ax, ay, az, gx, gy, gz;
    // mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);

    // Serial.printf("%hd ", ax);
    // Serial.printf("%hd ", ay);
    // Serial.printf("%hd ", az);

    Serial.printf("%hd ", gx);
    Serial.printf("%hd ", gy);
    Serial.printf("%hd ", gz);

    Serial.println();
}

#endif