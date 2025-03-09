#pragma once

#include <JuceHeader.h>
#include "DistortionProcessor.h"

class ControlPanel : public juce::Component,
                     private juce::Timer
{
public:
    ControlPanel(DistortionProcessor &processor);
    ~ControlPanel() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    DistortionProcessor &distortionProcessor;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Last known values for change detection
    float lastDrive;
    float lastMix;
    juce::String lastAlgorithm;

    // Timer callback to check for processor changes
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};