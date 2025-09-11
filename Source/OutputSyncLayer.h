//
// Created by Sol Harter on 10/09/2025.
//

#ifndef AUDIO_NEURO_MOD_OUTPUTSYNCLAYER_H
#define AUDIO_NEURO_MOD_OUTPUTSYNCLAYER_H

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "lsl/EegRingBuf.h"

/*
 * Adds a fixed delay to compensate for Jitter in LSL inlet
 */

class OutputSyncLayer
{
public:
    OutputSyncLayer() = default;
    ~OutputSyncLayer() = default;

    // Setup and configuration
    void prepare(double sampleRate);
    void setDelayMs(float delayMs);
    void attachRing(EegRingBuf* ringBuffer);

    // Core timing method - gets EEG value with fixed lookback delay
    float getEegValueAtTime(int64_t globalSample);

    // Getters for debugging/info
    int64_t getLookbackDelaySamples() const { return lookbackDelaySamples; }
    float getLookbackDelayMs() const { return (float)lookbackDelaySamples * 1000.0f / (float)sampleRate; }
    bool isReady() const { return ring != nullptr && sampleRate > 0.0; }

private:
    EegRingBuf* ring = nullptr;
    double sampleRate = 44100.0;
    int64_t lookbackDelaySamples = 441; // Default 10ms at 44.1kHz
};

#endif // AUDIO_NEURO_MOD_OUTPUTSYNCLAYER_H
