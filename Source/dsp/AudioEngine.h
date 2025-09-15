//
// Created by Sol Harter on 15/09/2025.
//

#ifndef AUDIO_NEURO_MOD_AUDIOENGINE_H
#define AUDIO_NEURO_MOD_AUDIOENGINE_H

#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Carrier.h"

class AudioEngine
{
public:
    enum class ModulationMode { OpenLoop, ClosedLoop };

    struct Parameters {
        float carrierFreq = 440.0f;
        float carrierGain = 0.5f;
        float modFreq = 10.0f;
        float modDepth = 0.7f;
        float minModDepth = 0.15f;
        ModulationMode mode = ModulationMode::OpenLoop;
    };

    AudioEngine();

    void prepare(double sampleRate, int samplesPerBlock, int numOutputChannels);
    void process(juce::AudioBuffer<float>& buffer, const Parameters& params,
                const std::function<float(int64_t)>& getModulationValue = nullptr);

private:
    Carrier carrier;
    juce::dsp::Oscillator<float> sineModulator;

    void processOpenLoopModulation(juce::AudioBuffer<float>& buffer, const Parameters& params);
    void processClosedLoopModulation(juce::AudioBuffer<float>& buffer, const std::function<float(int64_t)>& getModulationValue);
};

#endif //AUDIO_NEURO_MOD_AUDIOENGINE_H