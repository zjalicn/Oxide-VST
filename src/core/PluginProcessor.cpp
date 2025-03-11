#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetManager.h"

OxideAudioProcessor::OxideAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      presetManagerInitialized(false)
{
    // Defer PresetManager initialization to avoid constructor issues
    // It will be created on first access via getPresetManager()
}

OxideAudioProcessor::~OxideAudioProcessor()
{
    // Destroy preset manager first (it has a reference to this processor)
    presetManager.reset();
}

PresetManager *OxideAudioProcessor::getPresetManager()
{
    // Lazy initialization of the PresetManager to avoid constructor issues
    if (!presetManagerInitialized)
    {
        try
        {
            presetManager = std::make_unique<PresetManager>(*this);
            presetManagerInitialized = true;
        }
        catch (const std::exception &)
        {
            // Silently handle any initialization errors
        }
    }

    return presetManager.get();
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

void OxideAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

void OxideAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    levelLeft.reset(sampleRate, 0.1); // Smooth over 100ms
    levelRight.reset(sampleRate, 0.1);

    // Prepare DSP components in signal chain order
    delayProcessor.prepare(sampleRate, samplesPerBlock);
    distortionProcessor.prepare(sampleRate);
    filterProcessor.prepare(sampleRate, samplesPerBlock);
    pulseProcessor.prepare(sampleRate, samplesPerBlock);
}

void OxideAudioProcessor::releaseResources()
{
    // When playback stops, release all resources
    delayProcessor.reset();
    filterProcessor.reset();
    pulseProcessor.reset();
}

bool OxideAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    // We only support stereo and mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void OxideAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels that didn't contain input data
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

    // Get the current BPM from the host
    double currentBpm = 120.0; // Default value
    if (auto *playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo positionInfo;
        if (playHead->getCurrentPosition(positionInfo))
        {
            currentBpm = positionInfo.bpm;
        }
    }

    // Update pulse processor BPM
    pulseProcessor.setBpm(currentBpm);

    // Process audio through signal chain
    delayProcessor.processBlock(buffer);      // First delay
    distortionProcessor.processBlock(buffer); // Then distortion
    filterProcessor.processBlock(buffer);     // Then filter
    pulseProcessor.processBlock(buffer);      // Finally pulse effect

    // Store post-processed buffer for oscilloscope
    {
        juce::ScopedLock lock(outputBufferLock);
        outputBuffer.makeCopyOf(buffer);
    }
}

bool OxideAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *OxideAudioProcessor::createEditor()
{
    return new OxideAudioProcessorEditor(*this);
}

void OxideAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // Store plugin state (parameters)
    juce::MemoryOutputStream stream(destData, true);

    // Store distortion parameters
    stream.writeFloat(distortionProcessor.getDrive());
    stream.writeFloat(distortionProcessor.getMix());
    stream.writeFloat(distortionProcessor.getInputGain());
    stream.writeFloat(distortionProcessor.getOutputGain());

    // Store the distortion algorithm as an integer value
    int algorithmValue = static_cast<int>(distortionProcessor.getAlgorithm());
    stream.writeInt(algorithmValue);

    // Store delay parameters
    stream.writeFloat(delayProcessor.getDelayTime());
    stream.writeFloat(delayProcessor.getFeedback());
    stream.writeFloat(delayProcessor.getMix());
    stream.writeInt(delayProcessor.getPingPong() ? 1 : 0);

    // Store filter parameters
    stream.writeFloat(filterProcessor.getFrequency());
    stream.writeFloat(filterProcessor.getResonance());
    stream.writeInt(static_cast<int>(filterProcessor.getFilterType()));

    // Store pulse parameters
    stream.writeFloat(pulseProcessor.getMix());
    stream.writeInt(static_cast<int>(pulseProcessor.getRate()));
}

void OxideAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // Restore plugin state (parameters)
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

    // Check how much data we have available
    const int bytesAvailable = stream.getNumBytesRemaining();

    if (bytesAvailable >= sizeof(float) * 4 + sizeof(int) + sizeof(float) * 4 + sizeof(int) * 3 + sizeof(float))
    {
        // Full state with all processor parameters including pulse

        // Distortion parameters
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        float inputGain = stream.readFloat();
        float outputGain = stream.readFloat();
        int algorithmValue = stream.readInt();

        // Delay parameters
        float delayTime = stream.readFloat();
        float feedback = stream.readFloat();
        float delayMix = stream.readFloat();
        bool pingPong = stream.readInt() > 0;

        // Filter parameters
        float filterFreq = stream.readFloat();
        float resonance = stream.readFloat();
        int filterTypeValue = stream.readInt();

        // Pulse parameters
        float pulseMix = stream.readFloat();

        // Apply distortion parameters
        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(inputGain);
        distortionProcessor.setOutputGain(outputGain);
        distortionProcessor.setAlgorithm("soft_clip");

        // Apply delay parameters
        delayProcessor.setDelayTime(delayTime);
        delayProcessor.setFeedback(feedback);
        delayProcessor.setMix(delayMix);
        delayProcessor.setPingPong(pingPong);

        // Apply filter parameters
        filterProcessor.setFrequency(filterFreq);
        filterProcessor.setResonance(resonance);
        filterProcessor.setFilterType(static_cast<FilterType>(filterTypeValue));

        // Apply pulse parameters
        pulseProcessor.setMix(pulseMix);
    }
    else if (bytesAvailable >= sizeof(float) * 4 + sizeof(int) + sizeof(float) * 4 + sizeof(int) * 3)
    {
        // Full state with all processor parameters but no pulse

        // Distortion parameters
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        float inputGain = stream.readFloat();
        float outputGain = stream.readFloat();
        int algorithmValue = stream.readInt();

        // Delay parameters
        float delayTime = stream.readFloat();
        float feedback = stream.readFloat();
        float delayMix = stream.readFloat();
        bool pingPong = stream.readInt() > 0;

        // Filter parameters
        float filterFreq = stream.readFloat();
        float resonance = stream.readFloat();
        int filterTypeValue = stream.readInt();

        // Apply distortion parameters
        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(inputGain);
        distortionProcessor.setOutputGain(outputGain);
        distortionProcessor.setAlgorithm("soft_clip");

        // Apply delay parameters
        delayProcessor.setDelayTime(delayTime);
        delayProcessor.setFeedback(feedback);
        delayProcessor.setMix(delayMix);
        delayProcessor.setPingPong(pingPong);

        // Apply filter parameters
        filterProcessor.setFrequency(filterFreq);
        filterProcessor.setResonance(resonance);
        filterProcessor.setFilterType(static_cast<FilterType>(filterTypeValue));

        // Use default for pulse
        pulseProcessor.setMix(0.0f);
    }
    else if (bytesAvailable >= sizeof(float) * 4 + sizeof(int))
    {
        // State with just distortion parameters (backward compatibility)
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        float inputGain = stream.readFloat();
        float outputGain = stream.readFloat();
        int algorithmValue = stream.readInt();

        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(inputGain);
        distortionProcessor.setOutputGain(outputGain);
        distortionProcessor.setAlgorithm("soft_clip");

        // Use default values for delay and filter
        // Use default for pulse
        pulseProcessor.setMix(0.0f);
    }
    else if (bytesAvailable >= sizeof(float) * 4)
    {
        // State without algorithm (backward compatibility)
        float drive = stream.readFloat();
        float mix = stream.readFloat();
        float inputGain = stream.readFloat();
        float outputGain = stream.readFloat();

        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(inputGain);
        distortionProcessor.setOutputGain(outputGain);
        // Use default algorithm
        distortionProcessor.setAlgorithm("soft_clip");

        // Use default values for delay and filter
        // Use default for pulse
        pulseProcessor.setMix(0.0f);
    }
    else if (bytesAvailable >= sizeof(float) * 2)
    {
        // Even older version with just drive and mix
        float drive = stream.readFloat();
        float mix = stream.readFloat();

        distortionProcessor.setDrive(drive);
        distortionProcessor.setMix(mix);
        distortionProcessor.setInputGain(0.0f);
        distortionProcessor.setOutputGain(0.0f);
        distortionProcessor.setAlgorithm("soft_clip");

        // Use default values for delay and filter
        // Use default for pulse
        pulseProcessor.setMix(0.0f);
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new OxideAudioProcessor();
}