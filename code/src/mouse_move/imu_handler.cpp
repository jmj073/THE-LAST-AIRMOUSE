// #define IMU_FIFO_ENABLE
#define IMU_USE_ACCELEROMETER

#include "settings.h"

#include "imu_handler.h"
#include "debug.h"
#include "measure.h"

using namespace mouse_move;

static constexpr const uint8_t GY_FS_SEL = MPU9250_GYRO_FS_2000;
static constexpr float GY_LSB_DPS = float(1 << 15) / (250 << GY_FS_SEL); /* LSB/dps */

#ifdef ENABLE_PROTOTYPE
    #define GYRO_OFFSET_X 42
    #define GYRO_OFFSET_Y 45
    #define GYRO_OFFSET_Z 28

    #define ACCEL_OFFSET_X 5583
    #define ACCEL_OFFSET_Y 3458
    #define ACCEL_OFFSET_Z 10169
#else
    #define GYRO_OFFSET_X 83
    #define GYRO_OFFSET_Y 87
    #define GYRO_OFFSET_Z 11

    #define ACCEL_OFFSET_X 5025
    #define ACCEL_OFFSET_Y 5240
    #define ACCEL_OFFSET_Z 9026
#endif

static constexpr float COMPLE_ALPHA = 0.96;

static constexpr const float MOUSE_SPEED_IMU_X = 16;
static constexpr const float MOUSE_SPEED_IMU_Y = 16;

static constexpr const size_t ONLY_GYRO_FIFO_PACKET_SIZE = 4;
static constexpr const size_t GYRO_ACCEL_FIFO_PACKET_SIZE = 12;

#ifdef IMU_USE_ACCELEROMETER
    static constexpr size_t FIFO_PACKET_SIZE = GYRO_ACCEL_FIFO_PACKET_SIZE;
#else /* IMU_USE_ACCELEROMETER */
    static constexpr size_t FIFO_PACKET_SIZE = ONLY_GYRO_FIFO_PACKET_SIZE;
#endif /* IMU_USE_ACCELEROMETER */

#define IMU_GYRO_DISCARD_THRESHOLD  gyro_dps2raw(1)

static constexpr inline
float gyro_raw2dps(int16_t raw) { return raw / GY_LSB_DPS; }
static constexpr inline
int16_t gyro_dps2raw(float dps) { return dps * GY_LSB_DPS; }
static constexpr inline
float gyro_raw2degree(int16_t raw, unsigned long us) {
    return gyro_raw2dps(raw) * us / 1e6;
}
static constexpr inline
float gyro_raw2radian(int16_t raw, uint32_t us) {
    return radians(gyro_raw2degree(raw, us));
}
static constexpr inline
float accel_raw2roll(int16_t ax, int16_t ay, int16_t az) {
    return atan2(ay, sqrt(sq(ax) + sq(az)));
}
static inline
float accel_raw2roll2(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2roll(ax, ay, az);
    return az >= 0 ? tmp : PI - tmp;
}
static inline
float accel_raw2pitch(int16_t ax, int16_t ay, int16_t az) {
    return atan2(-ax, sqrt(sq(ay) + sq(az)));
}
static inline
float accel_raw2pitch2(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2pitch(ax, ay, az);
    return az >= 0 ? tmp : PI - tmp;
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
    #ifdef ENABLE_PROTOTYPE
    IMU_getMotion6FIFO(mpu, ax, ay, az, gx, gy, gz);
    #else
    IMU_getMotion6FIFO(mpu, ax, az, ay, gx, gz, gy);
    #endif
#else /* IMU_FIFO_ENABLE */
    #ifdef ENABLE_PROTOTYPE
    mpu.getMotion6(ax, ay, az, gx, gy, gz);
    #else
    mpu.getMotion6(ax, az, ay, gx, gz, gy);
    #endif
#endif /* IMU_FIFO_ENABLE */
    #ifndef ENABLE_PROTOTYPE
    *ax = -*ax;
    *gx = -*gx;
    #endif
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

void IMUHandler::begin() {
    Wire.begin();
    Wire.setClock(400000);

    if (!mpu.testConnection()) {
        DEBUG_PRINTF("mpu connection failed! (%d)\n", (int)mpu.getDeviceID());
    }

    mpu.reset();
    mpu.initialize();

    /* GYRO INIT */ {
        mpu.setFullScaleGyroRange(GY_FS_SEL);
        mpu.setXGyroOffsetUser(GYRO_OFFSET_X);
        mpu.setYGyroOffsetUser(GYRO_OFFSET_Y);
        mpu.setZGyroOffsetUser(GYRO_OFFSET_Z);
    }

    /* ACCEL INIT */ {
#ifndef IMU_USE_ACCELEROMETER
        mpu.setStandbyXAccelEnabled(true);
        mpu.setStandbyYAccelEnabled(true);
        mpu.setStandbyZAccelEnabled(true);
#endif /* IMU_USE_ACCELEROMETER */
        mpu.setXAccelOffset(ACCEL_OFFSET_X);
        mpu.setYAccelOffset(ACCEL_OFFSET_Y);
        mpu.setZAccelOffset(ACCEL_OFFSET_Z);
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

bool IMUHandler::available() const {
#ifdef IMU_FIFO_ENABLE
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

static inline
int16_t _discard_gyro_value(int16_t value) {
    return abs(value) < IMU_GYRO_DISCARD_THRESHOLD ? 0 : value;
}

Move IMUHandler::operator()(unsigned long interval_us) {
    static Measure<unsigned long long> measure(10000);
    measure.appendValue(interval_us);

    int16_t ax, ay, az, gx, gy, gz;
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);

    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    float dx = gyro_raw2degree(gx, interval_us);
    float dy = gyro_raw2degree(gy, interval_us);
    float dz = gyro_raw2degree(gz, interval_us);

    int32_t ax2 = sq(ax), ay2 = sq(ay), az2 = sq(az);
    double n = sqrt(ax2 + ay2 + az2);
    float yaw   = float((ax/n * dx) + (ay/n * dy) + (az/n * dz));
    
    // auto apitch = accel_raw2pitch(ax, ay, az);
    auto roll2 = accel_raw2roll2(ax, ay, az);
    float pitch = cos(roll2) * dy + sin(-roll2) * dz;

#if 0
    static Measure<int> measure(1000);
    // min: 0.000000, mean: 0.000172, max: 0.001863 
    // measure.appendValue(abs(yaw));
    // min: 0.000000, mean: 0.137573, max: 0.610352 
    // measure.appendValue(abs(gyro_raw2dps(gz)));
    measure.appendValue(abs(gz));
#endif

    return { 
        .x = yaw * MOUSE_SPEED_IMU_X,
        .y = pitch * MOUSE_SPEED_IMU_Y,
    };
}

#if 0 /* only gyro */
#ifdef IMU_USE_ACCELEROMETER 
    #error Accelerometer should be disabled. 
#endif /* IMU_USE_ACCELEROMETER */

Move IMUHandler::operator()(unsigned long interval_us) {
    int16_t gy, gz;

    // measure::begin(); // 426
    IMU_getGyroYZ(mpu, &gy, &gz);
    // measure::end();

    float yaw   = gyro_raw2degree(gz, interval_us);
    float pitch = gyro_raw2degree(gy, interval_us);

    return { 
        .x = yaw * MOUSE_SPEED_IMU_X,
        .y = pitch * MOUSE_SPEED_IMU_Y,
    };
}
#endif /* only gyro*/