#include <Arduino.h>

#include <MPU9250.h>
#include <Wire.h>
#include <BleMouse.h>
#include <algorithm>

/** Raw data to degree
 * dps means degree per second
 * @param raw gyro raw data
 * @param us micro second
 * @return unit is degree
 */
static inline
float gyro_raw2degree(int16_t raw, uint32_t us) {
    constexpr float LSB = float(1 << 15) / (250 << GY_FS_SEL); // LSB/dps
    float degree =  raw / LSB; // raw to dps
    return degree * us / 1e6;
}

void mpu_loop() {

}