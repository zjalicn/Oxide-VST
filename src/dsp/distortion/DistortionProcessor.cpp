#include "DistortionProcessor.h"

DistortionProcessor::DistortionProcessor()
    : drive(0.5f), mix(0.5f), inputGain(0.0f), outputGain(0.0f),
      inputGainLinear(1.0f), outputGainLinear(1.0f)
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
    
    // Apply input gain
    if (inputGainLinear != 1.0f)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] *= inputGainLinear;
            }
        }
    }
    
    // Apply distortion
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = distort(channelData[sample]);
        }
    }
    
    // Apply wet/dry mix
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
    
    // Apply output gain
    if (outputGainLinear != 1.0f)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] *= outputGainLinear;
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

void DistortionProcessor::setInputGain(float gainInDb)
{
    inputGain = juce::jlimit(-12.0f, 12.0f, gainInDb);
    inputGainLinear = dbToGain(inputGain);
}

float DistortionProcessor::getInputGain() const
{
    return inputGain;
}

void DistortionProcessor::setOutputGain(float gainInDb)
{
    outputGain = juce::jlimit(-12.0f, 12.0f, gainInDb);
    outputGainLinear = dbToGain(outputGain);
}

float DistortionProcessor::getOutputGain() const
{
    return outputGain;
}

float DistortionProcessor::dbToGain(float gainInDb)
{
    return std::pow(10.0f, gainInDb / 20.0f);
}