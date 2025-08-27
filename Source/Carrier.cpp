
#include "Carrier.h"

void Carrier::prepare(const double sampleRate, const int numChannels) {
    currentSampleRate = static_cast<float>(sampleRate);
    timeIncrement = 1.0f / currentSampleRate;
    currentTime.resize(static_cast<size_t>(numChannels), 0.0f);
}

void Carrier::process(juce::AudioBuffer<float> &buffer) {

    if (currentTime.size() != static_cast<size_t>(buffer.getNumChannels())) {
        return;
    }

    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {

        auto* output = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
            output[sample] = amplitude * std::sinf(juce::MathConstants<float>::twoPi *frequency * currentTime[channel]);
            currentTime[channel] += timeIncrement;
        }
    }
}
