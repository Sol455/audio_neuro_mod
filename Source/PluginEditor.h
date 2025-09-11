#pragma once

#include "PluginProcessor.h"
#include "ui/EegScopeComponent.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, public juce::ComboBox::Listener, public juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void updateConnectButtonState();
    void updateChannelSelector();
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

    EegScopeComponent rawEegScope;
    EegScopeComponent phaseScope;
    EegScopeComponent modulationScope;

    juce::Slider freqSlider, gainSlider, phaseSlider;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> freqSliderAttachment, gainSliderAttachment, phaseSliderAttachment;

    juce::Label freqLabel {"freqLabel", "Frequency"};
    juce::Label gainLabel {"gainLabel", "Gain"};
    juce::Label phaseLabel {"phaseLabel", "Phase Offset"};

    juce::TextButton connectButton, streamButton;

    juce::ComboBox channelSelector;
    juce::Label channelLabel;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
