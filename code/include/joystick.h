#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "settings.h"
#include "pins.h"
#include <Arduino.h>

class Joystick {
public:
    Joystick(int pinx, int piny)
        : pinx{ pinx }, piny{ piny }
    { }

    long getx() { return analogRead(pinx) + offsetx; }
    long gety() { return analogRead(piny) + offsety; }

    void setOffsetX(long offset) { offsetx = offset; }
    long getOffsetX() const { return offsetx; }
    void setOffsetY(long offset) { offsety = offset; }
    long getOffsetY() const { return offsety; }

private:
    int pinx;
    int piny;

    long offsetx = 0;
    long offsety = 0;
};

class MyJoystick {
private:
    static constexpr const size_t SAMPLING_COUNT = 1000;
private:
    MyJoystick()
        : handle(PIN_JOYSTICK_X, PIN_JOYSTICK_Y)
    { }
    MyJoystick(const MyJoystick&) = delete;
    MyJoystick& operator=(const MyJoystick&) = delete;
    ~MyJoystick() { }

public:
    void begin() {
        long sumx{}, sumy{};
        for (size_t i = 0; i < SAMPLING_COUNT; ++i) {
            sumx += handle.getx();
            sumy += handle.gety();
        }
        handle.setOffsetX(-(sumx / SAMPLING_COUNT));
        handle.setOffsetY(-(sumy / SAMPLING_COUNT));
    }
    static MyJoystick& getInstance() {
        static MyJoystick instance;
        return instance;
    }

#ifdef ENABLE_PROTOTYPE
    long getx() { return handle.gety(); }
    long gety() { return handle.getx(); }
#else /* ENABLE_PROTOTYPE */
    long getx() { return handle.getx(); }
    long gety() { return -handle.gety(); }
#endif /* ENABLE_PROTOTYPE */

private:
    Joystick handle;
};

#endif /* _JOYSTICK_H_ */