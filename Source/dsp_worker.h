//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_DSP_WORKER_H
#define AUDIO_NEURO_MOD_DSP_WORKER_H

#include <juce_core/juce_core.h>
#include <atomic>
#include <vector>
#include "lsl/EegFIFO.h"
#include "filterMod.h"
#include "lsl/EegRingBuf.h"

class DSPWorker : private juce::Thread
{
public:
    explicit DSPWorker (EegFIFO& src, EegRingBuf& dst, EegFIFO& ui)
        : juce::Thread ("DSP Worker"), sourceFIFO (src), destRING (dst), uiDestFIFO (ui){}
    void prepare(double eegFs, double centreHz = 10.0, double Q = 2.0);

    void startWorker() { if (! isThreadRunning()) startThread (Priority::high); }
    void stopWorker()  { signalThreadShouldExit(); stopThread (1500); }

private:
    void process(const EegSample& sample);

    void run() override;

    float madeModSignal(float env_sample);

    double envelope_95_ref = 7.688501426439704;
    double mod_depth = 0.8f;
    double mod_min_depth = 0.15f;

    EegFIFO &sourceFIFO;
    EegRingBuf &destRING;
    EegFIFO &uiDestFIFO;
    filterMod filtermod;
};


#endif //AUDIO_NEURO_MOD_DSP_H