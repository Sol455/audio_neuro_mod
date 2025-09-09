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
    EegScopeComponent modulationScope;
    juce::Slider freqSlider, gainSlider;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> freqSliderAttachment, gainSliderAttachment;

    juce::Label freqLabel {"freqLabel", "Frequency"};
    juce::Label gainLabel {"gainLabel", "Gain"};
    juce::TextButton connectButton, streamButton;

    juce::ComboBox channelSelector;
    juce::Label channelLabel;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
