#include "looper.h"

#include "joystick_handler.h"
#include "imu/imu_handler.h"

using namespace mouse_move;

static IMUHandler imu_handler;
static JoystickHandler joystick_handler;

void InputHandler::begin() {
    imu_handler.begin();
    joystick_handler.begin();
}

void InputHandler::setInputMode(InputMode mode) {
    switch (mode) {
        case IMU:
            handler = &imu_handler;
            break;
        case JOYSTICK:
            handler = &joystick_handler;
            break;
    }

    this->mode = mode;
}