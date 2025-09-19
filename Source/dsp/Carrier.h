
#ifndef AUDIO_NEURO_MOD_CARRIER_H
#define AUDIO_NEURO_MOD_CARRIER_H
#include "juce_audio_basics/juce_audio_basics.h"

#include <juce_dsp/juce_dsp.h>

class Carrier
{
public:
    Carrier() = default;
    enum class WaveformType { Sine, Square, Saw, Triangle };

    void prepare (const juce::dsp::ProcessSpec& s);

    void process (juce::AudioBuffer<float>& buffer);

    void reset();

    void setAmplitude (float newAmp);
    void setFrequency (float newFreqHz);
    void setWaveform (WaveformType newWaveform);

    [[nodiscard]] float getAmplitude() const { return amplitude; }
    [[nodiscard]] float getFrequency() const { return frequencyHz; }

private:
    juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
    std::vector<std::array<juce::dsp::Oscillator<float>, 4>> oscillatorBank;

    float amplitude   = 0.5f;
    float frequencyHz = 440.0f;
    WaveformType waveform = WaveformType::Sine;

    static float sineWave(float x) { return std::sin(x); }
    static float squareWave(float x) { return (std::sin(x) > 0.0f) ? 1.0f : -1.0f; }
    static float sawWave(float x) { return (2.0f / juce::MathConstants<float>::pi) * (x - juce::MathConstants<float>::pi * std::floor(x / juce::MathConstants<float>::pi + 0.5f)); }
    static float triangleWave(float x) { return (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(x)); }
};


#endif //AUDIO_NEURO_MOD_CARRIER_H