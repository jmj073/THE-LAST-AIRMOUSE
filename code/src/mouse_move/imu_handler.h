#ifndef _MOUSE_MOVE_INPUT_IMU_HANDLER_H_
#define _MOUSE_MOVE_INPUT_IMU_HANDLER_H_

#include "looper.h"
#include <MPU9250.h>

namespace mouse_move {
    class IMUHandler: public HandlerInterface {
    public:
        using InputData = Move;

    public:
        void begin();

    public: // HandlerInterface trait for InputHandler
        bool available() const override;
        void reset() override;
        Move operator()(unsigned long interval_us) override;

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