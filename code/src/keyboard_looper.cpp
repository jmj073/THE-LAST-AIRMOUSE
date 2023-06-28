#include "keyboard_looper.h"

#include "pins.h"
#include "joystick.h"

using namespace keyboard;

auto InputHandler::operator()(unsigned long interval_us) -> InputData {
    int x = joystickGetX();
    int y = joystickGetY();

    uint8_t key{};
    key |= x < -1000 ? KEY::KEY_D
        : (x > 1000 ? KEY::KEY_A : 0);

    key |= y < -1000 ? KEY::KEY_W 
        : (y > 1000 ? KEY::KEY_S : 0);

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