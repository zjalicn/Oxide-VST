#include "DistortionProcessor.h"

DistortionProcessor::DistortionProcessor()
    : drive(0.5f), mix(0.5f)
{
}

void DistortionProcessor::prepare(double sampleRate)
{
}

void DistortionProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Create a dry buffer for the mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = distort(channelData[sample]);
        }
    }
    
    if (mix < 1.0f)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* wetData = buffer.getWritePointer(channel);
            const float* dryData = dryBuffer.getReadPointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                wetData[sample] = wetData[sample] * mix + dryData[sample] * (1.0f - mix);
            }
        }
    }
}

float DistortionProcessor::distort(float sample)
{
    float driven = sample * (1.0f + drive * 3.0f);
    
    float distorted = std::tanh(driven);
    
    return distorted;
}

void DistortionProcessor::setDrive(float newDrive)
{
    drive = juce::jlimit(0.0f, 1.0f, newDrive);
}

float DistortionProcessor::getDrive() const
{
    return drive;
}

void DistortionProcessor::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

float DistortionProcessor::getMix() const
{
    return mix;
}