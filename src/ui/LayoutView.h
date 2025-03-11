#pragma once

#include <JuceHeader.h>
#include "DistortionProcessor.h"
#include "DelayProcessor.h"
#include "FilterProcessor.h"
#include "PulseProcessor.h"

class LayoutView : public juce::Component,
                   private juce::Timer
{
public:
    LayoutView(DistortionProcessor &distortionProcessor,
               DelayProcessor &delayProcessor,
               FilterProcessor &filterProcessor,
               PulseProcessor &pulseProcessor);
    ~LayoutView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Update audio buffer for oscilloscope
    void updateBuffer(const juce::AudioBuffer<float> &buffer);

    // Update levels for meters
    void updateLevels(float leftLevel, float rightLevel, float outLeftLevel, float outRightLevel);

    // Set input/output gain values
    void setInputGain(float newGain);
    void setOutputGain(float newGain);

    // Update the preset list in the dropdown
    void updatePresetList(const juce::StringArray &presets);

    // Force a refresh of all UI parameters
    void refreshAllParameters();

    // Callback functions for parameter changes
    std::function<void(float)> onInputGainChanged;
    std::function<void(float)> onOutputGainChanged;
    std::function<void(const juce::String &)> onPresetSelected;
    std::function<void()> onSaveClicked;

    // URL handler for callbacks from JS
    class LayoutMessageHandler : public juce::WebBrowserComponent
    {
    public:
        LayoutMessageHandler(LayoutView &owner);
        bool pageAboutToLoad(const juce::String &url) override;

    private:
        LayoutView &ownerView;
    };

private:
    DistortionProcessor &distortionProcessor;
    DelayProcessor &delayProcessor;
    FilterProcessor &filterProcessor;
    PulseProcessor &pulseProcessor;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    // State tracking variables
    bool pageLoaded;
    juce::CriticalSection bufferLock;
    juce::AudioBuffer<float> latestBuffer;
    juce::StringArray presetList;

    // Input/Output
    float inputGain;
    float outputGain;
    float lastLeftLevel;
    float lastRightLevel;

    // Distortion
    float lastDrive;
    float lastMix;
    juce::String lastAlgorithm;

    // Delay
    float lastDelayTime;
    float lastFeedback;
    float lastDelayMix;
    bool lastPingPong;

    // Filter
    juce::String lastFilterType;
    float lastFilterFreq;
    float lastResonance;

    // Pulse
    float lastPulseMix;
    juce::String lastPulseRate;

    // Timer callback for UI updates
    void timerCallback() override;

    // Prepare waveform data for oscilloscope
    juce::String prepareWaveformData();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutView)
};