#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);

    freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);

    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);

    gainSliderAttachment  = std::make_unique<SliderAttachment>(processorRef.apvts, Params::IDs::Gain.getParamID()
, gainSlider);
    freqSliderAttachment  = std::make_unique<SliderAttachment>(processorRef.apvts, Params::IDs::Freq.getParamID()
, freqSlider);

    addAndMakeVisible(connectButton);
    connectButton.setButtonText("Connect");
    connectButton.setClickingTogglesState(false);
    connectButton.onClick = [this]() {
        if (processorRef.lslConnected())
            processorRef.disconnectLsl();
        else
            processorRef.connectLsl();

        updateConnectButtonState();
    };

    addAndMakeVisible(connectButton);

    freqLabel.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::white);
    freqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(freqLabel);
    gainLabel.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::white);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);

    setSize (800, 500);
}

void AudioPluginAudioProcessorEditor::updateConnectButtonState()
{
    if (processorRef.lslConnected()) {
        connectButton.setButtonText("Disconnect");
        connectButton.setToggleState(true, juce::dontSendNotification);
    } else {
        connectButton.setButtonText("Connect");
        connectButton.setToggleState(false, juce::dontSendNotification);
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello Neuro!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    freqLabel.setBounds(getWidth()/ 2 - 50, getHeight()/2 - 120 , 100, 20);
    freqSlider.setBounds(getWidth()/ 2 - 50, getHeight()/2 - 100 , 100, 200);
    gainSlider.setBounds(getWidth()/ 2 - 150, getHeight()/2 - 100 , 100, 200);
    gainLabel.setBounds(getWidth()/ 2 - 150, getHeight()/2 - 120 , 100, 20);
    connectButton.setBounds(getWidth()/ 2 - 50, getHeight()/2 + 120 , 100, 50);

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

