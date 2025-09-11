#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(phaseSlider);

    freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    phaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);

    freqSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, true, 50, 13);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, true, 50, 13);
    phaseSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, true, 50, 13);


    gainSliderAttachment  = std::make_unique<SliderAttachment>(processorRef.apvts, Params::IDs::Gain.getParamID()
, gainSlider);
    freqSliderAttachment  = std::make_unique<SliderAttachment>(processorRef.apvts, Params::IDs::Freq.getParamID()
, freqSlider);
    phaseSliderAttachment  = std::make_unique<SliderAttachment>(processorRef.apvts, Params::IDs::PhaseOffset.getParamID()
, phaseSlider);

    freqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(freqLabel);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    phaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(phaseLabel);

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

    addAndMakeVisible(streamButton);
    streamButton.setButtonText("Stream");

    streamButton.onClick = [this]() {
        processorRef.lsl_stream();
    };



    //EEG Scopes
    addAndMakeVisible (rawEegScope);
    rawEegScope.setAutoscale (true);
    rawEegScope.setSource (&processorRef.getUiRawRing(), 160.0); //@TO-DO remove hardcoded value
    rawEegScope.setTraceColour(juce::Colours::limegreen);

    addAndMakeVisible (phaseScope);
    phaseScope.setAutoscale (true);
    phaseScope.setSource (&processorRef.getUiPhaseRing(), 160.0); //@TO-DO remove hardcoded value
    phaseScope.setTraceColour(juce::Colours::firebrick);

    addAndMakeVisible (modulationScope);
    modulationScope.setAutoscale (true);
    modulationScope.setSource (&processorRef.getUiModRing(), 160.0); //@TO-DO remove hardcoded value
    modulationScope.setTraceColour(juce::Colours::coral);

    //Channel Selector
    channelLabel.setText("EEG Channel:", juce::dontSendNotification);
    addAndMakeVisible(channelLabel);

    channelSelector.addListener(this);
    channelSelector.setTextWhenNothingSelected("No channels available");
    addAndMakeVisible(channelSelector);

    // Update channel list if already connected
    updateChannelSelector();

    startTimer(1000);
    setSize (800, 500);
}

void AudioPluginAudioProcessorEditor::updateChannelSelector()
{
    channelSelector.clear();

    if (processorRef.lslConnected())
    {
        int channelCount = processorRef.getEegChannelCount();

        for (int i = 1; i <= channelCount; i++)
        {
            channelSelector.addItem("Channel " + juce::String(i), i);
        }

        if (channelCount > 0)
        {
            int currentChannel = processorRef.getCurrentEegChannel() + 1; // convert 0-based to 1-based
            if (currentChannel >= 1 && currentChannel <= channelCount)
                channelSelector.setSelectedId(currentChannel);
            else
                channelSelector.setSelectedId(1); // default to channel 1
        }

        channelSelector.setEnabled(true);
    }
    else
    {
        // No LSL connection
        channelSelector.setTextWhenNothingSelected("LSL not connected");
        channelSelector.setEnabled(false);
    }
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    static bool wasConnected = false;
    bool isConnected = processorRef.lslConnected();

    if (isConnected != wasConnected)
    {
        updateChannelSelector();
        wasConnected = isConnected;
    }
}

void AudioPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &channelSelector)
    {
        int selectedChannel = channelSelector.getSelectedId() - 1; // convert to 0-based
        if (selectedChannel >= 0)
        {
            processorRef.setEegChannel(selectedChannel);
            std::cout << "Editor: User selected channel " << (selectedChannel + 1) << std::endl;
        }
    }
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
    connectButton.setBounds(getWidth()/ 2 - 150, getHeight()/2 + 150 , 100, 50);
    streamButton.setBounds(getWidth()/ 2 - 50, getHeight()/2 + 150 , 100, 50);

    auto scope_bounds = getLocalBounds().withSizeKeepingCentre(700, 300);
    int thirdHeight = scope_bounds.getHeight() / 3;

    rawEegScope.setBounds(scope_bounds.removeFromTop(thirdHeight));
    phaseScope.setBounds(scope_bounds.removeFromTop(thirdHeight));
    modulationScope.setBounds(scope_bounds);

    channelLabel.setBounds(10, 10, 100, 25);
    channelSelector.setBounds(120, 10, 150, 25);

    // Position sliders relative to actual scope bounds
    int scopeBottom = scope_bounds.getBottom(); // Use actual scope bounds

    freqLabel.setBounds(getWidth() - 180, scopeBottom + 10, 160, 20);
    freqSlider.setBounds(getWidth() - 180, scopeBottom + 35, 160, 25);

    gainLabel.setBounds(getWidth() - 180, scopeBottom + 70, 160, 20);
    gainSlider.setBounds(getWidth() - 180, scopeBottom + 50, 160, 25);

    phaseLabel.setBounds(getWidth() - 180, scopeBottom - 370, 160, 20);
    phaseSlider.setBounds(getWidth() - 180, scopeBottom - 350, 160, 25);
}

