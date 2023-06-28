#ifndef _MOUSE_MOVE_INPUT_JOYSTICK_HANDLERS_H_
#define _MOUSE_MOVE_INPUT_JOYSTICK_HANDLERS_H_

#include "looper.h"
#include <MPU9250.h>

namespace mouse_move {
    class JoystickHandler: public HandlerInterface {
    public:
        bool available() const override;
        Move operator()(unsigned long interval_us) override;
        void begin() { }
    };
}

#endif /* _MOUSE_MOVE_INPUT_JOYSTICK_HANDLERS_H_ */