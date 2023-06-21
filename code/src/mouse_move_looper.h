#ifndef _MOUSE_MOVE_LOOPER_H_
#define _MOUSE_MOVE_LOOPER_H_

#include <BleCombo.h>

#include "looper.h"

namespace mouse_move {
    class InputHandler;
    class OutputHandler;
    using MouseMoveLooper = Looper<InputHandler, OutputHandler>;

    enum InputMode { IMU, JOYSTICK };
    struct Move { float x, y; };

    class InputHandler {
    public:
        using InputData = Move;

    public:
        explicit InputHandler(InputMode mode);

        void begin();

        InputMode getInputMode() const { return mode; }
        void setInputMode(InputMode mode);

    public: // InputHandler trait for Looper
        InputData operator()(unsigned long interval_us) {
            return handler(interval_us);
        }

        bool available() const;

        void reset() { }

    private:
        InputMode mode;
        InputData (*handler)(unsigned long interval);
    };

    class OutputHandler {
    public:
        using InputData = Move;

    public:
        explicit OutputHandler(BleCombo* combo)
            : combo{ combo }, yaw{ 0 }, pitch{ 0 }, prev_ms{ 0 }
        { }

        void moveMouse();

    public: // OutputHandler trait for Looper
        void operator()(const InputData& input) {
            yaw += input.x;
            pitch += input.y;
        }

        void reset() {
            yaw = pitch = 0;
        }

    private:
        BleCombo* combo;
        double yaw;
        double pitch;
        unsigned long prev_ms;
    };

} /* mouse_move */

using mouse_move::MouseMoveLooper;

#endif /* _MOUSE_MOVE_LOOPER_H_ */