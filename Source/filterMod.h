//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_FILTER_MOD_H
#define AUDIO_NEURO_MOD_FILTER_MOD_H

#pragma once
#include <juce_dsp/juce_dsp.h>

class filterMod
{
public:
    void prepare (double fs, double centreHz, double Q);

    void setBand (double centreHz, double Q);

    inline float process (float x) noexcept
    {
        return bp_.processSample (x);
    }

private:
    double fs_ = 0.0;
    juce::dsp::IIR::Filter<float> bp_;
};

#endif //AUDIO_NEURO_MOD_FILTER_H