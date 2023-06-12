#ifndef _KEYBOARD_LOOPER_H_
#define _KEYBOARD_LOOPER_H_

#include <BleCombo.h>

#include "looper.h"

class KeyboardHandler;
using KeyboardLooper = Looper<KeyboardHandler>;

class KeyboardHandler {
public:
    enum KEY: uint8_t {
        KEY_W = 1<<0,
        KEY_A = 1<<1,
        KEY_S = 1<<2,
        KEY_D = 1<<3,
    };
    using InputData = KEY;

public:
    KeyboardHandler(BleCombo* combo)
        : combo{ combo }, key{}
    { }

    void operator()(const InputData& input);

    void reset();

private:
    

private:
    BleCombo* combo;
    KEY key;
};

#endif /* _KEYBOARD_LOOPER_H_ */