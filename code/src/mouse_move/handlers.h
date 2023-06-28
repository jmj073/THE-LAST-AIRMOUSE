#ifndef _MOUSE_MOVE_INPUT_HANDLERS_H_
#define _MOUSE_MOVE_INPUT_HANDLERS_H_

#include "looper.h"
#include <MPU9250.h>

class IMUHandler: public mouse_move::HandlerInterface {
public:
    bool available() const override;
    mouse_move::Move operator()(unsigned long interval_us) override;
    void begin();
private:
    MPU9250 mpu;
};

class JoystickHandler: public mouse_move::HandlerInterface {
public:
    bool available() const override;
    mouse_move::Move operator()(unsigned long interval_us) override;
    void begin() { }
};

#endif /* _MOUSE_MOVE_INPUT_HANDLERS_H_ */