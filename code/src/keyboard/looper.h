#ifndef _KEYBOARD_LOOPER_H_
#define _KEYBOARD_LOOPER_H_

#include <BleCombo.h>
#include <looper.h>

namespace keyboard {
    class InputHandler;
    class OutputHandler;
    using KeyboardLooper = Looper<InputHandler, OutputHandler>;

    enum KEY: uint8_t {
            KEY_W = 1<<0,
            KEY_A = 1<<1,
            KEY_S = 1<<2,
            KEY_D = 1<<3,
    };

    class InputHandler {
    public:
        using InputData = KEY;

    public:
        InputHandler()
            : enable{ false }
        { }

    public: // InputHandler trait for Looper
        InputData operator()(unsigned long interval);

        bool available() const {
            return enable;
        }

        void inputEnable() {
            enable = true;
        }

        void inputDisable() {
            enable = false;
        }

        void reset() { }

    private:
        bool enable;
    };

    class OutputHandler {
    public:
        using InputData = KEY;

    public:
        explicit OutputHandler(BleCombo* combo)
            : combo{ combo }, key{}
        { }

    public: // OutputHandler trait for Looper
        void operator()(const InputData& input);

        void reset();

    private:
        BleCombo* combo;
        KEY key;
    };
} /* keyboard */

using keyboard::KeyboardLooper;

#endif /* _KEYBOARD_LOOPER_H_ */