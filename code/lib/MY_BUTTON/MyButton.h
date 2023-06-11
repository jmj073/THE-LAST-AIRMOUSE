/* refer to https://github.com/ArduinoGetStarted/button */

#ifndef _MY_BUTTON_H_
#define _MY_BUTTON_H_

#include <Arduino.h>

class MyButton {
public:
    using Handler = void (*)(bool);

public: // delete function
    MyButton(const MyButton&) = delete;
    MyButton& operator=(const MyButton&) = delete;

public:
    MyButton(int pin, Handler on_changed = nullptr)
        :  MyButton(pin, INPUT_PULLUP, on_changed) { }
    MyButton(int pin, int mode, Handler on_changed = nullptr);
    MyButton(MyButton&& other);

    MyButton& operator=(MyButton&& rhs);

    void update();
    void loop();

    void on_change(Handler on_changed) {
        this->on_changed = on_changed;
    }

    bool get_state() const {
        return steady_state;
    }

    void set_debounce_time(unsigned long debounce_time) {
        this->debounce_time = debounce_time;
    }

    ~MyButton() {
        pin = -1;
        on_changed = nullptr;
    }

private:
    int pin;
    void (*on_changed)(bool state);
    unsigned long debounce_time; // ms
    unsigned long prev_time;
    bool steady_state;
};

#endif /* _MY_BUTTON_H_ */