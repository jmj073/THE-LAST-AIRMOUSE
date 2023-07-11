#include "settings.h"
#include "mpu9250_handler.h"
#include "debug.h"
#include "measure.h"

#define GYRO_FS_SEL MPU9250_GYRO_FS_2000
#define ACCEL_FS_SEL MPU9250_ACCEL_FS_2
#include "mpu_util.h"

using namespace mouse_move;

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

static constexpr const float COMPLE_ALPHA = 0.98;

static constexpr const float MOUSE_SPEED_IMU_X = degrees(18);
static constexpr const float MOUSE_SPEED_IMU_Y = degrees(18);

static constexpr const size_t ONLY_GYRO_FIFO_PACKET_SIZE = 4;
static constexpr const size_t GYRO_ACCEL_FIFO_PACKET_SIZE = 12;

static constexpr const size_t FIFO_PACKET_SIZE = GYRO_ACCEL_FIFO_PACKET_SIZE;

#define IMU_GYRO_DISCARD_THRESHOLD  gyro_dps2raw(1)


static inline
void IMU_getAcceleration(MPU9250& mpu, int16_t* ax, int16_t* ay, int16_t* az) {
#ifdef ENABLE_PROTOTYPE
    mpu.getAcceleration(ax, ay, az);
    *ax = -*ax;
#else
    mpu.getAcceleration(ax, az, ay);
#endif
}

static inline
void IMU_getMotion6(MPU9250& mpu, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
#ifdef ENABLE_PROTOTYPE
    mpu.getMotion6(ax, ay, az, gx, gy, gz);
    *ax = -*ax;
    *gx = -*gx;
#else
    mpu.getMotion6(ax, az, ay, gx, gz, gy);
#endif
}

void MPU9250Handler::begin() {
    Wire.begin();
    Wire.setClock(400000);

    if (!mpu.testConnection()) {
        DEBUG_PRINTF("mpu connection failed! (%d)\n", (int)mpu.getDeviceID());
    }

    mpu.reset();
    mpu.initialize();

    /* GYRO INIT */ {
        mpu.setFullScaleGyroRange(GYRO_FS_SEL);
        mpu.setXGyroOffsetUser(GYRO_OFFSET_X);
        mpu.setYGyroOffsetUser(GYRO_OFFSET_Y);
        mpu.setZGyroOffsetUser(GYRO_OFFSET_Z);
    }

    /* ACCEL INIT */ {
        mpu.setFullScaleAccelRange(ACCEL_FS_SEL);
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
    mpu.setXGyroFIFOEnabled(true);
    mpu.setAccelFIFOEnabled(true);
    mpu.resetFIFO();
#endif /* IMU_FIFO_ENABLE*/
    }

    reset();
}

bool MPU9250Handler::available() const {
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

void MPU9250Handler::reset() {
    int16_t ax, ay, az;
    IMU_getAcceleration(mpu, &ax, &ay, &az);

    prev_roll = accel_raw2roll3(az, ay, az);
    prev_pitch = accel_raw2pitch3(az, ay, az);

    prev_ax = ax;
    prev_ay = ay;
    prev_az = az;
}

static inline
int16_t _discard_gyro_value(int16_t value) {
    return abs(value) < IMU_GYRO_DISCARD_THRESHOLD ? 0 : value;
}

#if 1 /* basic */
Move MPU9250Handler::operator()(unsigned long interval_us) {
    // static Measure<unsigned long long> measure(10000);
    // measure.appendValue(interval_us);

    int16_t ax, ay, az, gx, gy, gz;
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);

    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    float dx = gyro_raw2radian(gx, interval_us);
    float dy = gyro_raw2radian(gy, interval_us);
    float dz = gyro_raw2radian(gz, interval_us);

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
#endif

#if 0
Move MPU9250Handler::operator()(unsigned long interval_us) {
    static MeasureTime measure(10000);
    int16_t ax, ay, az, gx, gy, gz;
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);

    measure.measureStart();
    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    float dx = gyro_raw2radian(gx, interval_us);
    float dy = gyro_raw2radian(gy, interval_us);
    float dz = gyro_raw2radian(gz, interval_us);

    float apitch = accel_raw2pitch3(ax, ay, az);
    float aroll  = accel_raw2roll3(ax, ay, az);

    float gpitch = cos(aroll) * dy + sin(-aroll) * dz;
    float groll = cos(apitch) * dx + sin(-apitch) * dz;
    // auto gyaw = 
    // auto sin_roll = sin(prev_roll), sin_pitch = sin(prev_pitch);
    // auto gyaw = (sin_roll * gy) + (sin_pitch * gx) + (gz / sqrt(1 + sq(sin_pitch) + sq(prev_roll)));

    // prev_roll  = modular(prev_roll + groll, TWO_PI);
    // prev_pitch  = modular(prev_pitch + gpitch, TWO_PI);

    // Serial.print(degrees(aroll)); Serial.print(' ');
    // Serial.print(degrees(apitch)); Serial.print(' ');
    // Serial.print(degrees(prev_roll)); Serial.print(' ');
    // Serial.print(degrees(prev_pitch)); Serial.print(' ');
    // Serial.println();
    measure.measureStop();

    return { 
        .x = 0 * MOUSE_SPEED_IMU_X,
        .y = gpitch * MOUSE_SPEED_IMU_Y,
    };
}
#endif

#if 0
Move MPU9250Handler::operator()(unsigned long interval_us) {
    int16_t ax, ay, az, gx, gy, gz;
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);

    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    float dx = gyro_raw2radian(gx, interval_us);
    float dy = gyro_raw2radian(gy, interval_us);
    float dz = gyro_raw2radian(gz, interval_us);

    float apitch = accel_raw2pitch3(ax, ay, az);
    float aroll  = accel_raw2roll3(ax, ay, az);

    prev_pitch = modular(complementary_combine_angle(prev_pitch, apitch, COMPLE_ALPHA), TWO_PI);
    prev_roll = modular(complementary_combine_angle(prev_roll, aroll, COMPLE_ALPHA), TWO_PI);

    float gpitch = cos(prev_roll) * dy + sin(-prev_roll) * dz;
    // float groll = cos(apitch) * dx + sin(-apitch) * dz;

    Serial.print(degrees(prev_roll)); Serial.print(' ');
    Serial.print(degrees(prev_pitch)); Serial.print(' ');
    Serial.println();

    return { 
        .x = 0 * MOUSE_SPEED_IMU_X,
        .y = gpitch * MOUSE_SPEED_IMU_Y,
    };
}
#endif

#if 0 /* basic */
Move MPU9250Handler::operator()(unsigned long interval_us) {
    // static Measure<unsigned long long> measure(10000);
    // measure.appendValue(interval_us);

    int16_t ax, ay, az, gx, gy, gz;
    IMU_getMotion6(mpu, &ax, &ay, &az, &gx, &gy, &gz);

    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    float dx = gyro_raw2radian(gx, interval_us);
    float dy = gyro_raw2radian(gy, interval_us);
    float dz = gyro_raw2radian(gz, interval_us);

    ax = prev_ax * COMPLE_ALPHA + ax * (1 - COMPLE_ALPHA);
    ay = prev_ay * COMPLE_ALPHA + ay * (1 - COMPLE_ALPHA);
    az = prev_az * COMPLE_ALPHA + az * (1 - COMPLE_ALPHA);

    int32_t ax2 = sq(ax), ay2 = sq(ay), az2 = sq(az);
    double n = sqrt(ax2 + ay2 + az2);
    float yaw   = float((ax/n * dx) + (ay/n * dy) + (az/n * dz));
    
    // auto apitch = accel_raw2pitch(ax, ay, az);
    auto roll2 = accel_raw2roll2(ax, ay, az);
    float pitch = cos(roll2) * dy + sin(-roll2) * dz;

    prev_ax = ax;
    prev_ay = ay;
    prev_az = az;

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

#endif