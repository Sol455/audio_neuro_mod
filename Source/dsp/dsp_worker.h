//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_DSP_WORKER_H
#define AUDIO_NEURO_MOD_DSP_WORKER_H

#include <juce_core/juce_core.h>
#include <atomic>
#include <vector>
#include "../lsl/EegFIFO.h"
#include "filterMod.h"
#include "../lsl/EegRingBuf.h"
#include "RunningPercentile.h"
#include "../Params.h"
#include "simpleProfiler.h"

class DSPWorker : private juce::Thread
{
public:
    explicit DSPWorker (EegFIFO& src, EegRingBuf& dst, EegFIFO& ui, EegFIFO& uimod, EegFIFO& uiphase, Params::Cache& pCache
        )
        : juce::Thread ("DSP Worker"), sourceFIFO (src), destRING (dst), uiDestRawFIFO (ui), uiDestModFIFO (uimod), uiDestPhaseFIFO(uiphase), paramsCache(pCache){}
    void prepare(double eegFs, double centreHz, double Q, std::atomic<float>* modDepth, std::atomic<float>* minModDepth);

    void startWorker() { if (! isThreadRunning()) startThread (Priority::high); }
    void stopWorker()  {percentile.clear(); signalThreadShouldExit(); stopThread (1500); }

private:
    void process(const EegSample& sample);

    void run() override;

    void run_and_profile();

    RunningPercentile percentile;

    EegFIFO &sourceFIFO;

    Params::Cache &paramsCache;
    EegRingBuf &destRING;
    EegFIFO &uiDestRawFIFO;
    EegFIFO &uiDestPhaseFIFO;
    EegFIFO &uiDestModFIFO;
    filterMod filtermod;
};


#endif //AUDIO_NEURO_MOD_DSP_H