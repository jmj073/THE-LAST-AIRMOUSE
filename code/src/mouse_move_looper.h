#ifndef _MOUSE_MOVE_LOOPER_H_
#define _MOUSE_MOVE_LOOPER_H_

#include <BleCombo.h>

#include "looper.h"

class MouseMoveHandler;
using MouseMoveLooper = Looper<MouseMoveHandler>;

class MouseMoveHandler {
public:
    struct InputData {
        float yaw, pitch;
    };

public:
    MouseMoveHandler(BleCombo* combo)
        : combo{ combo }, yaw{ 0 }, pitch{ 0 }, prev_ms{ 0 }
    { }

    void operator()(const InputData& input) {
        yaw += input.yaw;
        pitch += input.pitch;
    }

    void reset() {
        yaw = pitch = 0;
    }

    void moveMouse();

private:
    BleCombo* combo;
    double yaw;
    double pitch;
    unsigned long prev_ms;
};

#endif /* _MOUSE_MOVE_LOOPER_H_ */