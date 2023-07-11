#include "joystick_handler.h"
#include "joystick.h"
#include "measure.h"

using namespace mouse_move;

static constexpr const double MOUSE_SPEED_JOYSTICK_X = 0.3;
static constexpr const double MOUSE_SPEED_JOYSTICK_Y = 0.3;

bool JoystickHandler::available() const {
    return true;
}

static inline
long _discard_joystick_value(long value) {
    return abs(value) < 100 ? 0 : value;
}

Move JoystickHandler::operator()(unsigned long interval_us) {
    auto& joystick = MyJoystick::getInstance();
    auto x = _discard_joystick_value(joystick.getx());
    auto y = _discard_joystick_value(joystick.gety());

#if 0
    // 이상하게 이거 빼면 joystick값 이상해지니까 빼면 안됨
    static Measure<int> measure(1000);
    measure.appendValue(abs(x));
    // measure.appendValue(y);
#endif

    return {
        .x = (int64_t)interval_us * x / (1e6 / MOUSE_SPEED_JOYSTICK_X),
        .y = (int64_t)interval_us * y / (1e6 / MOUSE_SPEED_JOYSTICK_Y),
    };
}


InputHandler::InputHandler(InputMode mode)
{
    setInputMode(mode);
}