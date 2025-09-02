//
// Created by Sol Harter on 01/09/2025.
//

#ifndef AUDIO_NEURO_MOD_TIMESTAMPMAPPER_H
#define AUDIO_NEURO_MOD_TIMESTAMPMAPPER_H

#pragma once
#include <atomic>
#include "lsl_cpp.h"

class timestampMapper {
public:
    void prepare(double sampleRate);

    void calibrate(double lslTime, int64_t hostSample);

    int64_t toHostSample(double lslTime);

private:
    std::atomic<double> a{48000.0};
    std::atomic<double> b{0.0};

    std::atomic<double> lastLslTime{0.0};
    std::atomic<int64_t> lastHostSample{0};
};


#endif //AUDIO_NEURO_MOD_TIMESTAMPMAPPER_H