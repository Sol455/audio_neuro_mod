
#ifndef AUDIO_NEURO_MOD_CARRIER_H
#define AUDIO_NEURO_MOD_CARRIER_H
#include "juce_audio_basics/juce_audio_basics.h"

#include <juce_dsp/juce_dsp.h>

class Carrier
{
public:
    Carrier() = default;

    void prepare (const juce::dsp::ProcessSpec& s);

    void process (juce::AudioBuffer<float>& buffer);

    void reset();

    void setAmplitude (float newAmp);
    void setFrequency (float newFreqHz);

    [[nodiscard]] float getAmplitude() const { return amplitude; }
    [[nodiscard]] float getFrequency() const { return frequencyHz; }

private:
    juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
    std::vector<juce::dsp::Oscillator<float>> oscillators;

    float amplitude   = 0.5f;
    float frequencyHz = 440.0f;
};


#endif //AUDIO_NEURO_MOD_CARRIER_H