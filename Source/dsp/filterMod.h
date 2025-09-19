//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_FILTER_MOD_H
#define AUDIO_NEURO_MOD_FILTER_MOD_H

#include <juce_dsp/juce_dsp.h>
#include "CFIRFilter.h"

class filterMod
{
public:

    void prepare (double fs, double centreHz, double Q);

    void setBand (double centreHz, double Q);

    float filter (float input);

    std::complex<float> filterComplex (float input);

    float makeModSignalReal(float sample, float percentile);

    float makeModSignalComplex(float env, float phase, float phase_offset, float percentile);

    void setParameterReferences(std::atomic<float>* modDepth, std::atomic<float>* minModDepth, std::atomic<float>* envMix);


private:
    //parameter refs
    std::atomic<float>* modDepthRef = nullptr;
    std::atomic<float>* minModDepthRef = nullptr;
    std::atomic<float>* envMixRef = nullptr;


    double fs_ = 0.0;
    juce::dsp::IIR::Filter<float> bp_;
    CFIRFilter cf_;

    double envelope_95_ref = 7.688501426439704;

};

#endif //AUDIO_NEURO_MOD_FILTER_H