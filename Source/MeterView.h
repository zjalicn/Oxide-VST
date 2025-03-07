#pragma once

#include <JuceHeader.h>

class MeterView : public juce::Component
{
public:
    MeterView();
    ~MeterView() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update the levels displayed in the meters
    void updateLevels(float leftLevel, float rightLevel);
    
private:
    std::unique_ptr<juce::WebBrowserComponent> webView;
    juce::File tempHtmlFile;
    
    void createHtmlContent();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterView)
};