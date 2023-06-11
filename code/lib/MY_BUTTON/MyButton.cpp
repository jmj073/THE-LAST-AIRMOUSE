#include "MyButton.h"

MyButton::MyButton(int pin, int mode, Handler on_changed)
    : pin{ pin }, on_changed{ on_changed }
    , debounce_time{ 0 }, prev_time{ 0 }
{
    pinMode(this->pin, mode);
    steady_state = digitalRead(this->pin);
}

MyButton::MyButton(MyButton&& o) {
    *this = std::move(o);
}

MyButton& MyButton::operator=(MyButton&& rhs) {
    if (this == &rhs) return *this;

    pin = rhs.pin;
    on_changed = rhs.on_changed;
    debounce_time = rhs.debounce_time;
    prev_time = rhs.prev_time;

    return *this;
}

void MyButton::loop() {
    bool curr_state = digitalRead(pin);
    if (steady_state == curr_state) return;

    unsigned long curr_time = millis();
    if (curr_time - prev_time >= debounce_time) {
        prev_time = curr_time;
        steady_state = curr_state;
        if (on_changed) on_changed(curr_state);
    }

}