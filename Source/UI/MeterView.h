#pragma once

#include <JuceHeader.h>

class MeterView : public juce::Component, 
                  private juce::Timer
{
public:
    MeterView();
    ~MeterView() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update the levels displayed in the meters
    void updateLevels(float leftLevel, float rightLevel);
    
    // Set gain values directly (e.g. when loading presets)
    void setInputGain(float newGain);
    void setOutputGain(float newGain);
    
    // Callback functions for gain changes
    std::function<void(float)> onInputGainChanged;
    std::function<void(float)> onOutputGainChanged;
    
    // Methods called by the WebBrowserComponent when gain changes occur in the UI
    void notifyInputGainChanged(float newGain);
    void notifyOutputGainChanged(float newGain);
    
private:
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    // Gain values in dB (-12 to +12)
    float inputGain;
    float outputGain;
    
    // Keep track of last levels to update gains without new level data
    float lastLeftLevel;
    float lastRightLevel;
    
    // Output levels (calculated based on input levels and gains)
    float outLeftLevel;
    float outRightLevel;
    
    // Flag to track if HTML page is loaded
    bool pageLoaded;
    
    // Timer callback to initialize the page after loading
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterView)
};