#ifndef _MOUSE_MOVE_INPUT_MPU9250_HANDLER_H_
#define _MOUSE_MOVE_INPUT_MPU9250_HANDLER_H_

#include "../looper.h"
#include <MPU9250.h>

namespace mouse_move {
    class MPU9250Handler: public HandlerInterface {
    public:
        using InputData = Move;

    public:
        void begin();

    public: // HandlerInterface trait for InputHandler
        bool available() const override final;
        void reset() override final;
        Move operator()(unsigned long interval_us) override final;

    private:
        MPU9250 mpu;
        float prev_roll;
        float prev_pitch;

        int16_t prev_ax;
        int16_t prev_ay;
        int16_t prev_az;
    };
}

#endif /* _MOUSE_MOVE_INPUT_IMU_HANDLER_H_ */