#pragma once

#include <JuceHeader.h>
#include "DelayProcessor.h"

class DelayControlPanel : public juce::Component,
                          private juce::Timer
{
public:
    DelayControlPanel(DelayProcessor &processor);
    ~DelayControlPanel() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    DelayProcessor &delayProcessor;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Last known values for change detection
    float lastDelayTime;
    float lastFeedback;
    float lastMix;
    float lastFilterFreq;
    bool lastPingPong;

    // Timer callback to check for processor changes
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayControlPanel)
};