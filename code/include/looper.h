#ifndef _LOOPER_H_
#define _LOOPER_H_

#include <Arduino.h>
#include <functional>
#include <utility>

/* type trait

InputHandler:
    type: InputData
    function: InputData operator()(unsigned long interval_us)
    function: void reset()
    function: bool available() const

OutputHandler:
    type: InputData
    function: void operator()(InputData input) // or (const InputData& input), ...
    function: reset()
*/

template <typename InputHandler, typename OutputHandler>
class Looper {
public: // define type
    using InputData = typename InputHandler::InputData;

public: // delete function
    Looper(const Looper&) = delete;
    Looper& operator=(const Looper&) = delete;
     Looper(Looper&&) = delete;
    Looper& operator=(Looper&&) = delete;

public:
    Looper(InputHandler iHandler = InputHandler(), OutputHandler oHandler = OutputHandler())
        : iHandler{ std::move(iHandler) }, oHandler{ std::move(oHandler) }
    { }

    InputHandler& getInputHandler() {
        return iHandler;
    }
    const InputHandler& getInputHandler() const {
        return iHandler;
    }

    OutputHandler& getOutputHandler() {
        return oHandler;
    }
    const OutputHandler& getOutputHandler() const {
        return oHandler;
    }

    void reset();
    void loop();

private:
    InputHandler iHandler;
    OutputHandler oHandler;
    unsigned long prev_us;
    bool isFirst = true;
};

template <typename IH, typename OH>
void Looper<IH, OH>::reset() {
    isFirst = true;
    oHandler.reset();
}

template <typename IH, typename OH>
void Looper<IH, OH>::loop() {
    if (!iHandler.available()) {
        isFirst = true;
        return;
    }
    
    if (isFirst) {
        isFirst = false;
        iHandler.reset();
        prev_us = micros();
        return;
    }

    auto curr_us = micros();
    unsigned long interval = curr_us - prev_us;
    prev_us = curr_us;

    oHandler(iHandler(interval));
}

#endif /* _LOOPER_H_ */