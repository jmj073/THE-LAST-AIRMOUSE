#define BLE_SEND_INTERVAL 12UL // millisecond

#include "looper.h"

#include "utils.h"

using namespace mouse_move;

void OutputHandler::moveMouse() {
    uint32_t curr_ms = millis();
    if (curr_ms - prev_ms >= BLE_SEND_INTERVAL) {
        prev_ms = curr_ms;

        auto x = (signed char)std::clamp<int32_t>(move_x, -128, 127);
        auto y = (signed char)std::clamp<int32_t>(move_y, -128, 127);

        move_x -= x;
        move_y -= y;

        combo->move(-x, y);
    }
}