#pragma once

#include <JuceHeader.h>

enum class DistortionAlgorithm
{
    SoftClip,   // Smooth, tape-like saturation
    HardClip,   // More aggressive clipping, similar to pedals
    Foldback,   // Signal folding for more harmonics
    Waveshaper, // Custom waveshaping curve
    Bitcrusher  // Digital distortion with sample rate/bit reduction
};

class DistortionProcessor
{
public:
    DistortionProcessor();

    void prepare(double sampleRate);

    void processBlock(juce::AudioBuffer<float> &buffer);

    void setDrive(float newDrive);
    float getDrive() const;

    void setMix(float newMix);
    float getMix() const;

    void setAlgorithm(DistortionAlgorithm newAlgorithm);
    void setAlgorithm(const juce::String &algorithmName);
    DistortionAlgorithm getAlgorithm() const;
    juce::String getAlgorithmName() const;

    // Input and output gain in dB (-12 to +12)
    void setInputGain(float gainInDb);
    float getInputGain() const;

    void setOutputGain(float gainInDb);
    float getOutputGain() const;

private:
    float drive;      // Distortion amount (0.0 - 1.0)
    float mix;        // Wet/dry mix (0.0 - 1.0)
    float inputGain;  // Input gain in dB (-12 to +12)
    float outputGain; // Output gain in dB (-12 to +12)

    float inputGainLinear;
    float outputGainLinear;

    DistortionAlgorithm currentAlgorithm;

    float distort(float sample);
    float softClip(float sample);
    float hardClip(float sample);
    float foldback(float sample);
    float waveshaper(float sample);
    float bitcrusher(float sample);

    float dbToGain(float gainInDb);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionProcessor)
};