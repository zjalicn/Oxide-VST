#pragma once

#include <JuceHeader.h>

class PulseProcessor
{
public:
    PulseProcessor();
    ~PulseProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float> &buffer);
    void reset();

    // Parameter setters
    void setMix(float newMix);  // 0.0 - 1.0
    void setBpm(double newBpm); // In BPM

    // Parameter getters
    float getMix() const;
    double getBpm() const;

private:
    // Parameters
    float mix;         // Wet/dry mix
    double currentBpm; // BPM for pulse timing
    double currentSampleRate;
    int bufferSize;

    // Internal phase for pulse generation
    double phase;
    double phaseIncrement;

    // Smoothed envelope for anti-click transitions
    juce::LinearSmoothedValue<float> smoothedEnvelope;

    // Calculate the pulse envelope value for a specific phase position
    float calculateEnvelope(double phasePosition) const;

    // Update phase increment based on BPM
    void updatePhaseIncrement();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PulseProcessor)
};