//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_FILTER_MOD_H
#define AUDIO_NEURO_MOD_FILTER_MOD_H

#include <juce_dsp/juce_dsp.h>
#include "CFIRFilter.h"
#include "RunningPercentile.h"

class filterMod
{
public:

    void prepare (double fs, double centreHz, double Q);

    void setBand (double centreHz, double Q);

    float filter (float input);

    std::complex<float> filterComplex (float input);

    float makeModSignalReal(float sample, RunningPercentile& percentile);

    float makeModSignalComplex(float env, float phase, RunningPercentile& percentile);


private:
    double fs_ = 0.0;
    juce::dsp::IIR::Filter<float> bp_;
    CFIRFilter cf_;

    double envelope_95_ref = 7.688501426439704;
    double mod_depth = 0.9f;
    double mod_min_depth = 0.15f;
};

#endif //AUDIO_NEURO_MOD_FILTER_H