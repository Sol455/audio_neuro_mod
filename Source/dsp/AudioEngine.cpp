//
// Created by Sol Harter on 15/09/2025.
//

#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
    sineModulator.initialise([](float x) { return std::sin(x); }, 1024);
}

void AudioEngine::prepare(double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    juce::dsp::ProcessSpec carrierSpec;
    carrierSpec.sampleRate = sampleRate;
    carrierSpec.maximumBlockSize = samplesPerBlock;
    carrierSpec.numChannels = numOutputChannels;

    carrier.prepare(carrierSpec);

    lowPassFilter.prepare(carrierSpec);
    lowPassFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    juce::dsp::ProcessSpec modSpec;
    modSpec.sampleRate = sampleRate;
    modSpec.maximumBlockSize = samplesPerBlock;
    modSpec.numChannels = 1; // Single channel for mod signal

    sineModulator.prepare(modSpec);

}

void AudioEngine::process(juce::AudioBuffer<float>& buffer, const Parameters& params,
                         const std::function<float(int64_t)>& getModulationValue)
{
    // Generate carrier
    carrier.setFrequency(params.carrierFreq);
    carrier.setAmplitude(params.carrierGain);
    carrier.process(buffer);

    std::vector<float> modValues;

    // Generate modulation values
    if (params.mode == ModulationMode::OpenLoop)
    {
        modValues = generateOLModulationValues(buffer.getNumSamples(), params);
    }
    else if (params.mode == ModulationMode::ClosedLoop && getModulationValue)
    {
        modValues = generateCLModulationValues(buffer.getNumSamples(),getModulationValue);

    }

    switch (params.modType) {
        case AudioEngine::ModulationType::AM:
            //applyFilterModulation(buffer, modValues);
            //DBG("AM");
            applyAmplitudeModulation(buffer, modValues);
            break;
        case AudioEngine::ModulationType::FM:
            applyFilterModulation(buffer, modValues);
            //DBG("FM");
            break;
        case AudioEngine::ModulationType::ISO:
            //DBG("ISO");

            //TO -DP
            break;
        default:
            // Fallback to ISO (both)
            //applyAmplitudeModulation(buffer, modValues);
            break;
    }

}

std::vector<float> AudioEngine::generateCLModulationValues(int numSamples, const std::function<float(int64_t)>& getModulationValue)
{
    std::vector<float> modValues(numSamples);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get raw mod value
        float modValue = getModulationValue(sample);
        modValues[sample] = juce::jlimit(0.1f, 1.0f, modValue);
    }

    return modValues;
}

std::vector<float> AudioEngine::generateOLModulationValues(int numSamples, const Parameters& params)
{
    sineModulator.setFrequency(params.modFreq);

    std::vector<float> modValues(numSamples);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float modValue = (sineModulator.processSample(0.0f) + 1.0f) * 0.5f; // 0 to 1
        float scaledMod = params.modDepth * modValue;
        modValues[sample] = params.minModDepth + (1.0f - params.minModDepth) * scaledMod;
    }

    return modValues;
}

//apply amplitude modulation
void AudioEngine::applyAmplitudeModulation(juce::AudioBuffer<float>& buffer, const std::vector<float>& modValues)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] *= modValues[sample];
        }
    }
}
// Apply filter modulation
void AudioEngine::applyFilterModulation(juce::AudioBuffer<float>& buffer, const std::vector<float>& modValues)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const int updateRate = 32;

    const float minCutoff = 50.0f;
    const float maxCutoff = 1000.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (sample % updateRate == 0)
        {
            float cutoff = minCutoff + (maxCutoff - minCutoff) * modValues[sample];
            lowPassFilter.setCutoffFrequency(cutoff);
        }

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[sample] = lowPassFilter.processSample(channel, channelData[sample]);
        }
    }
}

