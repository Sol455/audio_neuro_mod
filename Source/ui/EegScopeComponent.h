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
        samplesPerFrame = juce::jlimit (32, 4096, (int) (fs * 0.05)); // ~50 ms per repaint
        startTimerHz (25); 
    }

    void setAutoscale (bool enabled) { autoscale = enabled; }
    void setTraceThickness (float px) { thickness = juce::jlimit (0.5f, 4.0f, px); }

    void setTraceColour (const juce::Colour& newColour) { traceColour = newColour; }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        if (plot.empty())
            return;

        const auto w = (float) getWidth();
        const auto h = (float) getHeight();
        const float yMid = h * 0.5f;

        // Determine vertical scale
        float vmax = 1.0f;
        if (autoscale)
        {
            vmax = 1e-6f;
            for (float v : plot) vmax = std::max (vmax, std::abs (v));
        }
        const float kY = (vmax > 0.0f ? (yMid - 2.0f) / vmax : 1.0f);

        // Build a path
        juce::Path p;
        p.preallocateSpace ((int) plot.size() * 3);
        for (size_t i = 0; i < plot.size(); ++i)
        {
            const float x = w * (float) i / (float) (std::max<size_t>(1, plot.size() - 1));
            const float y = yMid - kY * plot[i];
            if (i == 0) p.startNewSubPath (x, y);
            else        p.lineTo         (x, y);
        }

        g.setColour (traceColour);
        g.strokePath (p, juce::PathStrokeType (thickness));
    }

    void resized() override {}

private:
    void timerCallback() override
    {
        if (ring == nullptr)
            return;

        auto rv = ring->beginRead (samplesPerFrame);
        const int n = rv.total();
        temp.resize ((size_t) n);

        int k = 0;
        for (int i = 0; i < rv.n1; ++i) temp[(size_t) k++] = rv.p1[i].value;
        for (int i = 0; i < rv.n2; ++i) temp[(size_t) k++] = rv.p2[i].value;
        ring->finishRead (n);

        const int keep = juce::jlimit (samplesPerFrame, 16384, samplesPerFrame * 8);
        if ((int) plot.size() + n > keep)
            plot.erase (plot.begin(), plot.begin() + ((int) plot.size() + n - keep));
        plot.insert (plot.end(), temp.begin(), temp.end());

        repaint();
    }

    EegFIFO* ring = nullptr;
    double fs = 0.0;
    int samplesPerFrame = 256;
    bool autoscale = true;
    float thickness = 1.5f;

    std::vector<float> temp;
    std::vector<float> plot;

    juce::Colour traceColour = juce::Colours::lime;
};

#endif //AUDIO_NEURO_MOD_EEGSCOPECOMPONENT_H