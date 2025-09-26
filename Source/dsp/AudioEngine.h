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

    enum class ModulationType {AM, FM, ISO};

    struct Parameters {
        float carrierFreq = 440.0f;
        float carrierGain = 0.5f;
        float modFreq = 10.0f;
        float modDepth = 0.7f;
        float minModDepth = 0.15f;
        float filterCutoff = 1000.0f;
        float filterQ = 0.707f;
        ModulationMode mode = ModulationMode::ClosedLoop;
        ModulationType modType = ModulationType::AM;
        Carrier::WaveformType waveformType = Carrier::WaveformType::Sine;
    };

    AudioEngine();

    void prepare(double sampleRate, int samplesPerBlock, int numOutputChannels);
    void process(juce::AudioBuffer<float>& buffer, const Parameters& params,
                const std::function<float(int64_t)>& getModulationValue = nullptr);
    void setModulationCallback(std::function<void(float)> callback) {
        onModulationGenerated = callback;
    }

private:
    Carrier carrier;
    juce::dsp::Oscillator<float> sineModulator;
    juce::dsp::StateVariableTPTFilter<float> lowPassFilter;
    std::function<void(float)> onModulationGenerated;


    std::vector<float> generateOLModulationValues(int numSamples, const Parameters& params);
    std::vector<float> generateCLModulationValues(int numSamples, const std::function<float(int64_t)>& getModulationValue);

    void applyAmplitudeModulation(juce::AudioBuffer<float>& buffer, const std::vector<float>& modValues);
    void applyFilterModulation(juce::AudioBuffer<float>& buffer, const std::vector<float>& modValues);

};

#endif //AUDIO_NEURO_MOD_AUDIOENGINE_H