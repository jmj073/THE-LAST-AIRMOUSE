#include "mouse_move_looper.h"
#include "utils.h"

#define BLE_SEND_INTERVAL 10UL // millisecond

void MouseMoveHandler::moveMouse() {
    uint32_t curr_ms = millis();
    if (curr_ms - prev_ms >= BLE_SEND_INTERVAL) {
        prev_ms = curr_ms;

        auto y = (signed char)std::clamp<int32_t>(yaw, -128, 127);
        auto p = (signed char)std::clamp<int32_t>(pitch, -128, 127);

        yaw -= y;
        pitch -= p;

        combo->move(-y, p);
    }
}
