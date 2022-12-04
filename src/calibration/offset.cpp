#if 1

#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
#include <BleMouse.h>

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

void IRAM_ATTR ISR_FIFO_OVF() {
	digitalWrite(LED, LED_ON);
}

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

        /* DLPF */
        mpu.setDLPFMode(6);
        mpu.setAccelDLPFMode(6);

        /* INT */
        mpu.setIntFIFOBufferOverflowEnabled(true);
        mpu.setInterruptLatch(true);

        /* OFFSET
         * 현재 설정된 full scale이 어떻든 offset은 16G 기준이다.
         * 따라서 15bit기준, 각 스텝당 0.9765625mG이다(LSB는 reserved).
         * 마이 센서는 기존 값이 (5622, 3472, 9986)이다.
         */
        mpu.setXAccelOffset(mpu.getXAccelOffset() + 0);
        mpu.setYAccelOffset(mpu.getYAccelOffset() + 0);
        mpu.setZAccelOffset(mpu.getZAccelOffset() + -2048);

        /* FIFO */
        mpu.resetFIFO();
        mpu.setFIFOMode(true);
        mpu.setFIFOEnabled(true);

        #define FIFO_DATA_SIZE (3 * 2)
        mpu.setAccelFIFOEnabled(true);
        // mpu.setXGyroFIFOEnabled(true);
        // mpu.setYGyroFIFOEnabled(true);
        // mpu.setXGyroFIFOEnabled(true);
    }

    pinMode(MPU_INT, INPUT);
    attachInterrupt(MPU_INT, ISR_FIFO_OVF, RISING);

    // Serial.println("ax ay az gx gy gz");
}

void loop() {
    static uint16_t fifo_cnt;

    if (fifo_cnt < FIFO_DATA_SIZE) {
        if ((fifo_cnt = mpu.getFIFOCount()) < FIFO_DATA_SIZE) {
            return;
        }
    }

    fifo_cnt -= FIFO_DATA_SIZE;

    uint8_t buf[FIFO_DATA_SIZE];
    mpu.getFIFOBytes(buf, FIFO_DATA_SIZE);

    for (uint8_t j = 0; j < FIFO_DATA_SIZE; j += 2) {
        uint16_t data = (buf[j] << 8) | buf[j + 1];
        Serial.printf("%10hd", data);
    }
    Serial.println();

    // delay(100);
}

#endif