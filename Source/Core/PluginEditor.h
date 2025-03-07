#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ControlPanel.h"
#include "MeterView.h"

class OxideAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    OxideAudioProcessorEditor(OxideAudioProcessor&);
    ~OxideAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OxideAudioProcessor& audioProcessor;
    
    MeterView meterView;
    ControlPanel controlPanel;
    
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OxideAudioProcessorEditor)
};