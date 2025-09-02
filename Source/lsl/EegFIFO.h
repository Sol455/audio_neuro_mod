//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_EEGFIFO_H
#define AUDIO_NEURO_MOD_EEGFIFO_H

#include <juce_core/juce_core.h>
#include <vector>
#include "eegTypes.h"

class EegFIFO
{
public:
    explicit EegFIFO (int capacitySamples): fifo (capacitySamples), buffer ((size_t) capacitySamples) {}

    int capacity()  const noexcept { return (int) buffer.size(); }
    int available() const noexcept { return fifo.getNumReady(); }

    bool addSample(const EegSample& sample) {
        auto w = beginWrite(1);
        if (w.total() >= 1) {
            *w.p1 = sample;
            finishWrite(1);
            return true;
        }
        return false; // Buffer full
    }

    bool readSample(EegSample& sample) {
        auto r = beginRead(1);
        if (r.total() >= 1) {
            sample = *r.p1;
            finishRead(1);
            return true;
        }
        return false; // No data available
    }

    struct WriteView { EegSample* p1{}; int n1{}; EegSample* p2{}; int n2{}; int total() const { return n1 + n2; } };

    WriteView beginWrite (int n)
    {
        WriteView v{};
        int s1, s2;
        fifo.prepareToWrite (n, s1, v.n1, s2, v.n2);
        v.p1 = v.n1 ? &buffer[(size_t) s1] : nullptr;
        v.p2 = v.n2 ? &buffer[(size_t) s2] : nullptr;
        return v;
    }

    void finishWrite (int wrote) { fifo.finishedWrite (wrote); }

    struct ReadView  { const EegSample* p1{}; int n1{}; const EegSample* p2{}; int n2{}; int total() const { return n1 + n2; } };

    ReadView beginRead (int maxToRead)
    {
        ReadView v{};
        const int toRead = juce::jmin (maxToRead, fifo.getNumReady());
        int s1, s2;
        fifo.prepareToRead (toRead, s1, v.n1, s2, v.n2);
        v.p1 = v.n1 ? &buffer[(size_t) s1] : nullptr;
        v.p2 = v.n2 ? &buffer[(size_t) s2] : nullptr;
        return v;
    }

    void finishRead (int consumed) { fifo.finishedRead (consumed); }

    // template <typename Fn>
    // int consume (int maxToRead, Fn&& fn)
    // {
    //     auto rv = beginRead (maxToRead);
    //     for (int i = 0; i < rv.n1; ++i) fn (rv.p1[i]);
    //     for (int i = 0; i < rv.n2; ++i) fn (rv.p2[i]);
    //     finishRead (rv.total());
    //     return rv.total();
    // }

private:
    juce::AbstractFifo     fifo;
    std::vector<EegSample> buffer;
};

#endif //AUDIO_NEURO_MOD_EEGRINGBUFFER_H