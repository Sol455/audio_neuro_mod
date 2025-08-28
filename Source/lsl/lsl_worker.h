// Created by Sol Harter on 28/08/2025.

#ifndef AUDIO_NEURO_MOD_LSL_WORKER_H
#define AUDIO_NEURO_MOD_LSL_WORKER_H

#pragma once
#include <juce_core/juce_core.h>
#include <lsl_cpp.h>
#include <atomic>
#include <vector>
#include "EegRingBuffer.h"

class LslWorker : private juce::Thread
{
public:
    explicit LslWorker (EegRingBuffer& dst)
        : juce::Thread ("LSL Worker"), ring (dst) {}

    void setInlet   (lsl::stream_inlet* p) { inlet.store (p, std::memory_order_release); }
    void setChannel (const int idx)              { channel.store (idx, std::memory_order_relaxed); }
    int  getChannel () const               { return channel.load (std::memory_order_relaxed); }

    void startWorker() { if (! isThreadRunning()) startThread (Priority::high); }
    void stopWorker()  { signalThreadShouldExit(); stopThread (1500); }

private:
    void run() override
    {
        std::vector<float> oneSample; // resize once we know channel count
        int chCount = 0;

        while (! threadShouldExit())
        {
            auto* in = inlet.load (std::memory_order_acquire);
            if (! in) { juce::Thread::yield(); continue; }

            // Discover channel count lazily
            if (chCount <= 0)
            {
                try { chCount = in->info().channel_count(); }
                catch (...) { juce::Thread::sleep (5); continue; }

                if (chCount <= 0) { juce::Thread::sleep (5); continue; }
                oneSample.resize ((size_t) chCount);
            }

            const int wantCh = juce::jlimit (0, chCount - 1, channel.load (std::memory_order_relaxed));

            double ts = 0.0; // non blocking
            try {
                ts = in->pull_sample (oneSample.data(), chCount, /*timeout*/ 0.0);
            } catch (...) {
                juce::Thread::yield();
                continue;
            }

            if (ts != 0.0)  //sample got
            {
                auto w = ring.beginWrite (1);
                if (w.n1 > 0) {
                    w.p1[0].value = oneSample[(size_t) wantCh];
                    w.p1[0].stamp = ts;
                    ring.finishWrite (1);
                }
                else if (w.n2 > 0) {
                    w.p2[0].value = oneSample[(size_t) wantCh];
                    w.p2[0].stamp = ts;
                    ring.finishWrite (1);
                }
                else {
                    // Ring full; drop one sample
                    juce::Thread::yield();
                }
            }
            else
            {
                // No sample ready
                juce::Thread::yield();
            }
        }
    }

    EegRingBuffer&                  ring;
    std::atomic<lsl::stream_inlet*> inlet   { nullptr }; // non-owning
    std::atomic<int>                channel { 0 };
};

#endif // AUDIO_NEURO_MOD_LSL_WORKER_H

