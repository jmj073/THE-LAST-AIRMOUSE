#include "joystick_handler.h"
#include "joystick.h"
#include "measure.h"

using namespace mouse_move;

static constexpr const float MOUSE_SPEED_JOYSTICK_X = 0.3;
static constexpr const float MOUSE_SPEED_JOYSTICK_Y = 0.3;

bool JoystickHandler::available() const {
    return true;
}

static inline
int _discard_joystick_value(int value) {
    return abs(value) < 200 ? 0 : value;
}

Move JoystickHandler::operator()(unsigned long interval_us) {
    int x = _discard_joystick_value(joystickGetX());
    int y = _discard_joystick_value(joystickGetY());

#if 0
    static Measure<int> measure(1000);
    measure.appendValue(x);
    // measure.appendValue(y);
#endif

    return {
        .x = (int64_t)interval_us * x / (1e6f / MOUSE_SPEED_JOYSTICK_X),
        .y = (int64_t)interval_us * y / (1e6f / MOUSE_SPEED_JOYSTICK_Y),
    };
}


InputHandler::InputHandler(InputMode mode)
{
    setInputMode(mode);
}