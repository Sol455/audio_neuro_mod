//
// Created by Sol Harter on 28/08/2025.
//

#include "filterMod.h"

void filterMod::prepare (double fs, double centreHz, double Q)
{
    //BP
    fs_ = fs;
    setBand(centreHz, Q);
    bp_.reset();

    cf_.prepare(8.0, 12.0, fs, 0, 500, 2000);
}

void filterMod::setParameterReferences(std::atomic<float>* modDepth, std::atomic<float>* minModDepth)
{
    modDepthRef = modDepth;
    minModDepthRef = minModDepth;
}

float filterMod::filter (float input) {

    //output_sample
    return bp_.processSample (input);
}

float filterMod::makeModSignalReal(float sample, RunningPercentile& percentile)
{

    //DBG ("Sample in: " << sample);
    float mod_depth = modDepthRef ? modDepthRef->load() : 0.5f;
    float min_mod_depth = minModDepthRef ? minModDepthRef->load() : 0.1f;

    //normalise raw sample
    float ref95 = percentile.getPercentile(0.95f); //7.688501426439704e-05 envelope_95_ref; //

    float depth = 0.5f * ((sample / ref95) + 1.0f);

    //scale by modulation depth
    depth *= mod_depth;

    //clip into [0,1]
    depth = juce::jlimit(0.0f, 1.0f, depth);

    //apply min/max modulation depth window
    float modulator_signal = min_mod_depth + (1.0 - min_mod_depth) * depth;

    return modulator_signal;

}

std::complex<float> filterMod::filterComplex (float input) {

    std::complex<float> analyticSignal = cf_.processSample(input);
    return analyticSignal;;
}

float filterMod::makeModSignalComplex(float env, float phase, float phase_offset, RunningPercentile& percentile)
{
    float envelope_95_percentile = percentile.getPercentile(0.95f);
    float mod_depth = modDepthRef ? modDepthRef->load() : 0.5f;
    float min_mod_depth = minModDepthRef ? minModDepthRef->load() : 0.1f;

    //depth
    float depth = (env / envelope_95_percentile) * mod_depth;

    //clip
    depth = juce::jlimit(0.0f, 1.0f, depth);

    //Reconstruct modulation signal
    float cos_term = std::cos(phase + phase_offset);

    //Shift cosine into [0, 1]
    float cos_shifted = 0.5f * (1.0f + cos_term);

    //Envelope-weighted cosine
    float amp_mod_component = depth * cos_shifted;

    //apply minimum modulation depth floor
    float modulator_signal = min_mod_depth + (1.0f - min_mod_depth) * amp_mod_component;

    return modulator_signal;

}


void filterMod::setBand(double centreHz, double Q) {
    jassert (fs_ > 0.0);
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeBandPass (fs_, centreHz, Q);
    bp_.coefficients = coeff;
}
