//
// Created by Sol Harter on 28/08/2025.
//

#include "dsp_worker.h"

void DSPWorker::prepare(double eegFs, double centreHz, double Q) {
    filtermod.prepare(eegFs, centreHz, Q);
}

void DSPWorker::process (const EegSample& sample_in)
{
    //DSP Chain here:

    const float filtered_signal = filtermod.process(sample_in.value);
    const float mod_signal = madeModSignal(filtered_signal);
    const EegSample out { mod_signal, sample_in.stamp };

    //Write to output ring Buffer
    destRING.addSample(out);

    //Write out samples to the UI OUTLET
    if (!uiDestFIFO.addSample(out)) {
        // Dest Full, Drop one
    }
}

float DSPWorker::madeModSignal(float sample) {

    //DBG ("Sample in: " << sample);

    //1: normalise raw sample

    float depth = 0.5f * ((sample / 7.688501426439704e-05) + 1.0f);

    //2: scale by modulation depth
    depth *= mod_depth;

    //3: clip into [0,1]
    depth = juce::jlimit(0.0f, 1.0f, depth);

    //4: apply min/max modulation depth window
    float modulator_signal = mod_min_depth + (1.0 - mod_min_depth) * depth;

    return modulator_signal;

}

void DSPWorker::run()
{
        while (! threadShouldExit())
        {
            EegSample sample;
            if (sourceFIFO.readSample(sample)) {
                process(sample);
            }
            else
            {
                // No sample ready
                juce::Thread::yield();
            }
        }
}
