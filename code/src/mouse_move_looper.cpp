#define BLE_SEND_INTERVAL 7UL // millisecond
// #define ENABLE_IMU_INTERVAL_MEASUREMENT
// #define IMU_FIFO_ENABLE
#define IMU_USE_ACCELEROMETER

#include "mouse_move_looper.h"

#include <MPU9250.h>
#include "pins.h"
#include "utils.h"
#include "debug.h"

using namespace mouse_move;

static constexpr size_t ONLY_GYRO_FIFO_PACKET_SIZE = 4;
static constexpr size_t GYRO_ACCEL_FIFO_PACKET_SIZE = 12;

#ifdef IMU_USE_ACCELEROMETER
    static constexpr size_t FIFO_PACKET_SIZE = GYRO_ACCEL_FIFO_PACKET_SIZE;
#else /* IMU_USE_ACCELEROMETER */
    static constexpr size_t FIFO_PACKET_SIZE = ONLY_GYRO_FIFO_PACKET_SIZE;
#endif /* IMU_USE_ACCELEROMETER */

constexpr uint8_t GY_FS_SEL = MPU9250_GYRO_FS_250;

static MPU9250 mpu;

static inline
void measure_interval_if_enabled() {
#ifdef ENABLE_IMU_INTERVAL_MEASUREMENT
    constexpr unsigned long NUM = 1000;
    static unsigned long sum, cnt;
    
    if (cnt) sum += interval;
    if (++cnt > NUM) {
        Serial.printf("imu interval: %lu\r\n", sum / NUM);
        sum = cnt = 0;
    }
#endif /* ENABLE_IMU_INTERVAL_MEASUREMENT */
}

static inline
float gyro_raw2degree(int16_t raw, unsigned long us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

static inline
void IMU_getMotion6FIFO(MPU9250& mpu, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
    uint8_t buffer[GYRO_ACCEL_FIFO_PACKET_SIZE];
    mpu.getFIFOBytes(buffer, sizeof(buffer));

    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[6]) << 8) | buffer[7];
    *gy = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gz = (((int16_t)buffer[10]) << 8) | buffer[11];
}

static inline
void IMU_getMotion6(MPU9250& mpu, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
#ifdef IMU_FIFO_ENABLE
    IMU_getMotion6FIFO(mpu, ax, ay, az, gx, gy, gz);
#else /* IMU_FIFO_ENABLE */
    mpu.getMotion6(ax, ay, az, gx, gy, gz);
#endif /* IMU_FIFO_ENABLE */
}

static inline
void IMU_getGyroYZFIFO(MPU9250& mpu, int16_t* gy, int16_t* gz) {
    uint8_t data[ONLY_GYRO_FIFO_PACKET_SIZE];

    mpu.getFIFOBytes(data, sizeof(data));

    *gy = ((int16_t)data[0] << 8) | data[1];
    *gz = ((int16_t)data[2] << 8) | data[3];
}

static inline
void IMU_getGyroYZ(MPU9250& mpu, int16_t* gy, int16_t* gz) {
#ifdef IMU_FIFO_ENABLE
    IMU_getGyroYZFIFO(mpu, gy, gz);
#else /* IMU_FIFO_ENABLE */
    int16_t gx;
    mpu.getRotation(&gx, gy, gz);
#endif /* IMU_FIFO_ENABLE */ 
}

static Move handle_imu4move_mouse(unsigned long interval) {
    measure_interval_if_enabled();
    
#ifdef IMU_USE_ACCELEROMETER
    int16_t ax, ay, az, gx, gy, gz;
    // measure::begin(); // 658
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);
    // measure::end();

    // measure::begin(); // 23
    float dx = gyro_raw2degree((ax < 0 ? -gx : gx), interval);
    float dy = gyro_raw2degree((ay < 0 ? -gy : gy), interval);
    float dz = gyro_raw2degree((az < 0 ? -gz : gz), interval);

    int32_t ax2 = ax * ax, ay2 = ay * ay, az2 = az * az;
    double n = ay2 + az2, m = n + ax2;

    float yaw   = float((ax2/m * dx) + (ay2/m * dy) + (az2/m * dz)) * 16;
    float pitch = float((az2/n * dy) + (ay2/n * dz)) * 16;
    // measure::end();
#else /* IMU_USE_ACCELEROMETER */
    int16_t gy, gz;

    // measure::begin(); // 426
    IMU_getGyroYZ(mpu, &gy, &gz);
    // measure::end();

    float yaw   = gyro_raw2degree(gz, interval) * 16;
    float pitch = gyro_raw2degree(gy, interval) * 16;
#endif /* IMU_USE_ACCELEROMETER */

    return { .x = yaw, .y = pitch };
}

static
Move handle_joystick4move_mouse(unsigned long interval) {
    int x = analogRead(PIN_JOYSTICK_X) - 1958;
    int y = analogRead(PIN_JOYSTICK_Y) - 2019;

    return {
        .x = (int64_t)interval * y / (1e6f * 3),
        .y = (int64_t)interval * x / (1e6f * 3),
    };
}

static
void setup_mpu() {
    Wire.begin();
    Wire.setClock(400000);

    if (!mpu.testConnection()) {
        DEBUG_PRINTF("mpu connection failed! (%d)\n", (int)mpu.getDeviceID());
    }

    mpu.reset();
    mpu.initialize();

    /* GYRO INIT */ {
        mpu.setFullScaleGyroRange(GY_FS_SEL);

        mpu.setXGyroOffsetUser(42);
        mpu.setYGyroOffsetUser(45);
        mpu.setZGyroOffsetUser(28);
    }

    /* ACCEL INIT */ {
#ifndef IMU_USE_ACCELEROMETER
        mpu.setStandbyXAccelEnabled(true);
        mpu.setStandbyYAccelEnabled(true);
        mpu.setStandbyZAccelEnabled(true);
#endif /* IMU_USE_ACCELEROMETER */
        mpu.setXAccelOffset(5583);
        mpu.setYAccelOffset(3458);
        mpu.setZAccelOffset(10169);
    }

    /* DLPF */ {
        // mpu.setDLPFMode(1);
        mpu.setAccelDLPFMode(6);
    }

    /* FIFO INIT */ {
#ifdef IMU_FIFO_ENABLE
    mpu.resetFIFO();
    mpu.setFIFOMode(true);
    mpu.setFIFOEnabled(true);

    mpu.setYGyroFIFOEnabled(true);
    mpu.setZGyroFIFOEnabled(true);
#ifdef IMU_USE_ACCELEROMETER
    mpu.setXGyroFIFOEnabled(true);
    mpu.setAccelFIFOEnabled(true);
#endif /* IMU_USE_ACCELEROMETER */
    mpu.resetFIFO();
#endif /* IMU_FIFO_ENABLE*/
    }

}

InputHandler::InputHandler(InputMode mode)
{
    setInputMode(mode);
}

void InputHandler::begin() {
    setup_mpu();
}

void InputHandler::setInputMode(InputMode mode) {
    switch (mode) {
        case IMU:
            handler = handle_imu4move_mouse;
            break;
        case JOYSTICK:
            handler = handle_joystick4move_mouse;
            break;
    }

    this->mode = mode;
}

bool InputHandler::available() const {
#ifdef IMU_FIFO_ENABLE
    if (mode == JOYSTICK) return true;
    auto fifo_count = mpu.getFIFOCount();
    if (fifo_count == MPU9250_FIFO_MAX_SIZE || mpu.getIntFIFOBufferOverflowStatus()) {
        mpu.resetFIFO();
        // DEBUG_PRINTLN("fifo overflow!");
        return false;
    }
    return fifo_count >= FIFO_PACKET_SIZE;
#else /* IMU_FIFO_ENABLE */
    return true;
#endif /* IMU_FIFO_ENABLE */
}

void OutputHandler::moveMouse() {
    uint32_t curr_ms = millis();
    if (curr_ms - prev_ms >= BLE_SEND_INTERVAL) {
        prev_ms = curr_ms;

        auto y = (signed char)std::clamp<int32_t>(yaw, -128, 127);
        auto p = (signed char)std::clamp<int32_t>(pitch, -128, 127);

        yaw -= y;
        pitch -= p;

        combo->move(-y, p);
    }
}