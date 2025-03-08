#pragma once

#include <JuceHeader.h>

class HeaderView : public juce::Component
{
public:
    HeaderView();
    ~HeaderView() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Callback for when a preset is selected
    std::function<void(const juce::String&)> onPresetSelected;
    
    // Callback for when save is clicked
    std::function<void()> onSaveClicked;
    
private:
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderView)
};