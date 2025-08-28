//
// Created by Sol Harter on 28/08/2025.
//

#include "dsp_worker.h"

void DSPWorker::process (const EegSample& sample)
{
    //DBG("DSP EEG: " << sample.value << " ts=" << sample.stamp);

    //DSP loop goes here

    const EegSample y { sample.value, sample.stamp };

    //Write out samples to the inlet
    auto w = dest.beginWrite(1);
    if (w.n1 > 0) { w.p1[0] = y; dest.finishWrite(1); }
    else if (w.n2 > 0) { w.p2[0] = y; dest.finishWrite(1); }
    else {
        // Dest Full, Drop one
    }
}

void DSPWorker::run()
{
        while (! threadShouldExit())
        {
            //======================Read in Samples========================
            int samples_to_move = 1;
            //DBG ("EEG BUF Available " << source.available());
            if (source.available() >= samples_to_move) {
                auto readView = source.beginRead (samples_to_move);

                for (int i = 0; i < readView.n1; ++i) process(readView.p1[i]);
                for (int i = 0; i < readView.n2; ++i) process(readView.p2[i]);
                source.finishRead(readView.total());
            }
            else
            {
                // No sample ready
                juce::Thread::yield();
            }
        }
}
