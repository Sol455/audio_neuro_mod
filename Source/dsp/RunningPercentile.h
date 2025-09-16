//
// Created by Sol Harter on 09/09/2025.
//

#ifndef AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H
#define AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H

#include <juce_core/juce_core.h>
#include <vector>
#include <algorithm>
#include "../lsl/EegRingBuf.h"

class RunningPercentile {
public:
    RunningPercentile() : ringBuffer(4000) {}

    void addSample(float value) {
        EegSample sample{std::abs(value), 0};
        ringBuffer.addSample(sample);
        needsUpdate = true;
    }

    float getPercentile(float p = 0.95f) {
        if (needsUpdate && ++callsSinceUpdate >= updateInterval) {
            auto values = ringBuffer.getAllValues();

            if (!values.empty()) {
                std::sort(values.begin(), values.end());
                size_t idx = static_cast<size_t>(p * (values.size() - 1));
                cachedPercentile = values[idx];
            }

            callsSinceUpdate = 0;
            needsUpdate = false;
        }
        return cachedPercentile;
    }

    void clear() {
        ringBuffer.clear();
        needsUpdate = true;
        cachedPercentile = 1.0f;
        callsSinceUpdate = 0;
    }

private:
    EegRingBuf ringBuffer;
    float cachedPercentile = 1.0f;
    int callsSinceUpdate = 0;
    bool needsUpdate = true;
    static constexpr int updateInterval = 16; // Update every 16 calls (~0.1 seconds at 160Hz)
};

#endif //AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H