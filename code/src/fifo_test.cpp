#if 0

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>

#include "utils.h"

#define my_assert(expr, fmt, ...)\
if (!(expr)) {\
    log_e(fmt, ##__VA_ARGS__);\
    while (1) { yield(); }\
}

#define MPU_INT 19

#define LED     LED_BUILTIN
#define LED_ON  LOW
#define LED_OFF HIGH

#define PACKET_SIZE 12

static inline void led_refresh();
static inline void led_loop();

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

        // mpu.setDLPFMode(1);
        // mpu.setAccelDLPFMode(6);

        mpu.resetFIFO();
        mpu.setFIFOMode(true);
        mpu.setFIFOEnabled(true);
        // mpu.setAccelFIFOEnabled(true);
        mpu.setXGyroFIFOEnabled(true);
        mpu.setYGyroFIFOEnabled(true);
        mpu.setZGyroFIFOEnabled(true);
        mpu.resetFIFO();
    }

    // Serial.println("ax ay az gx gy gz");
}

#if 1
void loop() {
//    static Measure measure(10,
//        [&] (unsigned long time) {
//            Serial.printf("measure: %lu\r\n", time);
//        });
    // led_loop();
    auto fifo_cnt = mpu.getFIFOCount();

    if (fifo_cnt == 512 || mpu.getIntFIFOBufferOverflowStatus()) {
        mpu.resetFIFO();
        // led_refresh();
        return;
    }
    
    if (fifo_cnt < PACKET_SIZE) {
        return;
    }

    uint8_t buf[PACKET_SIZE];
    mpu.getFIFOBytes(buf, sizeof(buf));

    static unsigned long prev;
    auto curr = millis();
    if (curr - prev < 100) return;
    prev = curr;

    // measure.start();
    for (int j = 0; j < 6; j += 2) {
        Serial.print(int16_t((buf[j] << 8) | buf[j + 1]));
        Serial.print(' ');
    }
    Serial.println();
    // measure.stop();
}
#endif

#if 0
void loop() {
    static constexpr const
    int16_t threshold = 500;

    if (mpu.getIntFIFOBufferOverflowStatus()) {
        mpu.resetFIFO();
        return;
    }

    auto fifo_cnt = mpu.getFIFOCount();
    if (fifo_cnt < PACKET_SIZE) {
        return;
    }

    uint8_t buf[PACKET_SIZE];
    mpu.getFIFOBytes(buf, sizeof(buf));

    for (int j = 6; j < sizeof(buf); j += 2) {
        int16_t data = (buf[j] << 8) | buf[j + 1];
        if (abs(data) > threshold) {
            digitalWrite(LED, LED_ON);
            delayMicroseconds(500);
            digitalWrite(LED, LED_OFF);
        }
    }
}
#endif

static unsigned long led_prev;

static inline
void led_refresh() {
    led_prev = millis();
    digitalWrite(LED, LED_ON);
}

static inline
void led_loop() {
    if (digitalRead(LED) != LED_ON) return;
    if (millis() - led_prev >= 100) {
        digitalWrite(LED, LED_OFF);
    }
}

#endif