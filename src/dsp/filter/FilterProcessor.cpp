#include "FilterProcessor.h"
#include <complex>

FilterProcessor::FilterProcessor()
    : frequency(1000.0f),              // 1kHz default frequency
      filterType(FilterType::LowPass), // Default to low pass
      resonance(0.7f),                 // Default resonance
      currentSampleRate(44100.0),
      bufferSize(0)
{
}

void FilterProcessor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    bufferSize = maxBlockSize;

    // Clear and resize filter objects
    filters.clear();

    // Create filters for stereo processing (2 channels)
    for (int i = 0; i < 2; ++i)
    {
        auto filter = std::make_unique<juce::IIRFilter>();
        filters.push_back(std::move(filter));
    }

    // Initialize filter coefficients
    updateFilters();
}

void FilterProcessor::processBlock(juce::AudioBuffer<float> &buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Early return if we haven't been prepared yet
    if (filters.empty())
        return;

    // Process each channel
    for (int channel = 0; channel < numChannels && channel < filters.size(); ++channel)
    {
        float *channelData = buffer.getWritePointer(channel);
        auto &filter = filters[channel];

        // Process the samples through the filter
        filter->processSamples(channelData, numSamples);
    }
}

void FilterProcessor::reset()
{
    for (auto &filter : filters)
    {
        filter->reset();
    }
}

void FilterProcessor::updateFilters()
{
    // Ensure the filter is within valid range
    float safeFreq = juce::jlimit(20.0f, 20000.0f, frequency);
    float safeRes = juce::jlimit(0.1f, 10.0f, resonance);

    // Create appropriate coefficients based on filter type
    juce::IIRCoefficients coeffs;

    switch (filterType)
    {
    case FilterType::LowPass:
        coeffs = juce::IIRCoefficients::makeLowPass(currentSampleRate, safeFreq, safeRes);
        break;
    case FilterType::BandPass:
        coeffs = juce::IIRCoefficients::makeBandPass(currentSampleRate, safeFreq, safeRes);
        break;
    case FilterType::HighPass:
        coeffs = juce::IIRCoefficients::makeHighPass(currentSampleRate, safeFreq, safeRes);
        break;
    default:
        coeffs = juce::IIRCoefficients::makeLowPass(currentSampleRate, safeFreq, safeRes);
        break;
    }

    // Apply coefficients to all filters
    for (auto &filter : filters)
    {
        filter->setCoefficients(coeffs);
    }
}

void FilterProcessor::setFrequency(float newFrequency)
{
    frequency = juce::jlimit(20.0f, 20000.0f, newFrequency);
    updateFilters();
}

void FilterProcessor::setFilterType(FilterType newType)
{
    filterType = newType;
    updateFilters();
}

void FilterProcessor::setFilterType(const juce::String &typeName)
{
    if (typeName == "lowpass")
        filterType = FilterType::LowPass;
    else if (typeName == "bandpass")
        filterType = FilterType::BandPass;
    else if (typeName == "highpass")
        filterType = FilterType::HighPass;
    else
        filterType = FilterType::LowPass; // Default to lowpass for unknown types

    updateFilters();
}

void FilterProcessor::setResonance(float newResonance)
{
    resonance = juce::jlimit(0.1f, 10.0f, newResonance);
    updateFilters();
}

float FilterProcessor::getFrequency() const
{
    return frequency;
}

FilterType FilterProcessor::getFilterType() const
{
    return filterType;
}

juce::String FilterProcessor::getFilterTypeName() const
{
    switch (filterType)
    {
    case FilterType::LowPass:
        return "lowpass";
    case FilterType::BandPass:
        return "bandpass";
    case FilterType::HighPass:
        return "highpass";
    default:
        return "lowpass";
    }
}

float FilterProcessor::getResonance() const
{
    return resonance;
}

void FilterProcessor::getMagnitudeResponse(double *frequencies, double *magnitudes, int numPoints)
{
    // Early return if filters are not initialized
    if (filters.empty())
        return;

    // Calculate the magnitude response at each frequency point
    for (int i = 0; i < numPoints; ++i)
    {
        // Log scale from 20Hz to 20kHz
        double freqHz = 20.0 * std::pow(1000.0, i / (double)(numPoints - 1));
        frequencies[i] = freqHz;

        // Normalized frequency (0 to π)
        double normalizedFreq = (2.0 * juce::MathConstants<double>::pi * freqHz) / currentSampleRate;

        // Calculate magnitude response manually based on the filter type
        double magnitude = 0.0;

        // Simple approximations for filter responses
        double omega = 2.0 * M_PI * freqHz / currentSampleRate;
        double cutoff = 2.0 * M_PI * frequency / currentSampleRate;
        double q = resonance;

        switch (filterType)
        {
        case FilterType::LowPass:
            // Simple lowpass filter response approximation
            magnitude = 1.0 / sqrt(1.0 + pow((omega / cutoff), (2.0 * q)));
            break;

        case FilterType::HighPass:
            // Simple highpass filter response approximation
            magnitude = 1.0 / sqrt(1.0 + pow((cutoff / omega), (2.0 * q)));
            break;

        case FilterType::BandPass:
            // Simple bandpass filter response approximation
            {
                double bw = 1.0 / q;
                magnitude = 1.0 / (1.0 + pow((omega / cutoff - cutoff / omega) / bw, 2.0));
            }
            break;

        default:
            magnitude = 1.0;
            break;
        }

        magnitudes[i] = magnitude;
    }
}