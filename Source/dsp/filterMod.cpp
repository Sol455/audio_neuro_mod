//
// Created by Sol Harter on 28/08/2025.
//

#include "filterMod.h"

void filterMod::prepare (double fs, double centreHz, double Q)
{
    fs_ = fs;
    cf_.prepare(8.0, 12.0, fs, CFIR_DELAY_SAMPLES, 500, 2000); // prepare complex filter @TO-DO have the alpha band measured and set per subject.

    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(fs, 1.0f);
    dcBlocker.coefficients = coeffs;
    dcBlocker.reset();
}

void filterMod::setParameterReferences(std::atomic<float>* modDepth, std::atomic<float>* minModDepth, std::atomic<float>* envMix, std::atomic<float>* modeMode)
{
    modDepthRef = modDepth;
    minModDepthRef = minModDepth;
    envMixRef = envMix;
    modModeRef = modeMode;
}

std::complex<float> filterMod::filterComplex (float input) {
    std::complex<float> analyticSignal = cf_.processSample(input);
    return analyticSignal;;
}

float filterMod::processDCBlock(float input)
{
    if (!dcBlockingEnabled)
        return input;

    return dcBlocker.processSample(input);
}

float filterMod::makeModSignal(float env, float phase, float phase_offset, float percentile)
{
    int modModeIndex = modModeRef ? static_cast<int>(modModeRef->load()) : 0;

    switch (modModeIndex) {
        case 0: // AM
        case 1: // FM
            return makeSmoothModSignal(env, phase, phase_offset, percentile);
        case 2: // ISO
            return makeIsochronicModSignal(env, phase, phase_offset, percentile);
        default:
            return makeSmoothModSignal(env, phase, phase_offset, percentile);
    }

}

float filterMod::makeSmoothModSignal(float env, float phase, float phase_offset, float percentile)
{
    float mod_depth = modDepthRef ? modDepthRef->load() : 0.5f;
    float min_mod_depth = minModDepthRef ? minModDepthRef->load() : 0.1f;
    float env_mix = envMixRef ? envMixRef->load() : 0.5f;

    float env_factor = env / percentile; //Scale modulation relative to running percentile

    // env_mix controls i.e. how much this envelope factor affects the modulation depth
    // At env_mix = 0: depth = mod_depth (constant), just relies on phase
    // At env_mix = 1: depth = env_factor * mod_depth (full envelope influence)
    float depth = mod_depth * (1.0f + (env_factor - 1.0f) * env_mix);
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

//Isochronic modulation
float filterMod::makeIsochronicModSignal(float env, float phase, float phase_offset, float percentile)
{
    float mod_depth = modDepthRef ? modDepthRef->load() : 0.5f;
    float min_mod_depth = minModDepthRef ? minModDepthRef->load() : 0.1f;
    float env_mix = envMixRef ? envMixRef->load() : 0.5f;

    float env_factor = env / percentile; //Scale modulation relative to running percentile

    // env_mix controls
    float depth = mod_depth * (1.0f + (env_factor - 1.0f) * env_mix);
    depth = juce::jlimit(0.0f, 1.0f, depth);

    // Generate phase-aligned pulse with fixed volume
    float adjusted_phase = phase + phase_offset;
    float normalized_phase = std::fmod(adjusted_phase, 2.0f * M_PI) / (2.0f * M_PI);
    if (normalized_phase < 0) normalized_phase += 1.0f;

    float pulse = (normalized_phase < 0.5f) ? 1.0f : 0.0f;

    // Apply envelope-modulated depth to pulse
    float amp_mod_component = depth * pulse;

    return min_mod_depth + (1.0f - min_mod_depth) * amp_mod_component;
}


