#ifndef _TMP_H_
#define _LOOPER_H_

#include <Arduino.h>
#include <functional>
#include <utility>

template <typename OutputHandler>
class Looper {
public: // define type
    using InputData = typename OutputHandler::InputData;
    using InputHandler = std::function<InputData(unsigned long interval)>;

public: // delete function
    Looper(const Looper&) = delete;
    Looper& operator=(const Looper&) = delete;
     Looper(Looper&&) = delete;
    Looper& operator=(Looper&&) = delete;

public:
    template <typename ... Types>
    Looper(InputHandler iHandler, Types&& ... args)
        : iHandler { std::move(iHandler) }, oHandler{ std::forward<Types>(args) ... }
    { }

    void setInputHandler(InputHandler handler);

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

template <typename H>
void Looper<H>::setInputHandler(InputHandler handler) {
    this->isFirst = true;
    this->iHandler = std::move(handler);
}


template <typename H>
void Looper<H>::reset() {
    isFirst = true;
    oHandler.reset();
}

template <typename H>
void Looper<H>::loop() {
    if (isFirst) {
        isFirst = false;
        prev_us = micros();
        return;
    }

    auto curr_us = micros();
    unsigned long interval = curr_us - prev_us;
    prev_us = curr_us;

    auto input = iHandler(interval);
    oHandler(std::move(input));
}

#endif /* _LOOPER_H_ */