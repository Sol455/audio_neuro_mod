
#ifndef AUDIO_NEURO_MOD_CARRIER_H
#define AUDIO_NEURO_MOD_CARRIER_H
#include "juce_audio_basics/juce_audio_basics.h"

#include <juce_dsp/juce_dsp.h>

/**
 * Simple sine carrier generator.
 * - Uses juce::dsp::Oscillator<float>
 * - Frequency and amplitude set externally
 * - process() always overwrites the buffer
 */
class Carrier
{
public:
    Carrier() = default;

    /** Prepare with a ProcessSpec */
    void prepare (const juce::dsp::ProcessSpec& s);

    /** Render into buffer (overwrites contents). */
    void process (juce::AudioBuffer<float>& buffer);

    /** Reset oscillator phases to 0 */
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


// class Carrier {
//
// public:
//     void prepare(const double sampleRate, const int numChannels);
//     void process(juce::AudioBuffer<float> &buffer);
//     [[nodiscard]] float getAmplitude() const {return amplitude; }
//     [[nodiscard]] float getFrequency() const {return frequency; }
//     void setAmplitude(const float newAmplitude) {amplitude = newAmplitude; }
//     void setFrequency(const float newFrequency) {frequency = newFrequency; }
//
// private:
//     int samplesPerBlock = 0;
//     float amplitude = 0.2f;
//     float frequency = 440.0f;
//     float currentSampleRate = 0.0f;
//     float timeIncrement = 0.0f;
//     std::vector<float> currentTime;
//
// };


#endif //AUDIO_NEURO_MOD_CARRIER_H