#ifndef _MPU9250_CALIBRATOR_H_
#define _MPU9250_CALIBRATOR_H_

#include "MPU9250.h"

struct SMotion6 {
    struct Accel {
        int16_t x, y, z;
    } a;
    struct Gyro {
        int16_t x, y, z;
    } g;

    SMotion6 operator+(const SMotion6& lhs);
    SMotion6& operator+=(const SMotion6& lhs) {
        return *this = *this + lhs;
    }
    SMotion6 operator-(const SMotion6& lhs);
    SMotion6& operator-=(const SMotion6& lhs) {
        return *this = *this - lhs;
    }
};

class MPU9250Calibrator {
public:

public:
    static constexpr int16_t ACCEL_DEADZONE = 8;
    static constexpr int16_t GYRO_DEADZONE = 1;

public:
    MPU9250Calibrator(MPU9250* mpu);

public:
    // void calibrate_accel();
    // void calibrate_gyro();
    void calibrate6();

private:
    void get_mean_motion6(SMotion6* s, size_t n);
    void mean_to_offset(SMotion6* s) const;
    void set_offsets(const SMotion6& s);
    void log_offset(const SMotion6& s) const;

private:
    MPU9250* m_mpu;
    int16_t afs, gfs;
};

#endif /* _MPU9250_CALIBRATOR_H_ */