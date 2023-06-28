#ifndef _MEASURE_H_
#define _MEASURE_H_

#include <Arduino.h>
#include <functional>
#include <algorithm>
#include <limits>

template <typename Data>
class Measure {
public:
    using Handler = std::function<void(Data minv, Data meanv, Data maxv)>;

public:
    Measure(size_t measure_cnt, Handler on_measure_finished=default_handler)
        : measure_cnt{ measure_cnt }
        , on_measure_finished{ std::move(on_measure_finished) }
    {
        resetMeasureValue();
    }

    void appendValue(Data value) {
        if (cnt++) {
            minv = std::min(minv, value);
            sumv += value;
            maxv = std::max(maxv, value);

            if (cnt > measure_cnt) {
                on_measure_finished(minv, sumv / measure_cnt, maxv);
                resetMeasureValue();
            }
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

public:
    static void default_handler(Data minv, Data meanv, Data maxv) {
        Serial.print("min: ");  Serial.print(minv);  Serial.print(", ");
        Serial.print("mean: "); Serial.print(meanv); Serial.print(", ");
        Serial.print("max: ");  Serial.print(maxv);  Serial.println();
    }

private:
    size_t measure_cnt;
    Handler on_measure_finished;
    size_t cnt;
    Data minv;
    Data sumv;
    Data maxv;
};

class MeasureTime: public Measure<unsigned long long> {
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
#endif /* _MEASURE_H_ */