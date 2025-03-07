#pragma once

#include <JuceHeader.h>
#include "DistortionProcessor.h"

class ControlPanel : public juce::Component,
                     private juce::Slider::Listener
{
public:
    ControlPanel(DistortionProcessor& processor);
    ~ControlPanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider* slider) override;
    
private:
    DistortionProcessor& distortionProcessor;
    
    juce::Slider driveSlider;
    juce::Slider mixSlider;
    
    juce::Label driveLabel;
    juce::Label mixLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};