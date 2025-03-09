#pragma once

#include <JuceHeader.h>

enum class FilterType
{
    LowPass,
    BandPass,
    HighPass
};

class FilterProcessor
{
public:
    FilterProcessor();
    ~FilterProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float> &buffer);
    void reset();

    // Parameter setters
    void setFrequency(float newFrequency); // 20 - 20000 Hz
    void setFilterType(FilterType newType);
    void setFilterType(const juce::String &typeName);
    void setResonance(float newResonance); // 0.1 - 10.0

    // Parameter getters
    float getFrequency() const;
    FilterType getFilterType() const;
    juce::String getFilterTypeName() const;
    float getResonance() const;

    // Get filter response for visual display
    void getMagnitudeResponse(double *frequencies, double *magnitudes, int numPoints);

private:
    // Parameters
    float frequency;       // Filter cutoff frequency in Hz
    FilterType filterType; // Type of filter
    float resonance;       // Q factor / resonance

    // Internal state
    double currentSampleRate;
    int bufferSize;

    // Filters for each channel
    std::vector<std::unique_ptr<juce::IIRFilter>> filters;

    // Update filter coefficients based on current settings
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor)
};