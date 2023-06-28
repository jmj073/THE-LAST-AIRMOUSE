#ifndef _MOUSE_MOVE_LOOPER_H_
#define _MOUSE_MOVE_LOOPER_H_

#include <BleCombo.h>

#include <looper.h>

namespace mouse_move {
    class InputHandler;
    class OutputHandler;
    using MouseMoveLooper = Looper<InputHandler, OutputHandler>;

    enum InputMode { IMU, JOYSTICK };
    struct Move { float x, y; };

    class HandlerInterface {
    public:
        virtual void reset() { }
        virtual bool available() const = 0;
        virtual Move operator()(unsigned long interval_us) = 0;
        virtual ~HandlerInterface() { }
    };

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
            return (*handler)(interval_us);
        }

        bool available() const {
            return handler->available();
        }

        void reset() {
            handler->reset();
        }

    private:
        InputMode mode;
        HandlerInterface* handler;
        // float prev_pitch;
    };

    class OutputHandler {
    public:
        using InputData = Move;

    public:
        explicit OutputHandler(BleCombo* combo)
            : combo{ combo }, move_x{ 0 }, move_y{ 0 }, prev_ms{ 0 }
        { }

        void moveMouse();

    public: // OutputHandler trait for Looper
        void operator()(const InputData& input) {
            move_x += input.x;
            move_y += input.y;
        }

        void reset() {
            move_x = move_y = 0;
        }

    private:
        BleCombo* combo;
        double move_x;
        double move_y;
        unsigned long prev_ms;
    };

} /* mouse_move */

using mouse_move::MouseMoveLooper;

#endif /* _MOUSE_MOVE_LOOPER_H_ */