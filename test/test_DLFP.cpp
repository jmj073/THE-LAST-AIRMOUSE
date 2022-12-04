#include <Arduino.h>
#include <unity.h>
#include <unity_config.h>
#include <Wire.h>
#include <MPU9250.h>

#define MPU_INT 19

#define LED     LED_BUILTIN
#define LED_ON  LOW
#define LED_OFF HIGH

static MPU9250 mpu;

void setUp() {
    mpu.reset();
    mpu.initialize();
}

void tearDown() {
    
}

void test_DLFP_none() {
    mpu.setIntDataReadyEnabled(true);

    yield();

    int cnt = 1000, prev_ready = 0;
    uint32_t prev_tick = micros();
    
    while (1) {
        int data_ready = digitalRead(MPU_INT);

        if (!prev_ready && data_ready) {
            if (!--cnt) break;
        }

        prev_ready = data_ready;
    }

    uint32_t diff_tick = micros() - prev_tick;
    Serial.printf("test_DLFP_none: %lu\r\n", diff_tick);
}

void test_DLFP() {
    mpu.setIntDataReadyEnabled(true);
    mpu.setDLPFMode(6);

    yield();

    int cnt = 1000, prev_ready = 0;
    uint32_t prev_tick = micros();
    
    while (1) {
        int data_ready = digitalRead(MPU_INT);

        if (!prev_ready && data_ready) {
            if (!--cnt) break;
        }

        prev_ready = data_ready;
    }

    uint32_t diff_tick = micros() - prev_tick;
    Serial.printf("test_DLFP_none: %lu", diff_tick);
}

void setup() {
    delay(2000);

    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    mpu.testConnection();

    UNITY_BEGIN();

    RUN_TEST(test_DLFP_none);
    RUN_TEST(test_DLFP);

    UNITY_END();
}

void loop() {

}