#if 0 /* FILE */

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

#if 0 /* GET OFFSET*/

void setup() {
    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000);

    /* MPU9250 INIT */ {
        my_assert(mpu.testConnection(), "mpu connection failed! (%d)", (int)mpu.getDeviceID());
        mpu.reset();
        mpu.initialize();

        Serial.println("accel");
        Serial.printf("%10hd", mpu.getXAccelOffset());
        Serial.printf("%10hd", mpu.getYAccelOffset());
        Serial.printf("%10hd", mpu.getZAccelOffset());
        Serial.println();
        Serial.println("gyro");
        Serial.printf("%10hd", mpu.getXGyroOffset());
        Serial.printf("%10hd", mpu.getYGyroOffset());
        Serial.printf("%10hd", mpu.getZGyroOffset());
        Serial.println();
        Serial.println("gyro user");
        Serial.printf("%10hd", mpu.getXGyroOffsetUser());
        Serial.printf("%10hd", mpu.getYGyroOffsetUser());
        Serial.printf("%10hd", mpu.getZGyroOffsetUser());
        Serial.println();
    }
}

void loop() { }

#endif /* GET OFFSET*/

#if 0 /* ACCEL */

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
        // mpu.setDLPFMode(6);
        // mpu.setAccelDLPFMode(6);

        /* OFFSET
         * 현재 설정된 full scale이 어떻든 offset은 16G 기준이다.
         * 상위 15bit가 사용되며, LSB는 예약되어 있다.
         * my 센서는 기존 값이 (5622, 3472, 9986)이다.
         */
        // mpu.setXAccelOffset(mpu.getXAccelOffset() + 0);
        // mpu.setYAccelOffset(mpu.getYAccelOffset() + 0);
        // mpu.setZAccelOffset(mpu.getZAccelOffset() + -2048);

        mpu.setXAccelOffset(5573);
        mpu.setYAccelOffset(3483);
        mpu.setZAccelOffset(10176);
    }

    // Serial.println("ax ay az");
}

static
void mean_sensors(int16_t* gx, int16_t* gy, int16_t* gz, size_t n) {
    int32_t acc_x = 0, acc_y = 0, acc_z = 0;

    for (size_t i = 0; i < n; ++i) {
        int16_t x, y, z;
        mpu.getAcceleration(&x, &y, &z);

        acc_x += x;
        acc_y += y;
        acc_z += z;

        delay(2);
    }

    *gx = acc_x / (int32_t)n;
    *gy = acc_y / (int32_t)n;
    *gz = acc_z / (int32_t)n;
}

void loop() {
    delay(200);

    int16_t x, y, z;
    mean_sensors(&x, &y, &z, 1000);

    Serial.printf("%10hd", x);
    Serial.printf("%10hd", y);
    Serial.printf("%10hd", z);
    Serial.println();
}

// void loop() {
//     int16_t ax, ay, az;
//     mpu.getAcceleration(&ax, &ay, &az);

//     Serial.print(ax); Serial.print(' ');
//     Serial.print(ay); Serial.print(' ');
//     Serial.print(az); Serial.print(' ');
//     Serial.println();
// }

#endif /* ACCEL */

#if 0 /* GYRO */

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
        // mpu.setDLPFMode(6);
        // mpu.setAccelDLPFMode(6);

        mpu.setFullScaleGyroRange(MPU9250_GYRO_FS_500);

        /* OFFSET
         * my 센서는 기존 값이 (32, 42, 48)이다.
         * user데이터 기존 값은 (0, 0, 0)이다.
         */

        // mpu.setXGyroOffset(mpu.getXGyroOffset() + 32);
        // mpu.setYGyroOffset(mpu.getYGyroOffset() + 0);
        // mpu.setZGyroOffset(mpu.getZGyroOffset() + 0);
        
        /*
         * OFFS_USR는 FS_SEL에 따라 가중치가 알맞게
         * 설정되므로 FS_SEL을 바꿀때 설정을 바꿀 필요가 없다.
         * OffsetLSB = OFFS_USR * 4 / 2^FS_SEL 
         */     
        // mpu.setXGyroOffsetUser(mpu.getXGyroOffsetUser() + 50); // -199
        // mpu.setYGyroOffsetUser(mpu.getYGyroOffsetUser()); // -151
        // mpu.setZGyroOffsetUser(mpu.getZGyroOffsetUser()); // -121

        mpu.setXGyroOffsetUser(54);
        mpu.setYGyroOffsetUser(32);
        mpu.setZGyroOffsetUser(32);
    }

    // Serial.println("gx gy gz");
}

static
void mean_sensors(int16_t* gx, int16_t* gy, int16_t* gz, size_t n) {
    int32_t acc_x = 0, acc_y = 0, acc_z = 0;

    for (size_t i = 0; i < n; ++i) {
        int16_t x, y, z;
        mpu.getRotation(&x, &y, &z);

        acc_x += x;
        acc_y += y;
        acc_z += z;

        delay(2);
    }

    *gx = acc_x / (int32_t)n;
    *gy = acc_y / (int32_t)n;
    *gz = acc_z / (int32_t)n;
}

void loop() {
    delay(200);

    int16_t x, y, z;
    mean_sensors(&x, &y, &z, 1000);

    Serial.printf("%10hd", x);
    Serial.printf("%10hd", y);
    Serial.printf("%10hd", z);
    Serial.println();
}

// void loop() {
//     int16_t gx, gy, gz;
//     mpu.getRotation(&gx, &gy, &gz);

//     Serial.print(gx); Serial.print(' ');
//     Serial.print(gy); Serial.print(' ');
//     Serial.print(gz); Serial.print(' ');
//     Serial.println();
// }

#endif /* GYRO */

#endif /* FILE */