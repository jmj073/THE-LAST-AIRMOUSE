#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include <Arduino.h>
#include "pins.h"

#define JOYSTICK_OFFSET_X -1968
#define JOYSTICK_OFFSET_Y -1939

static inline
int joystickGetX() {
    return analogRead(PIN_JOYSTICK_X) + JOYSTICK_OFFSET_X;
}

static inline
int joystickGetY() {
    return analogRead(PIN_JOYSTICK_Y) + JOYSTICK_OFFSET_Y;
}

#endif /* _JOYSTICK_H_ */