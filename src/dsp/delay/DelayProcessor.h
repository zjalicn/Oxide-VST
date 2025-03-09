#pragma once

#include <JuceHeader.h>

class DelayProcessor
{
public:
    DelayProcessor();
    ~DelayProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float> &buffer);
    void reset();

    // Parameter setters
    void setDelayTime(float newDelayTime);  // in seconds (0.01 - 2.0)
    void setFeedback(float newFeedback);    // 0.0 - 1.0
    void setMix(float newMix);              // 0.0 - 1.0
    void setFilterFreq(float newFrequency); // 20 - 20000 Hz
    void setPingPong(bool enabled);         // stereo ping-pong effect

    // Parameter getters
    float getDelayTime() const;
    float getFeedback() const;
    float getMix() const;
    float getFilterFreq() const;
    bool getPingPong() const;

private:
    // Parameters
    float delayTime;      // Delay time in seconds
    float feedback;       // Feedback amount
    float mix;            // Wet/dry mix
    float filterFreq;     // Filter cutoff frequency
    bool pingPongEnabled; // Stereo ping-pong mode

    // Internal state
    double currentSampleRate;
    int bufferSize;

    // Delay buffers for each channel
    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> delayBuffers;
    std::vector<int> writePositions;

    // Filter for feedback path
    std::vector<std::unique_ptr<juce::IIRFilter>> filters;

    // Utility functions
    float calculateDelaySamples() const;
    int getReadPosition(int writePosition, float delaySamples) const;
    float getInterpolatedSample(const float *bufferData, int bufferSize, float readPos) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayProcessor)
};