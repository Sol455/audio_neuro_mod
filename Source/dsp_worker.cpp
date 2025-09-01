//
// Created by Sol Harter on 28/08/2025.
//

#include "dsp_worker.h"

void DSPWorker::prepare(double eegFs, double centreHz, double Q) {
    filtermod.prepare(eegFs, centreHz, Q);
}

void DSPWorker::process (const EegSample& sample_in)
{
    //DBG("DSP EEG: " << sample.value << " ts=" << sample.stamp);

    //DSP Chain here:

    const float filtered_signal = filtermod.process(sample_in.value);
    const float mod_signal = madeModSignal(filtered_signal);

    const EegSample out { mod_signal, sample_in.stamp };

    //const EegSample y { sample.value, sample.stamp };

    //Write out samples to the DSP Outlet
    auto writeDSP = dest.beginWrite(1);
    if (writeDSP.n1 > 0) { writeDSP.p1[0] = out; dest.finishWrite(1); }
    else if (writeDSP.n2 > 0) { writeDSP.p2[0] = out; dest.finishWrite(1); }
    else {
        // Dest Full, Drop one
    }

    //Write out samples to the UI OUTLET
    auto writeUI = uidest.beginWrite(1);
    if (writeUI.n1 > 0) { writeUI.p1[0] = out; uidest.finishWrite(1); }
    else if (writeUI.n2 > 0) { writeUI.p2[0] = out; uidest.finishWrite(1); }
    else {
        // Dest Full, Drop one
    }
}

float DSPWorker::madeModSignal(float sample) {

    //DBG ("Sample in: " << sample);

    // --- Step 1: normalise raw sample -refP95..+refP95 → [0..1]

    float depth = 0.5f * ((sample / 7.688501426439704e-05) + 1.0f);

    // --- Step 2: scale by modulation depth
    depth *= mod_depth;

    // --- Step 3: clip into [0,1]
    depth = juce::jlimit(0.0f, 1.0f, depth);

    // --- Step 4: apply min/max modulation depth window
    float modulator_signal = mod_min_depth + (1.0 - mod_min_depth) * depth;

    // --- Done: 0..1 range with floor/ceiling
    //DBG ("Sample out: " << modulator_signal);

    return modulator_signal;

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
