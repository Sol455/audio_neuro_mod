//
// Created by Sol Harter on 28/08/2025.
//

#include "dsp_worker.h"

#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void DSPWorker::prepare(double eegFs, double centreHz, double Q, std::atomic<float>* modDepth, std::atomic<float>* minModDepth) {
    filtermod.prepare(eegFs, centreHz, Q);
    filtermod.setParameterReferences(modDepth, minModDepth);
}

void DSPWorker::process (const EegSample& sample_in)
{

    percentile.addSample(sample_in.value);

    //COMPLEX processing
    std::complex<float> analytic = filtermod.filterComplex(sample_in.value);
    float envelope = std::abs(analytic);
    float phase = std::arg(analytic);

    float phaseOffsetDegrees = paramsCache.phaseOffset->load();
    float phaseOffsetRadians = phaseOffsetDegrees * (M_PI / 180.0f);

    const float mod_signal = filtermod.makeModSignalComplex(envelope, phase, phaseOffsetRadians, percentile);


    const EegSample mod_out { mod_signal, sample_in.stamp };
    const EegSample phase_out { phase, sample_in.stamp };


    //Write to output ring Buffer
    destRING.addSample(mod_out);

    //Write out samples to the UI OUTLETs
    if (!uiDestRawFIFO.addSample(sample_in)) {
        DBG("UI Raw FIFO full");

        // Dest Full, Drop one
    }

    if (!uiDestPhaseFIFO.addSample(phase_out)) {
        DBG("UI Phase FIFO full");

        // Dest Full, Drop one
    }

    if (!uiDestModFIFO.addSample(mod_out)) {
        DBG("UI Mod FIFO full");

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

void DSPWorker::run_and_profile()
{
    SimpleProfiler processProfiler;
    auto lastPrintTime = std::chrono::high_resolution_clock::now();

    while (!threadShouldExit())
    {
        // Print stats every 100ms
        auto now = std::chrono::high_resolution_clock::now();
        auto timeSinceLastPrint = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPrintTime);

        if (timeSinceLastPrint.count() >= 100) {
            auto now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::cout << "Time: " << std::put_time(std::localtime(&now_c), "%H:%M:%S")
                      << " | FIFO: " << sourceFIFO.available() << "/" << sourceFIFO.capacity()
                      << " (" << (100.0f * sourceFIFO.available() / sourceFIFO.capacity()) << "%)"
                      << " | Process avg: " << std::fixed << std::setprecision(3) << processProfiler.getAverage() << "ms"
                      << std::endl;

            processProfiler.reset();
            lastPrintTime = now;
        }

        EegSample sample;
        if (sourceFIFO.readSample(sample)) {

            processProfiler.start();
            process(sample);
            processProfiler.stop();

            // Warn on slow calls
            if (processProfiler.getTime() > 10.0) {
                std::cout << "WARNING: Slow process() call took " << processProfiler.getTime() << "ms" << std::endl;
            }
        }
        else
        {
            // No sample ready
            juce::Thread::yield();
        }
    }
}