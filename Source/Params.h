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
        inline const juce::ParameterID PhaseOffset { "PHASE_OFFSET", 1 };
        inline const juce::ParameterID ProcessingMode { "PROCESSING_MODE", 1 };
        inline const juce::ParameterID ModDepth { "MOD_DEPTH", 1 };
        inline const juce::ParameterID ModMinDepth { "MOD_MIN_DEPTH", 1 };
        inline const juce::ParameterID ModFreq { "MOD_FREQ", 1 };
        inline const juce::ParameterID ModMode { "MOD_MODE", 1 };
        inline const juce::ParameterID EnvMix { "ENV_MIX", 1 };
        inline const juce::ParameterID WaveformType { "WAVE_TYPE", 1 };


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
        std::atomic<float>* phaseOffset = nullptr;
        std::atomic<float>* processingMode = nullptr;
        std::atomic<float>* modDepth = nullptr;
        std::atomic<float>* modMinDepth = nullptr;
        std::atomic<float>* modFreq = nullptr;
        std::atomic<float>* modMode = nullptr;
        std::atomic<float>* envMix = nullptr;
        std::atomic<float>* waveformType = nullptr;

        juce::AudioParameterFloat* freqParam = nullptr;
        juce::AudioParameterFloat* gainParam = nullptr;
        juce::AudioParameterFloat* phaseOffsetParam = nullptr;

        juce::AudioParameterChoice* processingModeParam = nullptr;
        juce::AudioParameterFloat* modDepthParam = nullptr;
        juce::AudioParameterFloat* modMinDepthParam = nullptr;
        juce::AudioParameterFloat* modFreqParam = nullptr;
        juce::AudioParameterChoice* modModeParam = nullptr;
        juce::AudioParameterFloat* envMixParam = nullptr;
        juce::AudioParameterChoice* waveformTypeParam = nullptr;

        void init (juce::AudioProcessorValueTreeState& apvts)
        {
            freq = raw (apvts, IDs::Freq);
            gain = raw (apvts, IDs::Gain);
            phaseOffset = raw (apvts, IDs::PhaseOffset);
            processingMode = raw (apvts, IDs::ProcessingMode);
            modDepth = raw (apvts, IDs::ModDepth);
            modMinDepth = raw (apvts, IDs::ModMinDepth);
            modFreq = raw (apvts, IDs::ModFreq);
            modMode = raw (apvts, IDs::ModMode);
            envMix = raw (apvts, IDs::EnvMix);
            waveformType = raw (apvts, IDs::WaveformType);

            freqParam = as<juce::AudioParameterFloat> (param (apvts, IDs::Freq));
            gainParam = as<juce::AudioParameterFloat> (param (apvts, IDs::Gain));
            phaseOffsetParam = as<juce::AudioParameterFloat> (param (apvts, IDs::PhaseOffset));
            processingModeParam = as<juce::AudioParameterChoice> (param (apvts, IDs::ProcessingMode));
            modDepthParam = as<juce::AudioParameterFloat> (param (apvts, IDs::ModDepth));
            modMinDepthParam = as<juce::AudioParameterFloat> (param (apvts, IDs::ModMinDepth));
            modFreqParam =  as<juce::AudioParameterFloat> (param (apvts, IDs::ModFreq));
            modModeParam = as<juce::AudioParameterChoice> (param (apvts, IDs::ModMode));
            envMixParam = as<juce::AudioParameterFloat> (param (apvts, IDs::EnvMix));
            waveformTypeParam = as<juce::AudioParameterChoice> (param (apvts, IDs::WaveformType));

        }
    };
}