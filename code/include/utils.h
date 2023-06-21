#ifndef _UTILS_H_
#define _UTILS_H_

#include <Arduino.h>
#include <functional>

#if __cplusplus < 201700
namespace std {
    template<typename _Tp>
    constexpr const _Tp&
    clamp(const _Tp& __val, const _Tp& __lo, const _Tp& __hi)
    {
        __glibcxx_assert(!(__hi < __lo));
        return (__val < __lo) ? __lo : (__hi < __val) ? __hi : __val;
    }
}
#else
#include <algorithm>
#endif

class Measure {
public:
    Measure(size_t measure_cnt, std::function<void(unsigned long)> on_measure_finished)
        : measure_cnt{ measure_cnt }, on_measure_finished{ on_measure_finished }
    { }

    void start() {
       start_time = micros();
    }

    void stop() {
        time_sum += micros() - start_time;
        if (++cnt >= measure_cnt) {
            on_measure_finished(time_sum / measure_cnt);
            time_sum = cnt = 0;
        }
    }

private:
    size_t measure_cnt;
    std::function<void(unsigned long)> on_measure_finished;
    unsigned long start_time;
    long cnt = 0;
    unsigned long time_sum = 0;
};

#endif /* _UTILS_H_ */