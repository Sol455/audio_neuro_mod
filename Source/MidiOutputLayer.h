//
// Created by Sol Harter on 29/08/2025.
//

#ifndef AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H
#define AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include "juce_audio_devices/midi_io/juce_MidiDevices.h"
#include "lsl/eegRingBuffer.h"

class MidiOutputLayer
{
public:
    void setIAC (bool toggle)               { iacEnabled = toggle; }
    void attachRing (EegRingBuffer* rb)     { ring = rb; }
    void setChannel (int ch)                { chan = juce::jlimit(1, 16, ch); }
    void setCcNumber (int num)              { cc   = juce::jlimit(0, 127, num); }
    void setRateHz (double hz)              { rateHz = juce::jmax(1.0, hz); }

    void prepare (double fs);

    void process (int numSamples, juce::MidiBuffer& midi);

private:
    bool tryReadLatestFromRing (float& out) noexcept;

    void openIAC();

    void sendIAC(int value);

    EegRingBuffer* ring = nullptr;
    int chan = 1, cc = 74;
    double rateHz = 50.0, sampleRate = 48000.0;
    int samplesPerCc = 960, samplesUntilNextCc = 0;
    float held = 0.0f;
    std::unique_ptr<juce::MidiOutput> iacOut;
    int iacChannel = 1;         // 1..16
    int iacCc      = 74;        // CC number to map (cutoff convention)
    std::atomic<bool> iacEnabled { true };
};


#endif //AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H