#ifndef _UTILS_H_
#define _UTILS_H_

#include <Arduino.h>
#include <functional>
#include <algorithm>
#include <limits>

#if __cplusplus < 201700L
namespace std {
    template<typename _Tp>
    constexpr const _Tp&
    clamp(const _Tp& __val, const _Tp& __lo, const _Tp& __hi)
    {
        __glibcxx_assert(!(__hi < __lo));
        return (__val < __lo) ? __lo : (__hi < __val) ? __hi : __val;
    }
}
#endif

template <typename Data>
class Measure {
public:
    using Handler = std::function<void(Data minv, Data meanv, Data maxv)>;

public:
    Measure(size_t measure_cnt, Handler on_measure_finished)
        : measure_cnt{ measure_cnt }
        , on_measure_finished{ std::move(on_measure_finished) }
    {
        resetMeasureValue();
    }

    void appendValue(Data value) {
        minv = std::min(minv, value);
        sumv += value;
        maxv = std::max(maxv, value);

        if (++cnt >= measure_cnt) {
            on_measure_finished(minv, sumv / measure_cnt, maxv);
            resetMeasureValue();
        }
    }

    size_t currentMeasureCount() const {
        return cnt;
    }

private:
    void resetMeasureValue() {
        cnt = 0;
        sumv = 0;
        minv = std::numeric_limits<Data>::max();
        maxv = std::numeric_limits<Data>::min();
    }

private:
    size_t measure_cnt;
    Handler on_measure_finished;
    size_t cnt;
    Data minv;
    Data sumv;
    Data maxv;
};

class MeasureTime: public Measure<unsigned long> {
public:
    using Measure::Measure;

    void measureStart() {
        start_us = micros();
    }

    void measureStop() {
        appendValue(micros() - start_us);
    }

private:
    unsigned long start_us;
};

#endif /* _UTILS_H_ */