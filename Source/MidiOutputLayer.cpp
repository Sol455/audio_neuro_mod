//
// Created by Sol Harter on 01/09/2025.
//
#include "MidiOutputLayer.h"

//
// Created by Sol Harter on 29/08/2025.
//

void MidiOutputLayer::prepare (double fs)
    {
        sampleRate = fs;
        samplesPerCc = juce::jmax(1, (int) std::lround(sampleRate / rateHz));
        nextCcSample = 0;
    }

void MidiOutputLayer::process (int numSamples, juce::MidiBuffer& midi, int64_t blockStartSample)
    {
        if (ring != nullptr) {
            for (int sampleInBlock = 0; sampleInBlock < numSamples; ++sampleInBlock)
            {
                auto currentGlobalSample = blockStartSample + sampleInBlock;

                if (currentGlobalSample >= nextCcSample)
                {
                    auto lookbackSample = currentGlobalSample - lookbackDelaySamples;
                    float eegValue;
                    if (ring && ring->getValueAtSample(lookbackSample, eegValue)) {
                        auto midiValue = scaleEegToMidi(eegValue);
                        auto midiMsg = juce::MidiMessage::controllerEvent(chan, cc, midiValue);
                        midi.addEvent(midiMsg, sampleInBlock);
                        //DBG ("Midi Value: " << midiValue << "eegValue" << eegValue);
                    }

                    nextCcSample += samplesPerCc;
                }
            }
        }
    }

int MidiOutputLayer::scaleEegToMidi(float eegValue) const
{
    const int midi_sample = static_cast<int>(std::lround(juce::jlimit(0.0f, 1.0f, eegValue) * 127.0f));
    return midi_sample;
}


