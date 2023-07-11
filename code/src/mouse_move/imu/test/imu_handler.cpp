#if 0

#include <Arduino.h>
#include "../imu_handler.h"

struct TestOutputHandler {
        using InputData = mouse_move::OutputHandler::InputData;
        void reset() { }
        void operator()(const InputData& input);
};

using TestLooper = Looper<mouse_move::IMUHandler, TestOutputHandler>;

static TestLooper looper;

void setup() {
    Serial.begin(115200);
    looper.getInputHandler().begin();
}

void loop() {
    looper.loop();
}

void TestOutputHandler::operator()(const InputData& input) {
    Serial.print(input.x); Serial.print(' ');
    Serial.print(input.y); Serial.println();
}

#endif