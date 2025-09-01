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

    void prepare (double fs)
    {
        if (iacEnabled) openIAC();
        sampleRate = fs;
        samplesPerCc = juce::jmax(1, (int) std::lround(sampleRate / rateHz));
        samplesUntilNextCc = 0;
        held = 0.0f;
    }

    void process (int numSamples, juce::MidiBuffer& midi)
    {
        if (ring != nullptr)
        {
            float latest = held;
            if (tryReadLatestFromRing(latest))
                held = latest;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            if (samplesUntilNextCc <= 0)
            {
                //DBG ("Latest: " << held);
                const int v = (int) std::lround(juce::jlimit(0.0f, 1.0f, held) * 127.0f);
                //DBG ("Midi Out: " << v);
                // midi.addEvent(juce::MidiMessage::controllerEvent(chan, cc, v), i);
                if (iacEnabled) sendIAC(v);

                samplesUntilNextCc += samplesPerCc;
            }
            --samplesUntilNextCc;
        }
    }

private:
    bool tryReadLatestFromRing (float& out) noexcept
    {
        if (!ring) return false;

        // Drain everything available this block
        const int toRead = ring->available();
        if (toRead <= 0) return false;

        auto rv = ring->beginRead(toRead);     // { const EegSample* p1; int n1; const EegSample* p2; int n2; ... }
        const int avail = rv.total();
        if (avail <= 0) { ring->finishRead(0); return false; }

        const EegSample* last =
            (rv.n2 > 0 ? &rv.p2[rv.n2 - 1]
                       : (rv.n1 > 0 ? &rv.p1[rv.n1 - 1] : nullptr));

        bool ok = (last != nullptr);
        if (ok)
        {
            float v = last->value;
            if (std::isnan(v)) v = 0.0f;                 // basic sanity
            out = juce::jlimit(0.0f, 1.0f, v);           // clamp to 0..1 for CC mapping
        }

        ring->finishRead(avail);                         // drain fully → no backlog/latency
        return ok;
    }

    void openIAC()
    {
        auto devices = juce::MidiOutput::getAvailableDevices();
        for (auto& d : devices)
        {
            if (d.name.containsIgnoreCase("IAC"))   // e.g. "IAC Driver Bus 1"
            {
                iacOut = juce::MidiOutput::openDevice(d.identifier);
                DBG("Opened IAC output: " << d.name);
                break;
            }
        }
    }

    void sendIAC(int value)
    {
        auto msg = juce::MidiMessage::controllerEvent(iacChannel, iacCc, value);
        if (iacEnabled.load() && iacOut)
            iacOut->sendMessageNow(msg);
    }



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