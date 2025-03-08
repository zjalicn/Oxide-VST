#include "PluginProcessor.h"
#include "PluginEditor.h"

OxideAudioProcessor::OxideAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

OxideAudioProcessor::~OxideAudioProcessor()
{
}

const juce::String OxideAudioProcessor::getName() const
{
    return "Oxide";
}

bool OxideAudioProcessor::acceptsMidi() const
{
    return false;
}

bool OxideAudioProcessor::producesMidi() const
{
    return false;
}

bool OxideAudioProcessor::isMidiEffect() const
{
    return false;
}

double OxideAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OxideAudioProcessor::getNumPrograms()
{
    return 1;
}

int OxideAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OxideAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String OxideAudioProcessor::getProgramName(int index)
{
    return {};
}

void OxideAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void OxideAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    levelLeft.reset(sampleRate, 0.1);  // Smooth over 100ms
    levelRight.reset(sampleRate, 0.1);
    
    distortionProcessor.prepare(sampleRate);
}

void OxideAudioProcessor::releaseResources()
{
    // When playback stops, you can use this to free up any resources
}

bool OxideAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    // We only support stereo and mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void OxideAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Calculate input levels for the meters
    float newLevelLeft = 0.0f;
    float newLevelRight = 0.0f;
    
    if (totalNumInputChannels > 0)
        newLevelLeft = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    
    if (totalNumInputChannels > 1)
        newLevelRight = buffer.getRMSLevel(1, 0, buffer.getNumSamples());
    
    levelLeft.setTargetValue(newLevelLeft);
    levelRight.setTargetValue(newLevelRight);
    levelLeft.skip(buffer.getNumSamples());
    levelRight.skip(buffer.getNumSamples());
    
    // Process the distortion
    distortionProcessor.processBlock(buffer);
}

bool OxideAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* OxideAudioProcessor::createEditor()
{
    return new OxideAudioProcessorEditor(*this);
}

void OxideAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Store plugin state (parameters) here
    juce::MemoryOutputStream stream(destData, true);
    
    stream.writeFloat(distortionProcessor.getDrive());
    stream.writeFloat(distortionProcessor.getMix());
    stream.writeFloat(distortionProcessor.getInputGain());
    stream.writeFloat(distortionProcessor.getOutputGain());
}

void OxideAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore plugin state (parameters) here
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    
    if (stream.getNumBytesRemaining() >= sizeof(float) * 4)
    {
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        float inputGain = stream.readFloat();
        float outputGain = stream.readFloat();
        
        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(inputGain);
        distortionProcessor.setOutputGain(outputGain);
    }
    else if (stream.getNumBytesRemaining() >= sizeof(float) * 2)
    {
        // Backward compatibility with older plugin versions
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        
        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(0.0f);
        distortionProcessor.setOutputGain(0.0f);
    }
}

// This creates the plugin instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OxideAudioProcessor();
}