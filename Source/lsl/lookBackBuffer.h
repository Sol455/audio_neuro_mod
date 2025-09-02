//
// Created by Sol Harter on 01/09/2025.
//

#ifndef AUDIO_NEURO_MOD_LOOKBACKBUFFER_H
#define AUDIO_NEURO_MOD_LOOKBACKBUFFER_H

//
// LookbackBuffer.h - Ring buffer with sample-accurate lookups
//

#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <atomic>
#include "eegTypes.h"

class LookbackBuffer {
public:
    explicit LookbackBuffer(int capacitySamples)
        : buffer(capacitySamples), capacity(capacitySamples) {}

    void addSample(const EegSample& sample) {
        int writeIndex = writePos.load() % capacity;
        buffer[writeIndex] = sample;
        writePos.fetch_add(1);

        // Track the range of valid sample indices
        if (validSamples.load() < capacity) {
            validSamples.fetch_add(1);
        }
    }

    // Get interpolated value
    bool getValueAtSample(int64_t targetSampleIndex, float& outValue) {
        int numValid = validSamples.load();
        if (numValid == 0) return false;

        const EegSample* before = nullptr;
        const EegSample* after = nullptr;

        int startIdx = juce::jmax(0, writePos.load() - numValid);
        int endIdx = writePos.load();

        // Search through valid samples in the ring
        for (int i = startIdx; i < endIdx; ++i) {
            const auto& sample = buffer[i % capacity];

            if (sample.stamp <= targetSampleIndex) {
                before = &sample;
            }
            if (sample.stamp >= targetSampleIndex && !after) {
                after = &sample;
            }
        }

        if (before && after && before != after) {
            double t = static_cast<double>(targetSampleIndex - before->stamp) /
                      static_cast<double>(after->stamp - before->stamp);
            outValue = before->value + static_cast<float>(t * (after->value - before->value));
            return true;
        }
        else if (before) {
            // Only have data before target - use most recent
            outValue = before->value;
            return true;
        }
        else if (after) {
            // Only have data after target - use earliest
            outValue = after->value;
            return true;
        }

        return false; // No valid data found
    }

    // Check if we have data in a reasonable range of the target
    bool hasDataNear(int64_t targetSampleIndex, int64_t maxDistance = 10000) {
        int numValid = validSamples.load();
        if (numValid == 0) return false;

        int startIdx = juce::jmax(0, writePos.load() - numValid);
        int endIdx = writePos.load();

        for (int i = startIdx; i < endIdx; ++i) {
            const auto& sample = buffer[i % capacity];
            if (std::abs(sample.stamp - targetSampleIndex) <= maxDistance) {
                return true;
            }
        }
        return false;
    }

private:
    std::vector<EegSample> buffer;
    int capacity;
    std::atomic<int> writePos{0};
    std::atomic<int> validSamples{0};
};

#endif //AUDIO_NEURO_MOD_LOOKBACKBUFFER_H
