#include "looper.h"

#include "joystick.h"
#include "measure.h"

using namespace keyboard;

static constexpr const long THRESHOLD = 1500;

auto InputHandler::operator()(unsigned long interval) -> InputData {
    (void)interval;

    auto& joystick = MyJoystick::getInstance();
    auto x = joystick.getx();
    auto y = joystick.gety();

#if 0
    // Serial.printf("%d %d\n", x, y);
    static Measure<int> measure(1000);
    measure.appendValue(abs(x));
#endif

    uint8_t key{};
    key |= x < -THRESHOLD ? KEY::KEY_D
        : (x > THRESHOLD ? KEY::KEY_A : 0);

    key |= y < -THRESHOLD ? KEY::KEY_W 
        : (y > THRESHOLD ? KEY::KEY_S : 0);

    return InputData(key);
}

void OutputHandler::operator()(const InputData& input) {
    if (key == input) return;
    uint8_t changed_key = key ^ input;

    auto foo = [&] (KEY key, uint8_t b) {
        if (changed_key & key) {
            if (input & key) combo->press(b);
            else combo->release(b);
        }
    };

    foo(KEY_W, 'w');
    foo(KEY_A, 'a');
    foo(KEY_S, 's');
    foo(KEY_D, 'd');

    key = input;
}

void OutputHandler::reset() {
    combo->releaseAll();
    key = (KEY)0;
}