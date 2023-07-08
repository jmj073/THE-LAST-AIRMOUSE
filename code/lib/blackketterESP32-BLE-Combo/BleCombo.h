#ifndef ESP32_BLE_COMBO_H
#define ESP32_BLE_COMBO_H
#include "BleComboKeyboard.h"
#include "BleComboMouse.h"

class BleCombo {
public:
    BleCombo(std::string deviceName = "ESP32 Combo", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100)
        : _keyboard(std::move(deviceName), std::move(deviceManufacturer), batteryLevel), _mouse(&_keyboard)
    { }

    void begin(void) {
        _keyboard.begin();
        _mouse.begin();
    }

    void end(void) {
        _mouse.end();
        _keyboard.end();
    }

    void setBatteryLevel(uint8_t level) {
        _keyboard.setBatteryLevel(level);
    }

    bool isConnected(void) {
        return _keyboard.isConnected();
    }

public: /* keyboard */
    size_t press(uint8_t k) {
        return _keyboard.press(k);
    }

    size_t release(uint8_t k) {
        return _keyboard.release(k);
    }

    void releaseAll(void) {
        _keyboard.releaseAll();
    }

public: /* mouse */
    void click(uint8_t b = MOUSE_LEFT) {
        _mouse.click(b);
    }
    void move(signed char x, signed char y, signed char wheel = 0, signed char hWheel = 0) {
        _mouse.move(x, y, wheel, hWheel);
    }
    void mousePress(uint8_t b = MOUSE_LEFT) {
        _mouse.press(b);
    }
    void mouseRelease(uint8_t b = MOUSE_LEFT) {
        _mouse.release(b);
    }
    bool isPressed(uint8_t b = MOUSE_LEFT) {
        return _mouse.isPressed(b);
    }


private:
    BleComboMouse _mouse;
    BleComboKeyboard _keyboard;
};

#endif
