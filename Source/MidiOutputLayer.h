//
// Created by Sol Harter on 29/08/2025.
//

#ifndef AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H
#define AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "lsl/EegRingBuf.h"
#include "OutputSyncLayer.h"
#include "macros.h"

class MidiOutputLayer
{
public:
    void prepare (double fs);
    void setChannel (int ch)                { chan = juce::jlimit(1, 16, ch); }
    void setCcNumber (int num)              { cc   = juce::jlimit(0, 127, num); }
    void setRateHz (double hz)              { rateHz = juce::jmax(1.0, hz); }
    void process (int numSamples, juce::MidiBuffer& midi, int64_t blockStartSample);
    void attachSyncLayer(OutputSyncLayer* syncLayer) { sync = syncLayer; }

private:

    int scaleEegToMidi(float eegValue) const;
    OutputSyncLayer* sync = nullptr;
    int chan = MIDI_OUTPUT_CHANNEL, cc = MIDI_OUTPUT_CC;
    double rateHz = 50.0, sampleRate = 44000.0;
    int samplesPerCc = 960;
    int64_t nextCcSample = 0;
};


#endif //AUDIO_NEURO_MOD_MIDIOUTPUTLAYER_H