#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     // #if ! JucePlugin_IsMidiEffect
                     //  #if ! JucePlugin_IsSynth
                     //   .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     //  #endif
                     //   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     // #endif
                     .withOutput ("Output",  juce::AudioChannelSet::stereo(), true)
                       ), apvts (*this, nullptr, "Parameters", createParameterLayout())

{
    paramsCache.init(apvts);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32> (samplesPerBlock),
        static_cast<juce::uint32> (getTotalNumOutputChannels())
    };

    juce::ignoreUnused (sampleRate, samplesPerBlock);
    carrier.prepare(spec);
    midiOut.attachRing(&dspOutletRing);
    midiOut.setChannel(1);
    midiOut.setCcNumber(74);
    midiOut.setRateHz(160.0);
    midiOut.prepare(sampleRate);
}

void AudioPluginAudioProcessor::releaseResources()
{
    disconnectLsl();
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals; // Maybe cut
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    const float freq = paramsCache.freq->load();
    const float gain = paramsCache.gain->load();


    // int samples_to_read = 1;
    // //DBG ("EEG BUF Available " << eegInletRing.available());
    //
    // if ( dspOutletRing.available() >= samples_to_read) {
    //     auto readView = dspOutletRing.beginRead (samples_to_read);
    //     int read = 0;
    //     for (int i = 0; i < readView.n1; i++) {
    //         DBG ("EEG: " << readView.p1[i].value << "  ts=" << readView.p1[i].stamp);
    //         read++;
    //     }
    //     for (int i = 0; i < readView.n2; i++) {
    //         DBG ("EEG: " << readView.p2[i].value << "  ts=" << readView.p2[i].stamp);
    //         read++;
    //     }
    //     dspOutletRing.finishRead(read);
    // }

    // std::cout << gain << std::endl;
    // std::cout << freq << std::endl;
    midiOut.process(buffer.getNumSamples(),midiMessages);


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
        carrier.setFrequency(freq);
        carrier.setAmplitude(gain);
        carrier.process(buffer);

    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(Params::IDs::Freq
, "freq",
    juce::NormalisableRange<float>(0.f, 10000.f, 10.0), 400.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(Params::IDs::Gain, "Gain",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    return { params.begin(), params.end() };
}