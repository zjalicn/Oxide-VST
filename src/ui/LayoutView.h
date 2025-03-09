#pragma once

#include <JuceHeader.h>
#include "DistortionProcessor.h"
#include "DelayProcessor.h"

class LayoutView : public juce::Component,
                   private juce::Timer
{
public:
    LayoutView(DistortionProcessor &distortionProcessor,
               DelayProcessor &delayProcessor);
    ~LayoutView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Update audio buffer for oscilloscope
    void updateBuffer(const juce::AudioBuffer<float> &buffer);

    // Update levels for meters
    void updateLevels(float leftLevel, float rightLevel);

    // Set input/output gain values
    void setInputGain(float newGain);
    void setOutputGain(float newGain);

    // Callback functions for parameter changes
    std::function<void(float)> onInputGainChanged;
    std::function<void(float)> onOutputGainChanged;
    std::function<void(const juce::String &)> onPresetSelected;
    std::function<void()> onSaveClicked;

private:
    // Processors
    DistortionProcessor &distortionProcessor;
    DelayProcessor &delayProcessor;

    // WebView
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // State tracking variables
    bool pageLoaded;
    juce::CriticalSection bufferLock;
    juce::AudioBuffer<float> latestBuffer;

    // Current parameter values
    float inputGain;
    float outputGain;
    float lastLeftLevel;
    float lastRightLevel;
    float lastDrive;
    float lastMix;
    juce::String lastAlgorithm;
    float lastDelayTime;
    float lastFeedback;
    float lastDelayMix;
    float lastFilterFreq;
    bool lastPingPong;

    // Timer callback for UI updates
    void timerCallback() override;

    // Prepare waveform data for oscilloscope
    juce::String prepareWaveformData();

    // URL handler for callbacks from JS
    class LayoutMessageHandler : public juce::WebBrowserComponent
    {
    public:
        LayoutMessageHandler(LayoutView &owner);
        bool pageAboutToLoad(const juce::String &url) override;

    private:
        LayoutView &ownerView;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutView)
};