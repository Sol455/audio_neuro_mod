//
// Created by Sol Harter on 10/09/2025.
//

#include "OutputSyncLayer.h"

void OutputSyncLayer::prepare(double fs)
{
    sampleRate = fs;
}

void OutputSyncLayer::setDelayMs(float delayMs)
{
    if (sampleRate <= 0.0) {
        jassertfalse; // prepare() must be called first
        return;
    }

    lookbackDelaySamples = static_cast<int64_t>(delayMs * sampleRate / 1000.0);

    lookbackDelaySamples = juce::jmax(lookbackDelaySamples, (int64_t)1); //ensure min delay
    DBG("OutputSyncLayer: Set delay to " << delayMs << "ms (" << lookbackDelaySamples << " samples)");
}

void OutputSyncLayer::attachRing(EegRingBuf* ringBuffer)
{
    ring = ringBuffer;
    DBG("OutputSyncLayer: Attached ring buffer");
}

float OutputSyncLayer::getEegValueAtTime(int64_t globalSample)
{
    if (!isReady()) {
        return 0.0f;
    }

    // Calculate lookback sample with fixed delay
    auto lookbackSample = globalSample - lookbackDelaySamples;

    // Get EEG value from ring buffer (with interpolation)
    float eegValue = 0.0f;
    bool success = ring->getValueAtSample(lookbackSample, eegValue);

    if (!success) {
        return 0.0f;
    }
    return eegValue;
}