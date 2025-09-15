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

    // Apply modulation
    if (params.mode == ModulationMode::OpenLoop)
    {
        processOpenLoopModulation(buffer, params);
    }
    else if (params.mode == ModulationMode::ClosedLoop && getModulationValue)
    {
        processClosedLoopModulation(buffer, getModulationValue);
    }
}

void AudioEngine::processOpenLoopModulation(juce::AudioBuffer<float>& buffer, const Parameters& params)
{
    sineModulator.setFrequency(params.modFreq);

    std::vector<float> modValues(buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float modValue = (sineModulator.processSample(0.0f) + 1.0f) * 0.5f;
        float ampModComponent = params.modDepth * modValue;
        modValues[sample] = params.minModDepth + (1.0f - params.minModDepth) * ampModComponent;
    }

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= modValues[sample];
        }
    }
}

void AudioEngine::processClosedLoopModulation(juce::AudioBuffer<float>& buffer,
                                             const std::function<float(int64_t)>& getModulationValue)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float modValue = getModulationValue(sample);
            modValue = juce::jlimit(0.1f, 1.0f, modValue);

            channelData[sample] *= modValue;
        }
    }
}