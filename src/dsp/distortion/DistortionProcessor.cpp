#include "DistortionProcessor.h"

DistortionProcessor::DistortionProcessor()
    : drive(0.5f), mix(0.5f), inputGain(0.0f), outputGain(0.0f),
      inputGainLinear(1.0f), outputGainLinear(1.0f),
      currentAlgorithm(DistortionAlgorithm::SoftClip)
{
}

void DistortionProcessor::prepare(double sampleRate)
{
    // Initialize any algorithm-specific setup here if needed
}

void DistortionProcessor::processBlock(juce::AudioBuffer<float> &buffer)
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
            float *channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] *= inputGainLinear;
            }
        }
    }

    // Apply distortion
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float *channelData = buffer.getWritePointer(channel);

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
            float *wetData = buffer.getWritePointer(channel);
            const float *dryData = dryBuffer.getReadPointer(channel);

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
            float *channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] *= outputGainLinear;
            }
        }
    }
}

float DistortionProcessor::distort(float sample)
{
    // Route to the appropriate algorithm implementation
    switch (currentAlgorithm)
    {
    case DistortionAlgorithm::SoftClip:
        return softClip(sample);

    case DistortionAlgorithm::HardClip:
        return hardClip(sample);

    case DistortionAlgorithm::Foldback:
        return foldback(sample);

    case DistortionAlgorithm::Waveshaper:
        return waveshaper(sample);

    case DistortionAlgorithm::Bitcrusher:
        return bitcrusher(sample);

    default:
        return softClip(sample);
    }
}

float DistortionProcessor::softClip(float sample)
{
    float driven = sample * (1.0f + drive * 3.0f);
    return std::tanh(driven);
}

float DistortionProcessor::hardClip(float sample)
{
    float threshold = 1.0f - drive * 0.9f;
    float driven = sample * (1.0f + drive * 5.0f);

    // Hard clipping at threshold
    if (driven > threshold)
        return threshold;
    else if (driven < -threshold)
        return -threshold;
    else
        return driven;
}

float DistortionProcessor::foldback(float sample)
{
    // Adjust threshold based on drive (lower drive = higher threshold)
    float threshold = 1.0f / (1.0f + drive * 3.0f);
    float driven = sample * (1.0f + drive * 3.0f);

    // Foldback algorithm
    if (driven > threshold || driven < -threshold)
    {
        // Get the number of times the signal has crossed the threshold
        float foldCount = std::floor(std::abs(driven) / threshold);
        // Apply folding
        if (static_cast<int>(foldCount) % 2 == 0)
            return driven - threshold * foldCount * (driven > 0 ? 1 : -1);
        else
            return threshold * (foldCount + 1) - driven * (driven > 0 ? 1 : -1);
    }

    return driven;
}

float DistortionProcessor::waveshaper(float sample)
{
    float driven = sample * (1.0f + drive * 5.0f);

    // Custom waveshaping curve (polynomial approximation)
    // This creates more complex harmonic distortion
    float x = driven;
    float absX = std::abs(x);
    float sign = x > 0 ? 1.0f : -1.0f;

    return sign * (1.0f - std::exp(-absX * (drive * 3.0f + 1.0f)));
}

float DistortionProcessor::bitcrusher(float sample)
{
    float driven = sample * (1.0f + drive * 3.0f);

    // Calculate bit reduction amount
    int bits = static_cast<int>(16.0f - drive * 14.0f); // Between 2 and 16 bits
    bits = juce::jlimit(2, 16, bits);

    // Calculate step size for the given bit depth
    float steps = std::pow(2.0f, static_cast<float>(bits));

    // Quantize the signal
    return std::floor(driven * steps) / steps;
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

void DistortionProcessor::setAlgorithm(DistortionAlgorithm newAlgorithm)
{
    currentAlgorithm = newAlgorithm;
}

void DistortionProcessor::setAlgorithm(const juce::String &algorithmName)
{
    // Convert string to enum
    if (algorithmName == "soft_clip")
        currentAlgorithm = DistortionAlgorithm::SoftClip;
    else if (algorithmName == "hard_clip")
        currentAlgorithm = DistortionAlgorithm::HardClip;
    else if (algorithmName == "foldback")
        currentAlgorithm = DistortionAlgorithm::Foldback;
    else if (algorithmName == "waveshaper")
        currentAlgorithm = DistortionAlgorithm::Waveshaper;
    else if (algorithmName == "bitcrusher")
        currentAlgorithm = DistortionAlgorithm::Bitcrusher;
    // Fallback to soft clip if unknown
    else
        currentAlgorithm = DistortionAlgorithm::SoftClip;
}

DistortionAlgorithm DistortionProcessor::getAlgorithm() const
{
    return currentAlgorithm;
}

juce::String DistortionProcessor::getAlgorithmName() const
{
    switch (currentAlgorithm)
    {
    case DistortionAlgorithm::SoftClip:
        return "soft_clip";
    case DistortionAlgorithm::HardClip:
        return "hard_clip";
    case DistortionAlgorithm::Foldback:
        return "foldback";
    case DistortionAlgorithm::Waveshaper:
        return "waveshaper";
    case DistortionAlgorithm::Bitcrusher:
        return "bitcrusher";
    default:
        return "soft_clip";
    }
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