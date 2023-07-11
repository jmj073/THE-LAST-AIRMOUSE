#include "settings.h"

#include "mpu6050_handler.h"
#include "debug.h"
#include "measure.h"

#define GYRO_FS_SEL MPU6050_GYRO_FS_2000
#define ACCEL_FS_SEL MPU6050_ACCEL_FS_2
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

static constexpr const float MOUSE_SPEED_IMU_X = degrees(16);
static constexpr const float MOUSE_SPEED_IMU_Y = degrees(16);


void MPU6050Handler::begin() {
    Wire.begin();
    Wire.setClock(400000);

    if (!mpu.testConnection()) {
        DEBUG_PRINTF("mpu connection failed! (%d)\n", (int)mpu.getDeviceID());
    }

    mpu.reset();
    mpu.initialize();
    mpu.dmpInitialize();

    /* GYRO INIT */ {
        mpu.setFullScaleGyroRange(GYRO_FS_SEL);
        mpu.setXGyroOffset(GYRO_OFFSET_X);
        mpu.setYGyroOffset(GYRO_OFFSET_Y);
        mpu.setZGyroOffset(GYRO_OFFSET_Z);
    }

    /* ACCEL INIT */ {
        mpu.setFullScaleAccelRange(ACCEL_FS_SEL);
        mpu.setXAccelOffset(ACCEL_OFFSET_X);
        mpu.setYAccelOffset(ACCEL_OFFSET_Y);
        mpu.setZAccelOffset(ACCEL_OFFSET_Z);
    }

    mpu.setDMPEnabled(true);

    reset();
}

bool MPU6050Handler::available() const {
    bool ovf = mpu.getIntFIFOBufferOverflowStatus();
    auto fifo_count = mpu.getFIFOCount();
    if (fifo_count == 1024 || ovf) {
        mpu.resetFIFO();
        return false;
    }
    return fifo_count >= mpu.dmpGetFIFOPacketSize();
}

void MPU6050Handler::reset() {
    is_first = true;
}

Move MPU6050Handler::operator()(unsigned long interval_us) {
    (void)interval_us;

    auto packet_size = mpu.dmpGetFIFOPacketSize();
    uint8_t fifo_buf[64];
    Quaternion q;
    VectorFloat gravity;
    float ypr[3];

    mpu.getFIFOBytes(fifo_buf, packet_size);
    mpu.dmpGetQuaternion(&q);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    float yaw = ypr[0];
    float pitch = ypr[1];

    if (is_first) {
        prev_yaw = yaw;
        prev_pitch = pitch;
        return { 0, 0 };
    }

    float diff_yaw = yaw - prev_yaw;
    float diff_pitch = pitch - prev_pitch;
    prev_yaw = yaw;
    prev_pitch = pitch;

    return {
        .x = diff_yaw * MOUSE_SPEED_IMU_X,
        .y = diff_pitch * MOUSE_SPEED_IMU_Y,
    };
}