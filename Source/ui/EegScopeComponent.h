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

        // Read rate:
        const int targetFps = 30;
        startTimerHz(targetFps);
    }

    void setDualSource(EegFIFO* primary, EegFIFO* secondary, double fsHz, bool usePrimary = true)
    {
        ring = primary;
        secondaryRing = secondary;
        fs = fsHz;
        showPrimary = usePrimary;

        // Fixed window (same logic as setSource)
        windowSeconds = 2.0f;
        const int bufferSize = (int)(fs * windowSeconds);
        circularBuffer.resize(bufferSize, 0.0f);
        writeIndex = 0;

        const int targetFps = 30;
        startTimerHz(targetFps);
    }

    void setActiveSource(bool usePrimary)
    {
        showPrimary = usePrimary;
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

        // Always drain primary FIFO
        drainFIFO(ring, showPrimary);

        // drain secondary FIFO if it exists
        if (secondaryRing != nullptr) {
            drainFIFO(secondaryRing, !showPrimary);
        }
    }

    void drainFIFO(EegFIFO* fifo, bool processData)
    {
        const int available = fifo->available();
        if (available == 0)
            return;

        auto rv = fifo->beginRead(available);
        const int n = rv.total();

        if (n > 0)
        {
            if (processData)
            {
                const int bufferSize = (int)circularBuffer.size();

                for (int i = 0; i < rv.n1; ++i)
                {
                    circularBuffer[writeIndex] = rv.p1[i].value;
                    writeIndex = (writeIndex + 1) % bufferSize;
                }

                for (int i = 0; i < rv.n2; ++i)
                {
                    circularBuffer[writeIndex] = rv.p2[i].value;
                    writeIndex = (writeIndex + 1) % bufferSize;
                }
            }

            fifo->finishRead(n);  // Always consume

            if (processData)
            {
                repaint();
            }
        }
    }

    EegFIFO* ring = nullptr;
    EegFIFO* secondaryRing = nullptr;
    bool showPrimary = true;
    double fs = 0.0;
    float windowSeconds = 2.0f;
    bool autoscale = true;
    float thickness = 1.5f;
    juce::Colour traceColour = juce::Colours::lime;

    std::vector<float> circularBuffer;
    int writeIndex = 0;
};

#endif //AUDIO_NEURO_MOD_EEGSCOPECOMPONENT_H