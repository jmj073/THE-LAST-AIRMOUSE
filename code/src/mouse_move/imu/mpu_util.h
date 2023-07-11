#ifndef _MPU_UTIL_H_
#define _MPU_UTIL_H_

#include <Arduino.h>
#include <cmath>

template <typename T, typename U>
auto modular(T x, U y) -> decltype(std::fmod(x, y) + y) {
    auto tmp = std::fmod(x, y);
    return tmp < 0 ? tmp + y : tmp;
}

template <typename T, typename U>
auto modular_diff(T b, T a, U m) -> decltype(b - a) {
    if (b >= a) {
        auto tmp1 = b - a;
        auto tmp2 = tmp1 - m;
        return tmp1 < abs(tmp2) ? tmp1 : tmp2;
    }
    auto tmp1 = b - a;
    auto tmp2 = m + tmp1;
    return tmp1 >= -abs(tmp2) ? tmp1 : tmp2;
}

static inline
float complementary_combine_angle(float a, float b, float alpha) {
    // return modular_diff(b, a, 360) * alpha + a;
    return b - modular_diff(b, a, 360) * alpha;
}

/* gyro */
#ifndef GYRO_FS_SEL
    #define dyro_raw2dps #error "GYRO_FS_SEL is not defined"
    #define dyro_dps2raw #error "GYRO_FS_SEL is not defined"
    #define dyro_raw2degree #error "GYRO_FS_SEL is not defined"
    #define dyro_raw2radian #error "GYRO_FS_SEL is not defined"
#else /* GYRO_FS_SEL */
    static constexpr const
    float GYRO_LSB_DPS = (float(1 << 15) / (250 << GYRO_FS_SEL)); /* LSB/dps */

    static constexpr inline
    float gyro_raw2dps(int16_t raw) { return raw / GYRO_LSB_DPS; }
    static constexpr inline
    int16_t gyro_dps2raw(float dps) { return dps * GYRO_LSB_DPS; }
    static constexpr inline
    float gyro_raw2degree(int16_t raw, unsigned long us) {
        return gyro_raw2dps(raw) * us / 1e6;
    }
    static constexpr inline
    float gyro_raw2radian(int16_t raw, uint32_t us) {
        return radians(gyro_raw2degree(raw, us));
    }
#endif /* GYRO_FS_SEL */

/* accel */
#ifndef ACCEL_FS_SEL
    #define accel_raw2g #error "ACCEL_FS_SEL is not defined"
#else /* ACCEL_LSB_G */
    static constexpr const
    float  ACCEL_LSB_G = float(1 << 15) / (2 << ACCEL_FS_SEL); /* LSB/g */

    static constexpr inline
    float accel_raw2g(int16_t a) { return a / ACCEL_LSB_G; }
#endif /* ACCEL_LSB_G */

static constexpr inline
float accel_raw2roll(int16_t ax, int16_t ay, int16_t az) {
    return atan2(ay, sqrt(sq(ax) + sq(az)));
}
static inline
float accel_raw2roll2(int16_t ax, int16_t ay, int16_t az) {
    auto d = sqrt(sq(ax) + sq(az));
    return atan2(ay, az < 0 ? -d : d);
}
static inline
float accel_raw2roll3(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2roll2(ax, ay, az);
    return tmp + (tmp < 0 ? TWO_PI : 0); // mod
}
static inline
float accel_raw2pitch(int16_t ax, int16_t ay, int16_t az) {
    return atan2(-ax, sqrt(sq(ay) + sq(az)));
}
static inline
float accel_raw2pitch2(int16_t ax, int16_t ay, int16_t az) {
    auto d = sqrt(sq(ay) + sq(az));
    return atan2(ax, az < 0 ? -d : d);
}
static inline
float accel_raw2pitch3(int16_t ax, int16_t ay, int16_t az) {
    auto tmp = accel_raw2pitch2(ax, ay, az);
    return tmp + (tmp < 0 ? TWO_PI : 0); // mod
}

#endif /* _MPU_UTIL_H_ */