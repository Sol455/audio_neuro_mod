//
// Created by Sol Harter on 28/08/2025.
//

#ifndef AUDIO_NEURO_MOD_EEGSCOPECOMPONENT_H
#define AUDIO_NEURO_MOD_EEGSCOPECOMPONENT_H

// EegScopeComponent.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../lsl/EegFIFO.h"

class EegScopeComponent : public juce::Component,
                          private juce::Timer
{
public:
    void setSource (EegFIFO* rb, double fsHz)
    {
        ring = rb;
        fs   = fsHz;

        // Fixed window
        windowSeconds = 2.0f;

        const int bufferSize = (int)(fs * windowSeconds);
        circularBuffer.resize(bufferSize, 0.0f);
        writeIndex = 0;

        // Read rate: ~ produced per frame
        const int targetFps = 30;
        startTimerHz(targetFps);
        samplesPerFrame = juce::jmax(1, (int)std::lround(fs / (double)targetFps));
    }

    void setAutoscale (bool enabled) { autoscale = enabled; }
    void setTraceThickness (float px) { thickness = juce::jlimit (0.5f, 4.0f, px); }
    void setTraceColour (const juce::Colour& newColour) { traceColour = newColour; }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        if (circularBuffer.empty() || fs <= 0.0)
            return;

        const auto w = (float) getWidth();
        const auto h = (float) getHeight();
        const float yMid = h * 0.5f;
        const int bufferSize = (int)circularBuffer.size();

        float vmax = 1.0f;
        if (autoscale)
        {
            vmax = 1e-6f;
            for (float v : circularBuffer)
                vmax = std::max(vmax, std::abs(v));
        }
        const float kY = (vmax > 0.0f ? (yMid - 2.0f) / vmax : 1.0f);

        juce::Path tracePath;
        tracePath.preallocateSpace(bufferSize * 3);

        for (int i = 0; i < bufferSize; ++i)
        {
            const float value = circularBuffer[i];

            const float x = w * (float)i / (float)(bufferSize - 1);
            const float y = yMid - kY * value;

            if (i == 0) {
                tracePath.startNewSubPath(x, y);
            } else {
                tracePath.lineTo(x, y);
            }
        }

        g.setColour(traceColour);
        g.strokePath(tracePath, juce::PathStrokeType(thickness));
    }

    void resized() override {}

private:
    void timerCallback() override
    {
        if (ring == nullptr || circularBuffer.empty())
            return;

        // Read new samples from ring
        auto rv = ring->beginRead(samplesPerFrame);
        const int n = rv.total();

        if (n > 0)
        {
            int samplesWritten = 0;
            const int bufferSize = (int)circularBuffer.size();

            for (int i = 0; i < rv.n1 && samplesWritten < n; ++i)
            {
                circularBuffer[writeIndex] = rv.p1[i].value;
                writeIndex = (writeIndex + 1) % bufferSize;
                ++samplesWritten;
            }

            for (int i = 0; i < rv.n2 && samplesWritten < n; ++i)
            {
                circularBuffer[writeIndex] = rv.p2[i].value;
                writeIndex = (writeIndex + 1) % bufferSize;
                ++samplesWritten;
            }

            ring->finishRead(n);
            repaint();
        }
    }

    EegFIFO* ring = nullptr;
    double fs = 0.0;
    float windowSeconds = 2.0f;
    int samplesPerFrame = 8;
    bool autoscale = true;
    float thickness = 1.5f;
    juce::Colour traceColour = juce::Colours::lime;

    std::vector<float> circularBuffer;
    int writeIndex = 0;
};

#endif //AUDIO_NEURO_MOD_EEGSCOPECOMPONENT_H