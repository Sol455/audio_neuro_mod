#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/Carrier.h"
#include "Params.h"
#include "lsl/lsl_connector.h"
#include "lsl/lsl_worker.h"
#include "lsl/EegFIFO.h"
#include "lsl/EegRingBuf.h"
#include "lsl/timestampMapper.h"
#include "dsp/dsp_worker.h"
#include "MidiOutputLayer.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor,  private juce::Timer
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
    bool lslConnected() const { return lsl_connector.isConnected(); }
    int getEegChannelCount() const { return lsl_connector.getChannelCount(); }
    void setEegChannel(int channel) { lslWorker.setChannel(channel); }
    int getCurrentEegChannel() const { return lslWorker.getChannel(); }
    void lsl_stream();
    void disconnectLsl();

    //
    EegFIFO& getUiRing() { return uiOutletFIFO; }

private:
    std::atomic<int64_t> globalSampleCounter{0};
    LslConnector lsl_connector;

    //EEG Buffers
    timestampMapper stampMapper;
    EegFIFO eegInletFIFO{ 1 << 14 };
    EegRingBuf dspRingBuffer { 1 << 14 };
    EegFIFO uiOutletFIFO { 1 << 14 };

    //Worker Threads
    LslWorker lslWorker { eegInletFIFO, stampMapper};
    DSPWorker dspWorker { eegInletFIFO , dspRingBuffer, uiOutletFIFO};

    //timestamp drift timer
    void timerCallback() override;

    MidiOutputLayer midiOut;
    Carrier carrier;
    Params::Cache paramsCache;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};

