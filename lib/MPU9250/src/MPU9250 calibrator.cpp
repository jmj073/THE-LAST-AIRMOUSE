/*
 * # Reference
 * https://mjwhite8119.github.io/Robots/mpu6050
 * 
 * # Calibration 방식
 * 1. sample을 1000개 읽은 다음 평균 값을 계산하여 IMU에 offset을 입력
 * 2. 보정이 공차 내에 도달할 때까지 보정 반복.
 */


#include "MPU9250 calibrator.h"

#include <Arduino.h>

SMotion6 SMotion6::operator+(const SMotion6& lhs) {
    SMotion6 tmp = *this;

    tmp.a.x += lhs.a.x;
    tmp.a.y += lhs.a.y;
    tmp.a.z += lhs.a.z;

    tmp.g.x += lhs.g.x;
    tmp.g.y += lhs.g.y;
    tmp.g.z += lhs.g.z;

    return tmp;
}

SMotion6 SMotion6::operator-(const SMotion6& lhs) {
    SMotion6 tmp = *this;

    tmp.a.x -= lhs.a.x;
    tmp.a.y -= lhs.a.y;
    tmp.a.z -= lhs.a.z;

    tmp.g.x -= lhs.g.x;
    tmp.g.y -= lhs.g.y;
    tmp.g.z -= lhs.g.z;

    return tmp;
}

MPU9250Calibrator::MPU9250Calibrator(MPU9250* mpu)
    : m_mpu{ mpu }
{ }

void MPU9250Calibrator::calibrate6() {
    SMotion6 mean, offset;

    set_offsets({});

    get_mean_motion6(&offset, 1000);
    mean_to_offset(&offset);

    while (1) {
        set_offsets(offset);
        get_mean_motion6(&mean, 1000);

        Serial.println("offset");
        log_sensors(offset);
        Serial.println("mean");
        log_sensors(mean);

        mean_to_offset(&mean);

        offset += mean;
    }

    log_sensors(offset);
}


void MPU9250Calibrator::get_mean_motion6(SMotion6* s, size_t n) {
    int32_t acc_ax = 0, acc_ay = 0, acc_az = 0;
    int32_t acc_gx = 0, acc_gy = 0, acc_gz = 0;

    for (size_t i = 0; i < n; ++i) {
        int16_t ax, ay, az, gx, gy, gz;
        m_mpu->getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        acc_ax += ax;
        acc_ay += ay;
        acc_az += az;
        acc_gx += gx;
        acc_gy += gy;
        acc_gz += gz;

        delay(2);
    }

    s->a.x = acc_ax / (int32_t)n;
    s->a.y = acc_ay / (int32_t)n;
    s->a.z = acc_az / (int32_t)n;
    s->g.x = acc_gx / (int32_t)n;
    s->g.y = acc_gy / (int32_t)n;
    s->g.z = acc_gz / (int32_t)n;
}

void MPU9250Calibrator::mean_to_offset(SMotion6* s) const {
    SMotion6& m = *s;

    int16_t afs = m_mpu->getFullScaleAccelRange();
    m.a.x = -m.a.x >> (3 - afs);
    m.a.y = -m.a.y >> (3 - afs);
    m.a.z = ((1 << (14 - afs)) - m.a.z) >> (3 - afs); // z축에는 1G가 있다

    int16_t gfs = m_mpu->getFullScaleGyroRange();
    m.g.x = -m.g.x << gfs >> 2;
    m.g.y = -m.g.y << gfs >> 2;
    m.g.z = -m.g.z << gfs >> 2;
}

void MPU9250Calibrator::set_offsets(const SMotion6& s) {
    m_mpu->setXAccelOffset(s.a.x);
    m_mpu->setYAccelOffset(s.a.y);
    m_mpu->setZAccelOffset(s.a.z);

    m_mpu->setXGyroOffsetUser(s.g.x);
    m_mpu->setYGyroOffsetUser(s.g.y);
    m_mpu->setZGyroOffsetUser(s.g.z);
}

void MPU9250Calibrator::log_sensors(const SMotion6& s) const {
    Serial.printf("ax: %7hd, ", s.a.x);
    Serial.printf("ay: %7hd, ", s.a.y);
    Serial.printf("az: %7hd, ", s.a.z);

    Serial.printf("gx: %7hd, ", s.g.x);
    Serial.printf("gy: %7hd, ", s.g.y);
    Serial.printf("gz: %7hd  ", s.g.z);

    Serial.println();
}