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

    freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    phaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);

    freqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 13);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 13);
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

    //==============Phase compensation=======================

    systemDelayLabel.setText("System Delay (ms):", juce::dontSendNotification);
    addAndMakeVisible(systemDelayLabel);

    systemDelayInput.setMultiLine(false);
    systemDelayInput.setText("50.0");
    systemDelayInput.setInputRestrictions(10, "0123456789.");
    addAndMakeVisible(systemDelayInput);

    // Desired phase input
    desiredPhaseLabel.setText("Desired Phase (°):", juce::dontSendNotification);
    addAndMakeVisible(desiredPhaseLabel);

    desiredPhaseInput.setMultiLine(false);
    desiredPhaseInput.setText("0");
    desiredPhaseInput.setInputRestrictions(10, "0123456789.-");
    addAndMakeVisible(desiredPhaseInput);

    // Brain frequency input
    brainFreqLabel.setText("Brain Freq (Hz):", juce::dontSendNotification);
    addAndMakeVisible(brainFreqLabel);

    brainFreqInput.setMultiLine(false);
    brainFreqInput.setText("10.0");
    brainFreqInput.setInputRestrictions(10, "0123456789.");
    addAndMakeVisible(brainFreqInput);

    // Calculate button
    calculateButton.setButtonText("Calculate & Set");
    calculateButton.onClick = [this]() { calculatePhaseOffset(); };
    addAndMakeVisible(calculateButton);


    //EEG Scopes
    addAndMakeVisible (rawEegScope);
    rawEegScope.setAutoscale (true);
    rawEegScope.setSource (&processorRef.getUiRawRing(), 160.0); //init with default value
    rawEegScope.setTraceColour(juce::Colours::limegreen);

    addAndMakeVisible (phaseScope);
    phaseScope.setAutoscale (true);
    phaseScope.setSource (&processorRef.getUiPhaseRing(), 160.0); //init with default value
    phaseScope.setTraceColour(juce::Colours::firebrick);

    addAndMakeVisible (modulationScope);
    modulationScope.setAutoscale (true);
    modulationScope.setSource (&processorRef.getUiModRing(), 160.0); //init with default value
    modulationScope.setTraceColour(juce::Colours::coral);

    processorRef.setOnSampleRateDetected([this](float sampleRate)
    {
        rawEegScope.setSource (&processorRef.getUiRawRing(), sampleRate);
        phaseScope.setSource (&processorRef.getUiPhaseRing(), sampleRate);
        modulationScope.setSource (&processorRef.getUiModRing(), sampleRate);
        DBG("LSL Steam Source updated. FS:" << sampleRate);

    });

    //Channel Selector
    channelLabel.setText("EEG Channel:", juce::dontSendNotification);
    addAndMakeVisible(channelLabel);

    channelSelector.addListener(this);
    channelSelector.setTextWhenNothingSelected("No channels available");
    addAndMakeVisible(channelSelector);

    // Update channel list if already connected
    updateChannelSelector();

    //Processing mode selection:

    addAndMakeVisible(processingModeCombo);
    processingModeCombo.addItem("Closed Loop", 1);
    processingModeCombo.addItem("Open Loop", 2);

    processingModeLabel.setText("Mode", juce::dontSendNotification);
    addAndMakeVisible(processingModeLabel);

    processingModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processorRef.apvts,
        Params::IDs::ProcessingMode.getParamID(),
        processingModeCombo
    );

    startTimer(1000);
    setSize (800, 500);
}

void AudioPluginAudioProcessorEditor::calculatePhaseOffset()
{
    float systemDelayMs = systemDelayInput.getText().getFloatValue();
    float desiredPhaseDeg = desiredPhaseInput.getText().getFloatValue();
    float brainFreqHz = brainFreqInput.getText().getFloatValue();

    if (brainFreqHz <= 0.0f) { //No division by 0
        brainFreqHz = 10.0f;
        brainFreqInput.setText("10.0");
    }

    float delayPhaseShift = (systemDelayMs / 1000.0f) * brainFreqHz * 360.0f;

    float compensationPhase = desiredPhaseDeg - delayPhaseShift;

    while (compensationPhase > 180.0f) compensationPhase -= 360.0f;
    while (compensationPhase < -180.0f) compensationPhase += 360.0f;

    phaseSlider.setValue(compensationPhase);
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

    auto bounds = getLocalBounds();
    bounds.reduce(10, 10);

    auto topControlsArea = bounds.removeFromTop(80);
    auto scopeArea = bounds.removeFromTop(300);
    auto bottomControlsArea = bounds;

    auto topLeftControls = topControlsArea.removeFromLeft(topControlsArea.getWidth() / 2);
    auto topRightControls = topControlsArea;

    int buttonSpace = 2;

    //=============================Top Left Controls================================

    auto tlThird = topLeftControls.getHeight()/3;
    //Top left controls
    auto topLeftRow1 = topLeftControls.removeFromTop(tlThird);
    channelLabel.setBounds(topLeftRow1.removeFromLeft(100));
    topLeftRow1.removeFromLeft(buttonSpace);
    channelSelector.setBounds(topLeftRow1.removeFromLeft(150));
    topLeftRow1.removeFromLeft(buttonSpace);
    connectButton.setBounds(topLeftRow1.removeFromLeft(100));

    auto topLeftRow2 = topLeftControls.removeFromTop(tlThird);
    processingModeLabel.setBounds(topLeftRow2.removeFromLeft(100));
    topLeftRow2.removeFromLeft(buttonSpace); // Small gap
    processingModeCombo.setBounds(topLeftRow2.removeFromLeft(150));
    topLeftRow2.removeFromLeft(buttonSpace);
    streamButton.setBounds(topLeftRow2.removeFromLeft(100));

    topLeftControls.reduce(20, 5);

    //===========================Top Right controls=========================

    auto columnsplit = topRightControls.getWidth() / 2;

    auto tr_column_1 = topRightControls.removeFromRight(columnsplit);
    phaseLabel.setBounds(tr_column_1.removeFromTop(20));
    tr_column_1.removeFromTop(buttonSpace); // Small gap
    phaseSlider.setBounds(tr_column_1.removeFromTop(25));

    auto tr_column_2 = topRightControls;
    auto tr_row_helper = topRightControls.getHeight() /4;

    auto delayRow = tr_column_2.removeFromTop(tr_row_helper);
    systemDelayLabel.setBounds(delayRow.removeFromLeft(80));
    delayRow.removeFromLeft(5);
    systemDelayInput.setBounds(delayRow.removeFromLeft(60));

    auto desiredRow = tr_column_2.removeFromTop(tr_row_helper);
    desiredPhaseLabel.setBounds(desiredRow.removeFromLeft(80));
    desiredRow.removeFromLeft(5);
    desiredPhaseInput.setBounds(desiredRow.removeFromLeft(60));

    auto freqRow = tr_column_2.removeFromTop(tr_row_helper);
    brainFreqLabel.setBounds(freqRow.removeFromLeft(80));
    freqRow.removeFromLeft(5);
    brainFreqInput.setBounds(freqRow.removeFromLeft(60));

    auto calcRow = tr_column_2;
    calculateButton.setBounds(calcRow.removeFromLeft(100));

    //===========================Scope Bounds=========================
    int scopeHeight = scopeArea.getHeight() / 3;

    rawEegScope.setBounds(scopeArea.removeFromTop(scopeHeight));
    phaseScope.setBounds(scopeArea.removeFromTop(scopeHeight));
    modulationScope.setBounds(scopeArea);

    //==========================Bottom Controls=======================

    auto leftMarginArea = bottomControlsArea.removeFromLeft(20);
    auto freqControlArea = bottomControlsArea.removeFromLeft(60);
    auto spacingArea = bottomControlsArea.removeFromLeft(20);
    auto gainControlArea = bottomControlsArea.removeFromLeft(60);

    // Frequency controls
    freqLabel.setBounds(freqControlArea.removeFromTop(25));
    freqSlider.setBounds(freqControlArea);

    // Gain controls
    gainLabel.setBounds(gainControlArea.removeFromTop(25));
    gainSlider.setBounds(gainControlArea);


}

