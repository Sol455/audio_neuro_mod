#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Carrier.h"
#include "Params.h"
#include "lsl/lsl_connector.h"
#include "lsl/lsl_worker.h"
#include "lsl/eegRingBuffer.h"
#include "dsp_worker.h"


//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    bool connectLsl()   { return lsl_connector.connectFirstEEG(); }
    void disconnectLsl(){ lsl_connector.disconnect(); }
    bool lslConnected() const { return lsl_connector.isConnected(); }
    void lsl_stream() {lslWorker.setInlet (lsl_connector.inlet()); lslWorker.setChannel(55); lslWorker.startWorker(); dspWorker.startWorker();}

private:
    LslConnector lsl_connector;
    EegRingBuffer eegInletRing { 1 << 14 };
    EegRingBuffer dspOutletRing { 1 << 14 };
    LslWorker lslWorker { eegInletRing };
    DSPWorker dspWorker { eegInletRing , dspOutletRing};
    Carrier carrier;
    Params::Cache paramsCache;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};

