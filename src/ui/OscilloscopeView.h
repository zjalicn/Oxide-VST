#pragma once

#include <JuceHeader.h>
#include "DistortionProcessor.h"

class OscilloscopeView : public juce::Component,
                         private juce::Timer
{
public:
    OscilloscopeView(DistortionProcessor &processor);
    ~OscilloscopeView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Update with latest audio buffer
    void updateBuffer(const juce::AudioBuffer<float> &buffer);

private:
    DistortionProcessor &distortionProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Audio data
    juce::AudioBuffer<float> latestBuffer;
    juce::CriticalSection bufferLock;

    // Timer callback to update the oscilloscope
    void timerCallback() override;

    // Prepare data for the oscilloscope
    juce::String prepareWaveformData();

    // Flag to track if page is loaded
    bool pageLoaded;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeView)
};