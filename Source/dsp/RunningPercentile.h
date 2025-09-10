//
// Created by Sol Harter on 09/09/2025.
//

#ifndef AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H
#define AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H
#include <juce_core/juce_core.h>

class RunningPercentile {
public:
    void addSample(float value) {
        samples.push_back(std::abs(value));
        if (samples.size() > MAX_SAMPLES) {
            samples.pop_front();
        }
    }

    float getPercentile(float p = 0.95f) {
        if (samples.empty()) return 1.0f;

        auto sorted = samples;
        std::sort(sorted.begin(), sorted.end());
        size_t idx = static_cast<size_t>(p * (sorted.size() - 1));
        return sorted[idx];
    }

    void clear() { samples.clear(); }

private:
    std::deque<float> samples;
    static constexpr size_t MAX_SAMPLES = 16000;

};


#endif //AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H