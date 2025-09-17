
#include "Carrier.h"

void Carrier::prepare (const juce::dsp::ProcessSpec& s)
{
    spec = s;
    oscillators.clear();
    oscillators.resize ((size_t) spec.numChannels);

    for (auto& osc : oscillators)
    {
        osc.initialise ([](float x) { return std::sin (x); }, 2048);
        osc.prepare (spec);
        osc.reset();
        osc.setFrequency (frequencyHz); // set once at prepare
    }
}


void Carrier::process (juce::AudioBuffer<float>& buffer)
{
    jassert ((int) spec.numChannels == buffer.getNumChannels());
    if (oscillators.empty()) return;

    const int numCh   = buffer.getNumChannels();
    const int numSmps = buffer.getNumSamples();

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* dst = buffer.getWritePointer (ch);
        auto& osc = oscillators[(size_t) ch];

        for (int n = 0; n < numSmps; ++n)
            dst[n] = amplitude * osc.processSample (0.0f);
    }
}

void Carrier::reset()
{
    for (auto& osc : oscillators)
        osc.reset();
}

void Carrier::setAmplitude (float newAmp)
{
    amplitude = juce::jlimit (0.0f, 1.0f, newAmp);
}

void Carrier::setFrequency (float newFreqHz)
{
    frequencyHz = juce::jmax (0.0f, newFreqHz);
    for (auto& osc : oscillators)
        osc.setFrequency (frequencyHz);
}
