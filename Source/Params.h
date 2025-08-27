//
// Created by Sol Harter on 27/08/2025.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace Params
{
    namespace IDs
    {

        inline const juce::ParameterID Freq { "FREQ", 1 };
        inline const juce::ParameterID Gain { "GAIN", 1 };

    }

    // Helpers
    inline std::atomic<float>* raw (juce::AudioProcessorValueTreeState& apvts,
                                    const juce::ParameterID& id)
    {
        auto* p = apvts.getRawParameterValue (id.getParamID());
        jassert (p != nullptr);
        return p;
    }

    inline juce::RangedAudioParameter* param (juce::AudioProcessorValueTreeState& apvts,
                                              const juce::ParameterID& id)
    {
        auto* p = apvts.getParameter (id.getParamID());
        jassert (p != nullptr);
        return p;
    }

    template <typename ParamT>
    inline ParamT* as (juce::RangedAudioParameter* p)
    {
        auto* casted = dynamic_cast<ParamT*> (p);
        jassert (casted != nullptr);
        return casted;
    }

    struct Cache
    {
        std::atomic<float>* freq = nullptr;
        std::atomic<float>* gain = nullptr;

        juce::AudioParameterFloat* freqParam = nullptr;
        juce::AudioParameterFloat* gainParam = nullptr;

        void init (juce::AudioProcessorValueTreeState& apvts)
        {
            freq = raw (apvts, IDs::Freq);
            gain = raw (apvts, IDs::Gain);

            freqParam = as<juce::AudioParameterFloat> (param (apvts, IDs::Freq));
            gainParam = as<juce::AudioParameterFloat> (param (apvts, IDs::Gain));
        }
    };
}