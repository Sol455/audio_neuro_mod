//
// Created by Sol Harter on 28/08/2025.
//

#include "dsp_worker.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void DSPWorker::prepare(double eegFs, double centreHz, double Q) {
    filtermod.prepare(eegFs, centreHz, Q);
}

void DSPWorker::process (const EegSample& sample_in)
{
    //DSP Chain here:
    percentile.addSample(sample_in.value);

    //REAL processing
    // const float filtered_signal = filtermod.filter(sample_in.value);
    // const float mod_signal = filtermod.makeModSignalReal(filtered_signal, percentile);

    //COMPLEX processing
    std::complex<float> analytic = filtermod.filterComplex(sample_in.value);
    float envelope = std::abs(analytic);
    float phase = std::arg(analytic);

    float phaseOffsetDegrees = paramsCache.phaseOffset->load();
    float phaseOffsetRadians = phaseOffsetDegrees * (M_PI / 180.0f);

    const float mod_signal = filtermod.makeModSignalComplex(envelope, phase, phaseOffsetRadians, percentile);

    //const float mod_signal = filtermod.makeModSignalReal(sample_in.value, percentile);

    const EegSample mod_out { mod_signal, sample_in.stamp };

    const EegSample phase_out { phase, sample_in.stamp };


    //Write to output ring Buffer
    destRING.addSample(mod_out);

    //Write out samples to the UI OUTLETs
    if (!uiDestRawFIFO.addSample(sample_in)) {
        // Dest Full, Drop one
    }

    if (!uiDestPhaseFIFO.addSample(phase_out)) {
        // Dest Full, Drop one
    }

    if (!uiDestModFIFO.addSample(mod_out)) {
        // Dest Full, Drop one
    }
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
