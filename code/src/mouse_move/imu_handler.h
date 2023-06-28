#ifndef _MOUSE_MOVE_INPUT_IMU_HANDLER_H_
#define _MOUSE_MOVE_INPUT_IMU_HANDLER_H_

#include "looper.h"
#include <MPU9250.h>

namespace mouse_move {
    class IMUHandler: public HandlerInterface {
    public:
        bool available() const override;
        Move operator()(unsigned long interval_us) override;
        void begin();
    private:
        MPU9250 mpu;
    };
}

#endif /* _MOUSE_MOVE_INPUT_IMU_HANDLER_H_ */