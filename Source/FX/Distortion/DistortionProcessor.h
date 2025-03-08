#pragma once

#include <JuceHeader.h>

class DistortionProcessor
{
public:
    DistortionProcessor();
    
    void prepare(double sampleRate);
    
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    void setDrive(float newDrive);
    float getDrive() const;
    
    void setMix(float newMix);
    float getMix() const;
    
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
    
    // Gain values converted to linear multipliers
    float inputGainLinear;
    float outputGainLinear;
    
    // Simple waveshaping function
    float distort(float sample);
    
    // Utility to convert dB to linear gain
    float dbToGain(float gainInDb);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionProcessor)
};