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
    
private:
    float drive;   // Distortion amount (0.0 - 1.0)
    float mix;     // Wet/dry mix (0.0 - 1.0)
    
    // Simple waveshaping function
    float distort(float sample);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionProcessor)
};