#ifndef _MOUSE_MOVE_INPUT_MPU6050_HANDLER_H_
#define _MOUSE_MOVE_INPUT_MPU6050_HANDLER_H_

#include "../looper.h"
#include <MPU6050_6axis_MotionApps20.h>

namespace mouse_move {
    class MPU6050Handler: public HandlerInterface {
    public:
        using InputData = Move;

    public:
        void begin();

    public: // HandlerInterface trait for InputHandler
        bool available() const override final;
        void reset() override final;
        Move operator()(unsigned long interval_us) override final;

    private:
        mutable MPU6050 mpu;
        float prev_yaw;
        float prev_pitch;
        bool is_first = true;
    };
}

#endif /* _MOUSE_MOVE_INPUT_IMU_HANDLER_H_ */