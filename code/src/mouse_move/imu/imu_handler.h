#ifndef _MOUSE_MOVE_INPUT_IMU_HANDLER_H_
#define _MOUSE_MOVE_INPUT_IMU_HANDLER_H_

#if 1
    #include "mpu9250_handler.h"
    namespace mouse_move { using IMUHandler = MPU9250Handler; }
#else
    #include "mpu6050_handler.h"
    namespace mouse_move { using IMUHandler = MPU6050Handler; }
#endif

#endif /* _MOUSE_MOVE_INPUT_IMU_HANDLER_H_ */