#include "keyboard_looper.h"


void KeyboardHandler::operator()(const InputData& input) {
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

void KeyboardHandler::reset() {
    combo->releaseAll();
    key = (KEY)0;
}