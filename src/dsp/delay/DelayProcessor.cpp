#include "DelayProcessor.h"

DelayProcessor::DelayProcessor()
    : delayTime(0.5f),        // 500ms default delay time
      feedback(0.4f),         // 40% default feedback
      mix(0.3f),              // 30% default mix
      filterFreq(5000.0f),    // 5kHz default filter cutoff
      pingPongEnabled(false), // Ping-pong disabled by default
      currentSampleRate(44100.0),
      bufferSize(0)
{
}

void DelayProcessor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;

    // Allocate a 2-second buffer at the current sample rate (more than enough for 2.0s max delay)
    bufferSize = static_cast<int>(currentSampleRate * 2.0);

    // Clear and resize delay buffers
    delayBuffers.clear();
    writePositions.clear();
    filters.clear();

    // Create 2 buffers for stereo processing
    for (int i = 0; i < 2; ++i)
    {
        // Create delay buffer
        auto delayBuffer = std::make_unique<juce::AudioBuffer<float>>(1, bufferSize);
        delayBuffer->clear();
        delayBuffers.push_back(std::move(delayBuffer));

        // Initialize write position
        writePositions.push_back(0);

        // Create feedback filter
        auto filter = std::make_unique<juce::IIRFilter>();
        filter->setCoefficients(juce::IIRCoefficients::makeLowPass(currentSampleRate, filterFreq));
        filters.push_back(std::move(filter));
    }
}

void DelayProcessor::processBlock(juce::AudioBuffer<float> &buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Early return if we haven't been prepared yet
    if (bufferSize == 0 || delayBuffers.empty())
        return;

    // Create a buffer for the dry signal
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Calculate delay time in samples with room for interpolation
    float delaySamples = calculateDelaySamples();

    // Process each channel
    for (int channel = 0; channel < numChannels && channel < 2; ++channel)
    {
        float *channelData = buffer.getWritePointer(channel);
        float *delayData = delayBuffers[channel]->getWritePointer(0);

        int channelForFeedback = channel;

        // In ping-pong mode, left channel feeds into right and vice versa
        if (pingPongEnabled && numChannels > 1)
        {
            channelForFeedback = 1 - channel;
        }

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Get the current write position
            int writePos = writePositions[channel];

            // Calculate the read position with interpolation
            float readPos = static_cast<float>(writePos) - delaySamples;
            if (readPos < 0)
                readPos += bufferSize;

            // Get the delayed sample with interpolation
            float delaySample = getInterpolatedSample(delayData, bufferSize, readPos);

            // Apply filter to the feedback signal
            float filteredSample = filters[channel]->processSingleSampleRaw(delaySample);

            // Write to the delay buffer (current input + filtered feedback)
            float inputWithFeedback = channelData[sample] + filteredSample * feedback;
            delayData[writePos] = inputWithFeedback;

            // Apply the wet/dry mix
            channelData[sample] = channelData[sample] * (1.0f - mix) + delaySample * mix;

            // Increment and wrap the write position
            writePositions[channel] = (writePos + 1) % bufferSize;
        }
    }
}

void DelayProcessor::reset()
{
    for (auto &buffer : delayBuffers)
    {
        buffer->clear();
    }

    for (auto &filter : filters)
    {
        filter->reset();
    }
}

float DelayProcessor::calculateDelaySamples() const
{
    // Convert delay time in seconds to samples
    return delayTime * static_cast<float>(currentSampleRate);
}

int DelayProcessor::getReadPosition(int writePosition, float delaySamples) const
{
    int readPos = static_cast<int>(writePosition - delaySamples);

    // Wrap around if needed
    while (readPos < 0)
        readPos += bufferSize;

    return readPos % bufferSize;
}

float DelayProcessor::getInterpolatedSample(const float *bufferData, int bufferSize, float readPos) const
{
    // Linear interpolation between two adjacent samples
    int pos1 = static_cast<int>(readPos);
    int pos2 = (pos1 + 1) % bufferSize;
    float frac = readPos - static_cast<float>(pos1);

    return bufferData[pos1] * (1.0f - frac) + bufferData[pos2] * frac;
}

void DelayProcessor::setDelayTime(float newDelayTime)
{
    // Clamp to reasonable range (10ms to 2 seconds)
    delayTime = juce::jlimit(0.01f, 2.0f, newDelayTime);
}

void DelayProcessor::setFeedback(float newFeedback)
{
    feedback = juce::jlimit(0.0f, 1.0f, newFeedback);
}

void DelayProcessor::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

void DelayProcessor::setFilterFreq(float newFrequency)
{
    filterFreq = juce::jlimit(20.0f, 20000.0f, newFrequency);

    // Update filter coefficients
    for (auto &filter : filters)
    {
        filter->setCoefficients(juce::IIRCoefficients::makeLowPass(currentSampleRate, filterFreq));
    }
}

void DelayProcessor::setPingPong(bool enabled)
{
    pingPongEnabled = enabled;
}

float DelayProcessor::getDelayTime() const
{
    return delayTime;
}

float DelayProcessor::getFeedback() const
{
    return feedback;
}

float DelayProcessor::getMix() const
{
    return mix;
}

float DelayProcessor::getFilterFreq() const
{
    return filterFreq;
}

bool DelayProcessor::getPingPong() const
{
    return pingPongEnabled;
}