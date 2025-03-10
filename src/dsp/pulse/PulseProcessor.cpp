#include "PulseProcessor.h"

PulseProcessor::PulseProcessor()
    : mix(0.0f),         // Default to no effect
      currentBpm(120.0), // Default 120 BPM
      currentSampleRate(44100.0),
      bufferSize(0),
      phase(0.0),
      phaseIncrement(0.0)
{
    updatePhaseIncrement();
}

void PulseProcessor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    bufferSize = maxBlockSize;

    // Reset phase
    phase = 0.0;

    // Update phase increment for the new sample rate
    updatePhaseIncrement();
}

void PulseProcessor::processBlock(juce::AudioBuffer<float> &buffer)
{
    // If mix is 0, no need to process
    if (mix <= 0.001f)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Create a dry buffer for the mix calculation
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Calculate the pulse envelope value for this sample
        float envelopeValue = calculateEnvelope(phase);

        // Apply envelope to all channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float *channelData = buffer.getWritePointer(channel);
            const float *dryData = dryBuffer.getReadPointer(channel);

            // Apply envelope with mix control
            // At mix = 0: use dry signal
            // At mix = 1: fully apply envelope
            float wetSignal = dryData[sample] * envelopeValue;
            channelData[sample] = (wetSignal * mix) + (dryData[sample] * (1.0f - mix));
        }

        // Advance phase
        phase += phaseIncrement;
        if (phase >= 1.0)
            phase -= 1.0;
    }
}

void PulseProcessor::reset()
{
    phase = 0.0;
}

float PulseProcessor::calculateEnvelope(double phasePosition) const
{
    // The amplitude will alternate between 0.0 (silence) and 1.0 (full volume)
    // based on the phase position.

    // We want a quarter note pulse, which means one up-down cycle per beat
    // The pattern starts silent and then increases to full volume over an eighth note

    // Start with silent, then smoothly increase to full volume
    if (phasePosition < 0.25)
    {
        // First quarter of the beat (eighth note) - ramp from silence to full volume
        // Use a smooth sine curve for natural pumping effect
        double normalizedPosition = phasePosition * 4.0; // Map 0.0-0.25 to 0.0-1.0

        // Calculate curve using a sine function that starts at 0 and ends at 1
        // sin function in range 0 to π/2 gives nice smooth curve from 0 to 1
        double curveValue = std::sin(normalizedPosition * juce::MathConstants<double>::halfPi);

        // This gives us a smooth curve from 0 to 1
        return static_cast<float>(curveValue);
    }
    else
    {
        // Rest of the beat - full volume
        return 1.0f;
    }
}

void PulseProcessor::updatePhaseIncrement()
{
    // Convert BPM to phase increment per sample
    // One complete cycle (phase 0 to 1) equals one beat (quarter note)
    double beatsPerSecond = currentBpm / 60.0;
    phaseIncrement = beatsPerSecond / currentSampleRate;
}

void PulseProcessor::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

void PulseProcessor::setBpm(double newBpm)
{
    currentBpm = juce::jlimit(20.0, 300.0, newBpm);
    updatePhaseIncrement();
}

float PulseProcessor::getMix() const
{
    return mix;
}

double PulseProcessor::getBpm() const
{
    return currentBpm;
}