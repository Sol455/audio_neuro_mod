//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_FILTER_MOD_H
#define AUDIO_NEURO_MOD_FILTER_MOD_H

#include <juce_dsp/juce_dsp.h>
#include "CFIRFilter.h"
#include "../macros.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class filterMod
{
public:

    void prepare (double fs, double centreHz, double Q);

    std::complex<float> filterComplex (float input);

    float makeModSignal(float env, float phase, float phase_offset, float percentile);

    void setParameterReferences(std::atomic<float>* modDepth, std::atomic<float>* minModDepth, std::atomic<float>* envMix, std::atomic<float>* modeMode);

    void setDCBlockingEnabled(bool enabled) { dcBlockingEnabled = enabled; }

    float processDCBlock(float input);

private:
    //modulation generation
    float makeSmoothModSignal(float env, float phase, float phase_offset, float percentile);
    float makeIsochronicModSignal(float env, float phase, float phase_offset, float percentile);
    //parameter refs
    std::atomic<float>* modDepthRef = nullptr;
    std::atomic<float>* minModDepthRef = nullptr;
    std::atomic<float>* envMixRef = nullptr;
    std::atomic<float>* modModeRef = nullptr;

    double fs_ = 0.0;
    CFIRFilter cf_;

    juce::dsp::IIR::Filter<float> dcBlocker;
    bool dcBlockingEnabled = true;

    double envelope_95_ref = 7.688501426439704;

};

#endif //AUDIO_NEURO_MOD_FILTER_H