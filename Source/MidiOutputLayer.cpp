//
// Created by Sol Harter on 01/09/2025.
//
#include "MidiOutputLayer.h"

//
// Created by Sol Harter on 29/08/2025.
//


void MidiOutputLayer::prepare (double fs)
    {
        if (iacEnabled) openIAC();
        sampleRate = fs;
        samplesPerCc = juce::jmax(1, (int) std::lround(sampleRate / rateHz));
        nextCcSample = 0;
        held = 0.0f;
    }

void MidiOutputLayer::process (int numSamples, juce::MidiBuffer& midi, int64_t blockStartSample)
    {
        if (ring != nullptr)
        {
            float latest = held;
            if (tryReadLatestFromRing(latest))
                held = latest;
        }

        for (int sampleInBlock = 0; sampleInBlock < numSamples; ++sampleInBlock)
        {
            auto currentGlobalSample = blockStartSample + sampleInBlock;

            if (currentGlobalSample >= nextCcSample)
            {
                float eegValue;
                auto lookbackSample = currentGlobalSample - lookbackDelaySamples;
                //DBG ("Latest: " << held);
                //getEegValueAtSample(lookbackSample, eegValue)

                auto midiValue = scaleEegToMidi(held);
                auto midiMsg = juce::MidiMessage::controllerEvent(chan, cc, midiValue);
                midi.addEvent(midiMsg, sampleInBlock);
                //if (iacEnabled) sendIAC(v);
                nextCcSample += samplesPerCc;
            }
        }
    }

int MidiOutputLayer::scaleEegToMidi(float eegValue) const
{
    const int midi_sample = static_cast<int>(std::lround(juce::jlimit(0.0f, 1.0f, eegValue) * 127.0f));
    return midi_sample;
}

bool MidiOutputLayer::tryReadLatestFromRing (float& out) noexcept
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
            if (std::isnan(v)) v = 0.0f;
            out = juce::jlimit(0.0f, 1.0f, v);           // clamp to 0..1 for CC mapping
        }

        ring->finishRead(avail);

        return ok;
    }

void MidiOutputLayer::openIAC()
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

void MidiOutputLayer::sendIAC(int value)
    {
        auto msg = juce::MidiMessage::controllerEvent(iacChannel, iacCc, value);
        if (iacEnabled.load() && iacOut)
            iacOut->sendMessageNow(msg);
    }

