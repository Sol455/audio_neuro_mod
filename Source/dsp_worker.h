//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_DSP_WORKER_H
#define AUDIO_NEURO_MOD_DSP_WORKER_H

#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <vector>
#include "lsl/EegRingBuffer.h"

class DSPWorker : private juce::Thread
{
public:
    explicit DSPWorker (EegRingBuffer& src, EegRingBuffer& dst)
        : juce::Thread ("DSP Worker"), source (src), dest (dst) {}

    void startWorker() { if (! isThreadRunning()) startThread (Priority::high); }
    void stopWorker()  { signalThreadShouldExit(); stopThread (1500); }

private:
    void process(const EegSample& sample);

    void run() override;

    EegRingBuffer &source;
    EegRingBuffer &dest;
};


#endif //AUDIO_NEURO_MOD_DSP_H