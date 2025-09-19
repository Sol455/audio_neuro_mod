
#include "Carrier.h"

void Carrier::prepare (const juce::dsp::ProcessSpec& s)
{
    spec = s;
    oscillatorBank.clear();
    oscillatorBank.resize((size_t)spec.numChannels);

    std::array<std::function<float(float)>, 4> waveFunctions = {
        sineWave, squareWave, sawWave, triangleWave
    };

    for (size_t ch = 0; ch < spec.numChannels; ++ch) {
        for (int waveType = 0; waveType < 4; ++waveType) {
            oscillatorBank[ch][waveType].initialise(waveFunctions[waveType], 2048);
            oscillatorBank[ch][waveType].prepare(spec);
            oscillatorBank[ch][waveType].reset();
            oscillatorBank[ch][waveType].setFrequency(frequencyHz);
        }
    }
}


void Carrier::process (juce::AudioBuffer<float>& buffer)
{
    jassert((int)spec.numChannels == buffer.getNumChannels());
    if (oscillatorBank.empty()) return;

    const int numCh = buffer.getNumChannels();
    const int numSmps = buffer.getNumSamples();
    const int waveformIndex = static_cast<int>(waveform);

    for (int ch = 0; ch < numCh; ++ch) {
        auto* dst = buffer.getWritePointer(ch);
        auto& osc = oscillatorBank[ch][waveformIndex]; //select waveform

        for (int n = 0; n < numSmps; ++n)
            dst[n] = amplitude * osc.processSample(0.0f);
    }
}

void Carrier::reset()
{
    for (auto& channelOscs : oscillatorBank) {
        for (auto& osc : channelOscs) {
            osc.reset();
        }
    }
}

void Carrier::setWaveform(WaveformType newWaveform)
{
    waveform = newWaveform;
}

void Carrier::setAmplitude (float newAmp)
{
    amplitude = juce::jlimit (0.0f, 1.0f, newAmp);
}

void Carrier::setFrequency(float newFreqHz) {
    frequencyHz = juce::jmax(0.0f, newFreqHz);

    for (auto& channelOscs : oscillatorBank) {
        for (auto& osc : channelOscs) {
            osc.setFrequency(frequencyHz);
        }
    }
}